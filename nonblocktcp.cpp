#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <string>
#include <string.h>
#include <iostream>

#include "global.hpp"

int tcpconnectNB(int port, std::string hostname)
{
    int res; 
    int soc;
  struct sockaddr_in addr; 
  long arg; 
  fd_set myset; 
  struct timeval tv; 
  int valopt; 
  socklen_t lon; 
  std::cout<<hostname<<" "<<port<<std::endl;

  // Create socket 
  soc = socket(AF_INET, SOCK_STREAM, 0); 
  if (soc < 0) { 
     fprintf(stderr, "Error creating socket (%d %s)\n", errno, strerror(errno)); 
     return FAILURE; 
  } 

  addr.sin_family = AF_INET; 
  addr.sin_port = htons(port); 
  addr.sin_addr.s_addr = inet_addr(hostname.c_str()); 

  // Set non-blocking 
  if( (arg = fcntl(soc, F_GETFL, NULL)) < 0) { 
     fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
     return FAILURE; 
  } 
  arg |= O_NONBLOCK; 
  if( fcntl(soc, F_SETFL, arg) < 0) { 
     fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
     return FAILURE; 
  } 
  // Trying to connect with timeout 
  res = connect(soc, (struct sockaddr *)&addr, sizeof(addr)); 
  if (res < 0) { 
     if (errno == EINPROGRESS) { 
        fprintf(stderr, "EINPROGRESS in connect() - selecting\n"); 
        do { 
           tv.tv_sec = 3; 
           tv.tv_usec = 0; 
           FD_ZERO(&myset); 
           FD_SET(soc, &myset); 
           res = select(soc+1, NULL, &myset, NULL, &tv); 
           if (res < 0 && errno != EINTR) { 
              fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
              return FAILURE; 
           } 
           else if (res > 0) { 
              // Socket selected for write 
              lon = sizeof(int); 
              if (getsockopt(soc, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) { 
                 fprintf(stderr, "Error in getsockopt() %d - %s\n", errno, strerror(errno)); 
                 return FAILURE; 
              } 
              // Check the value returned... 
              if (valopt) { 
                 fprintf(stderr, "Error in delayed connection() %d - %s\n", valopt, strerror(valopt) 
); 
                 return FAILURE; 
              } 
              break; 
           } 
           else { 
              fprintf(stderr, "Timeout in select() - Cancelling!\n"); 
              return FAILURE; 
           } 
        } while (1); 
     } 
     else { 
        fprintf(stderr, "Error connecting %d - %s\n", errno, strerror(errno)); 
        return FAILURE; 
     } 
  } 
  // Set to blocking mode again... 
  if( (arg = fcntl(soc, F_GETFL, NULL)) < 0) { 
     fprintf(stderr, "Error fcntl(..., F_GETFL) (%s)\n", strerror(errno)); 
     return FAILURE; 
  } 
  arg &= (~O_NONBLOCK); 
  if( fcntl(soc, F_SETFL, arg) < 0) { 
     fprintf(stderr, "Error fcntl(..., F_SETFL) (%s)\n", strerror(errno)); 
     return FAILURE; 
  } 
    return soc;
}