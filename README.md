# CS118 Project 1

## Information

Name: Akshara Sundararajan
UID: 404731846
Email: amedrops@g.ucla.edu

## Makefile

This provides a couple make targets for things.
By default (all target), it makes the `server` and `client` executables.

It provides a `clean` target, and `tarball` target to create the submission file as well.

You will need to modify the `Makefile` to add your userid for the `.tar.gz` turn-in at the top of the file.

## Academic Integrity Note

You are encouraged to host your code in private repositories on [GitHub](https://github.com/), [GitLab](https://gitlab.com), or other places.  At the same time, you are PROHIBITED to make your code for the class project public during the class or any time after the class.  If you do so, you will be violating academic honestly policy that you have signed, as well as the student code of conduct and be subject to serious sanctions.

## Provided Files

`server.cpp` and `client.cpp` are the entry points for the server and client part of the project.

## High Level Design

### Server

The server basically follows the following workflow:
- Function to parse the arguments into a struct
    - parsePort() checks that the port is valid and converts it into an int
    - create a directory, if the passed in one does not exist
- Setup signal handlers for SIGTERM and SIGQUIT
- Create the socket, set it so that the address can be reused
- Set the socket to nonblocking (setupEnvironment() function)
- createServerAddr() creates the sockaddr_in needed for binding
- bindSocket() binds the socket, checks for errors
- listen() loop to socket
    - accept the new connection
    - check for errors, that aren't EWOULDBLOCK
    - if the error is EWOULDBLOCK, just continue
    - if there's a new connection - create a new thread to start receiving
- the worker() function for the thread, sets the connection to non blocking, then calls communicate()
- communicate() selects to see if can read, writes to new file, and checks for timeout

### Client

The client follows the following workflow:
- parseArguments() parses arguments and checks for correctness, puts results into a struct
- create the socket
- set the socket to nonblocking in the setupEnvironment() function
- createServerAddr() creates the sockaddr_in struct
- serverConnect() connects to the socket, checks for errors and timeout
- communicate() handles the sending of data to the server, reading from the source file and checking for timeout

# Problems
- buffer overflow after making socket from server non blocking: sloved by adding O_NONBLOCK as flag to previous flags, instead of replacing the previous flags.
- transferring 100MiB files failed with a "client has not been able to transfer in 15s" error, because I forgot to reset the timeval struct and reset/clearing the fd_sets 


# Additional Libraries
thread
csignal
climits
chrono
fstream
errno
All the libraries from the sample code

# Acknowledgments

Resources used:
http://web.cs.ucla.edu/classes/spring17/cs118/hints/server.cpp
http://web.cs.ucla.edu/classes/spring17/cs118/hints/client.cpp
http://web.cs.ucla.edu/classes/spring17/cs118/hints/multi-thread.cpp
TA Slides
http://beej.us/guide/bgnet/html/multi/syscalls.html#socket
http://beej.us/guide/bgnet/html/multi/selectman.html
https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking
https://github.com/angrave/SystemProgramming/wiki/Networking,-Part-2:-Using-getaddrinfo
https://www.scottklement.com/rpg/socktut/nonblocking.html
https://stackoverflow.com/questions/3828192/checking-if-a-directory-exists-in-unix-system-call
https://www.geeksforgeeks.org/socket-programming-in-cc-handling-multiple-clients-on-server-without-multi-threading/
https://stackoverflow.com/questions/10204134/tcp-connect-error-115-operation-in-progress-what-is-the-cause
Piazza
