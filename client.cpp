#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <climits>
#include <unistd.h>

#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

const int NUMBER_OF_ARGS = 3;
const int PACKET_SIZE = 1024;

struct Arguments
{
    int port;
    string host;
    string filename;
};

void printUsage()
{
    cerr<< "USAGE: ./client <HOSTNAME-OR-IP> <PORT> <FILENAME>\n";
}

void printError(string message)
{
    cerr<<"ERROR: ";
    cerr<< message <<endl;
}

sockaddr_in createServerAddr(const int port, const string IP)
{
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);     // short, network byte order
    serverAddr.sin_addr.s_addr = inet_addr(IP.c_str());
    memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
    return serverAddr;
}

void serverConnect(const int sockfd, const struct sockaddr_in &serverAddr)
{
    // connect to the server
    if (connect(sockfd,(struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
    {
        perror("connect");
        exit(2);
    }
}

sockaddr_in createClientAddr(const int sockfd)
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

void connectionSetup(const struct sockaddr_in clientAddr)
{
    char ipstr[INET_ADDRSTRLEN] = {'\0'};
    inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
    cout << "Set up a connection from: " << ipstr << ":" <<
    ntohs(clientAddr.sin_port) << endl;
}

void communicate(const int sockfd, const string filename)
{
    // send/receive data to/from connection
    fstream fin;
    fin.open(filename, ios::in);
    string input;
    char buf[PACKET_SIZE] = {0};
    stringstream ss;
    
    do {
        fin.read(buf, PACKET_SIZE);
        
        if (send(sockfd, buf, fin.gcount(), 0) == -1)
        {
            printError("Unable to send data to server");
            exit(1);
        }
    } while (!fin.eof());
    fin.close();
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
    
    // host
    // TODO: use getaddrinfo to check for hostname if(getaddrinfo )
    args.host = argv[1];
    
    // port
    long temp_port = strtol(argv[2],nullptr,10);
    if(temp_port == 0 || temp_port==LONG_MAX || temp_port==LONG_MIN || (temp_port<1024))
    {
        printError("Port number needs to be a valid integer greater than 1023.");
        exit(1);
    }
    args.port = temp_port;
    
    // filename
    args.filename = (string) argv[3];
    
    return args;
}

int
main(int argc, char **argv)
{
  Arguments args = parseArguments(argc, argv);
    
  // create a socket using TCP IP
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  struct sockaddr_in serverAddr = createServerAddr(args.port, args.host);

  serverConnect(sockfd, serverAddr);

  struct sockaddr_in clientAddr = createClientAddr(sockfd);
  
  connectionSetup(clientAddr);
    
  communicate(sockfd, args.filename);

  close(sockfd);

  return 0;
}
