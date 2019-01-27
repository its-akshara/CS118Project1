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

sockaddr_in createServerAddr(int port, string IP)
{
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(40000);     // short, network byte order
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    return serverAddr;
}

void serverConnect(int sockfd, struct sockaddr_in &serverAddr)
{
    // connect to the server
    if (connect(sockfd,(struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("connect");
        exit(2);
    }
}

sockaddr_in createClientAddr(int sockfd)
{
    struct sockaddr_in clientAddr;
    socklen_t clientAddrLen = sizeof(clientAddr);
    if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1)
    {
        perror("getsockname");
        exit(3);
    }
    return clientAddr;
}

int
main()
{
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr = createServerAddr(40000, "127.0.0.1");

  serverConnect(sockfd, serverAddr);

  struct sockaddr_in clientAddr = createClientAddr(sockfd);

  char ipstr[INET_ADDRSTRLEN] = {'\0'};
  inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
  cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;


  // send/receive data to/from connection
  bool isEnd = false;
  string input;
  char buf[20] = {0};
  stringstream ss;

  while (!isEnd) {
    memset(buf, '\0', sizeof(buf));

    cout << "send: ";
    cin >> input;
    if (send(sockfd, input.c_str(), input.size(), 0) == -1) {
      perror("send");
      return 4;
    }


    if (recv(sockfd, buf, 20, 0) == -1) {
      perror("recv");
      return 5;
    }
    ss << buf << std::endl;
    cout << "echo: ";
    cout << buf << std::endl;

    if (ss.str() == "close\n")
      break;

    ss.str("");
  }

  close(sockfd);

  return 0;
}
