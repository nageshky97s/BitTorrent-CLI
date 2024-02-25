#pragma once
#include <string>
#include <iostream>
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <netdb.h>

void* getSinAddr(addrinfo *addr)
{
    switch (addr->ai_family)
    {
        case AF_INET:
            return &(reinterpret_cast<sockaddr_in*>(addr->ai_addr)->sin_addr);

        case AF_INET6:
            return &(reinterpret_cast<sockaddr_in6*>(addr->ai_addr)->sin6_addr);
    }

    return NULL;
}



int udpConnect(std::string hostname, int port, int resolution,struct sockaddr_in& servaddr)
{
    int sockfd;
    char ip[INET6_ADDRSTRLEN];

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        std::cout<<"UDP socket creation failed\n"; 
        exit(EXIT_FAILURE); 
    } 
    

     if(resolution)
    {
        strcpy(ip,hostname.c_str());

    }
    else
    {
        addrinfo hints = {};
        hints.ai_flags = AI_CANONNAME;
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_protocol = IPPROTO_TCP;

        addrinfo *res;
        
        int ret = getaddrinfo(hostname.c_str(),NULL, &hints, &res);
        if (ret != 0)
        {
            std::cout << "getaddrinfo() failed: " << gai_strerror(ret) << "\n";
        }
        
        

        else
        {
            if(res->ai_family==AF_INET6)
                res=res->ai_next;
                   
            std::cout << inet_ntop(res->ai_family, getSinAddr(res), ip, sizeof(ip)) << "\n";
            
        
            freeaddrinfo(res);
        }


    }
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = inet_addr(ip); // Server IP
    

    return sockfd;
}