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
        printError("bind() failed.");
        exit(1);
    }
}

void listenToSocket(const int sockfd)
{
    // set socket to listen status
    if (listen(sockfd, 1) == -1) {
        printError("listen() failed");
        exit(1);
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
    
    while (!isEnd)
    {
        memset(buf, '\0', sizeof(buf));
        
        int rec_res = recv(clientSockfd, buf, PACKET_SIZE, 0);
        
        if (rec_res == -1)
        {
            printError("Error in receiving data");
            exit(1);
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
