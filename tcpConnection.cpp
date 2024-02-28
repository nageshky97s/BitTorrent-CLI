#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <cctype>
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <unordered_map>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <algorithm>
#define FAILURE -2222
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

int tcpConnect(int port, std::string hostname, int resolve) {

    
    int sockfd=socket(AF_INET,SOCK_STREAM,0);
    char ip[INET6_ADDRSTRLEN];
    

    if(sockfd==-1)
    {
        std::cout<<"Failed to create socket \n";
        return -1;
    }
    if(resolve)
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
      /*for(addrinfo* addr = res; addr; addr = addr->ai_next)
        {
            std::cout << inet_ntop(addr->ai_family, getSinAddr(addr), ip, sizeof(ip)) << "\n";
        }*/
        
        std::cout << inet_ntop(res->ai_family, getSinAddr(res), ip, sizeof(ip)) << "\n";
      
        freeaddrinfo(res);
    }


    }
    
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); // Server port number
    server_address.sin_addr.s_addr = inet_addr(ip); // Server IP
    

    if(connect(sockfd,(struct sockaddr*)&server_address,sizeof(server_address))!=0)
    {
        std::cout<<"Connection Failed\n";
        return -FAILURE;
    }
    else
    {

        std::cout<<"Successfully connected to Server \n";
    }

    
    return sockfd;
    


    


 
}

