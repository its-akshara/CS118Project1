#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <climits>

using namespace std;

const int NUMBER_OF_ARGS = 2;
const int PACKET_SIZE = 1024;

struct Arguments
{
    int port;
    string fileDir;
};

void printUsage()
{
    cerr<< "USAGE: ./server <PORT> <FILE-DIR>\n";
}

void printError(string message)
{
    cerr<<"ERROR: ";
    cerr<< message <<endl;
}

long parsePort(char **argv)
{
    long temp_port = strtol(argv[1],nullptr,10);
    if(temp_port == 0 || temp_port==LONG_MAX || temp_port==LONG_MIN || (temp_port<1024))
    {
        printError("Port number needs to be a valid integer greater than 1023.");
        exit(1);
    }
    return temp_port;
}

void exitOnError(int sockfd)
{
    close(sockfd);
    exit(1);
}

Arguments parseArguments(int argc, char**argv)
{
    if(argc!=(NUMBER_OF_ARGS+1))
    {
        printError("Incorrect number of arguments");
        printUsage();
        exit(1);
    }
    Arguments args;
    
    // port
    args.port = parsePort(argv);
    
    // filename
    args.fileDir = (string) argv[2];
    
    return args;
}

void setReuse(const int sockfd)
{
    // allow others to reuse the address
    int yes = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        printError("setsockopt() failed.");
        exitOnError(sockfd);
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
        printError("bind() failed.");
        exitOnError(sockfd);
    }
}

void listenToSocket(const int sockfd)
{
    // set socket to listen status
    if (listen(sockfd, 1) == -1) {
        printError("listen() failed");
        exitOnError(sockfd);
    }
}

int establishConnection(const int sockfd)
{
    // accept a new connection
    struct sockaddr_in clientAddr;
    socklen_t clientAddrSize = sizeof(clientAddr);
    int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);
    
    if (clientSockfd == -1) {
        printError("accept() failed.");
        exit(1);
    }
    
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    cout << "Accept a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;
    return clientSockfd;
}

string getFileName(string fileDir, int num)
{
    return fileDir +"/" + to_string(num) + ".file";
}

void communicate(int clientSockfd, string fileDir, int num)
{
    // read/write data from/into the connection
    bool isEnd = false;
    char buf[PACKET_SIZE] = {0};
    fstream fout;
    fout.open(getFileName(fileDir,num), ios::out);
    
    fd_set readfds;
    FD_ZERO(&readfds);
    FD_SET(clientSockfd, &readfds);
    
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;
    
    while (!isEnd)
    {
        memset(buf, '\0', sizeof(buf));
        
        int sel_res = select(clientSockfd+1,&readfds,NULL,NULL,&timeout);
        
        if(sel_res == -1)
        {
            fout.close();
            printError("select() failed.");
            exitOnError(clientSockfd);
        }
        else if(sel_res==0)
        {
            fout.close();
            fout.open(getFileName(fileDir,num), ios::out);
            fout<<"ERROR: Timeout! Server has not received data from client in more than 15 sec.";
            fout.close();
            printError("Timeout! Server has not received data from client in more than 15 sec.");
            exitOnError(clientSockfd);
        }
 
        int rec_res = recv(clientSockfd, buf, PACKET_SIZE, 0);
        
        if (rec_res == -1)
        {
            printError("Error in receiving data");
            fout.close();
            exitOnError(clientSockfd);
        }
        else if(!rec_res)
        {
            break;
        }
        

        fout.write(buf, rec_res);
        
    }
    fout.close();
}

int main(int argc, char **argv)
{
    Arguments args = parseArguments(argc, argv);
    
  // create a socket using TCP IP
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);

    setReuse(sockfd);

    struct sockaddr_in addr = createServerAddr(sockfd, args.port, "127.0.0.1");

    bindSocket(sockfd, addr);
    
    listenToSocket(sockfd);

    int clientSockfd = establishConnection(sockfd);

    communicate(clientSockfd, args.fileDir, 1);

  close(clientSockfd);

  return 0;
}
