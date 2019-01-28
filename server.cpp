#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

using namespace std;

void setReuse(const int sockfd)
{
    // allow others to reuse the address
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        perror("setsockopt");
        exit(1);
    }
}

struct sockaddr_in createServerAddr(const int sockfd, const int port, const string IP)
{
    // bind address to socket
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);     // short, network byte order
    addr.sin_addr.s_addr = inet_addr(IP.c_str());
    memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));
    return addr;
}

void bindSocket(const int sockfd, const sockaddr_in addr)
{
    if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1)
    {
        perror("bind");
        exit(2);
    }
}

void listenToSocket(const int sockfd)
{
    // set socket to listen status
    if (listen(sockfd, 1) == -1) {
        perror("listen");
        exit(3);
    }
}

int establishConnection(const int sockfd)
{
    // accept a new connection
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    
    if (clientSockfd == -1) {
        perror("accept");
        exit(4);
    }
    
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;
    return clientSockfd;
}

void performTask(int clientSockfd)
{
    // read/write data from/into the connection
    bool isEnd = false;
    char buf[20] = {0};
    stringstream ss;
    
    while (!isEnd) {
        memset(buf, '\0', sizeof(buf));
        
        if (recv(clientSockfd, buf, 20, 0) == -1) {
            perror("recv");
            exit(5);
        }
        
        ss << buf <<endl;
        cout << buf << endl;
        
        if (send(clientSockfd, buf, 20, 0) == -1) {
            perror("send");
            exit(6);
        }
        
        if (ss.str() == "close\n")
            break;
        
        ss.str("");
    }
}

int
main()
{
  // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    setReuse(sockfd);

    struct sockaddr_in addr = createServerAddr(sockfd, 40000, "127.0.0.1");

    bindSocket(sockfd, addr);
    
    listenToSocket(sockfd);

    int clientSockfd = establishConnection(sockfd);

    performTask(clientSockfd);

  close(clientSockfd);

  return 0;
}
