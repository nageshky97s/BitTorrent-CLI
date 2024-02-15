#pragma once
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <iostream>
#include <chrono>

#include "messageHandlingServer.cpp"
#include "messagesCreate.cpp"
#include "peer.cpp"

int createServer(int portno)
{
    
    const int opt = 1;   
    int serverfd;
    int clientfd,clientaddrlen;
    struct sockaddr_in serv_addr, cli_addr;

    serverfd =  socket(AF_INET, SOCK_STREAM, 0);
    if(serverfd<0)
        std::cout<<"Error opening port\n";
    
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_addr.s_addr = INADDR_ANY; 
    serv_addr.sin_port = htons(portno);



    if (bind(serverfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              std::cout<<"ERROR on binding";

    if( setsockopt(serverfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,  
          sizeof(opt)) < 0 )   
    {   
        perror("setsockopt");   
        exit(EXIT_FAILURE);   
    }   

    if (listen(serverfd, 5) < 0)   
    {   
        perror("listen");   
        exit(EXIT_FAILURE);   
    }   
    

    return serverfd;



}


int uploadToPeer(url_params& url_serv,bitfield& bit_data,std::vector<std::fstream*>& file,tracker_info& trk,pieces& pee,int peersock)
{
    std::cout<<"Starting upload\n";
    peer_info peer_data;
    threadlock.lock();
    peer_data.no_of_havesent=url_serv.downloaded;
    threadlock.unlock();
    std::chrono::time_point<std::chrono::system_clock> current;
    const int handshake_size = 1+19+8+20+20;
    char handshake[handshake_size];
    createHandshakeMessage(handshake,std::string(reinterpret_cast<char*>(url_serv.info_hash),20),peerid_string);

    peer_data.peer_sock_id = peersock;
    

    //Once connected send and receive handshake message along with the bitfiled message 
    std::string handshake_str(handshake,handshake_size);
    std::string sendmes =handshake_str+ createBitfieldMessage(bit_data.spare_bits,pee.pieces_disk)+createLen1Message(UNCHOKE)+createKeepAlive();
    std::string recvmes;
    int n;
    int peerd;
    char buffer_peer[4096];

    

    if(send(peersock, sendmes.c_str(), sendmes.size(), MSG_NOSIGNAL)!=sendmes.size())
    {
        std::cout<<"\nProblem with sending message\n";
    }
    if(errno==EPIPE){
        std::cout<<" Failed to send INTERESTED MESSAGE\n";
        std::cout<<"Closing Connection Problem with Connection, writing to closed connection \n ";
        close(peer_data.peer_sock_id);
        return FAILURE;
    }
    while ((n = recv(peersock, buffer_peer, sizeof(buffer_peer), 0)) > 0)
    {   
        std::cout<<"Received data from peer : ";
        recvmes.append(buffer_peer, n);
    }
    for(int i=0;i<20;i++)
    {
    //std::cout<<peer_recv_str[i+1+8+(peer_recv_str[0])& 0xFFFF]<<"  "<<url.info_hash[i]<<std::endl;
        if(recvmes[i+1+8+(recvmes[0])& 0xFFFF]!=(char)url_serv.info_hash[i])
        {
            std::cout<<"Info Hash Mismatch, need to drop connection"<<std::endl;
            std::cout<<"Closing Connection because of info mismatch \n";
            close(peer_data.peer_sock_id);
            return FAILURE;
        }
    }
    std::cout<<"Handshake with peer successfull\n";
    recvmes=recvmes.substr(handshake_size);


    while(true)
    {
        processMessageServer(recvmes,peer_data,bit_data,file,trk,pee,sendmes,peerd);
        threadlock.lock();
        if(peer_data.no_of_havesent!=pee.havemesslist.size() && pee.havemesslist.size()>peer_data.no_of_havesent )
        {
            int z=peer_data.no_of_havesent;
            for(int i=0;i<pee.havemesslist.size()-z;i++)
            {
                sendmes+=createHaveMessage(pee.havemesslist.at(pee.havemesslist.size()-i-1));
                peer_data.no_of_havesent++;
            }
        }
        threadlock.unlock();
         if(send(peersock,  sendmes.c_str(), sendmes.size(), MSG_NOSIGNAL)!=sendmes.size())
        {
            std::cout<<"Problem with sending message\n";
        }
        if(errno==EPIPE){
            std::cout<<" Failed to send INTERESTED MESSAGE\n";
            std::cout<<"Closing Connection Problem with Connection, writing to closed connection \n ";
            close(peer_data.peer_sock_id);
            return FAILURE;
        }
        while ((n = recv(peersock, buffer_peer, sizeof(buffer_peer), 0)) > 0)
        {   
            std::cout<<"Received data from peer : ";
            recvmes.append(buffer_peer, n);
        }

        current=std::chrono::system_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(current-peer_data.latest_comms).count()>120000)//120000 ms is 2min in seconds
        {
            std::cout<<"More than 2 minutes without comms, dropping connection with peer\n";
            std::cout<<"Closing Connection lack of comms\n";
            close(peer_data.peer_sock_id); //yet to resolve this
            
            return SUCCESS;        
        }

        if(peerd==trk.no_of_pieces)
        {
            std::cout<<"The peer has finished downloading, closing connection\n";
            close(peer_data.peer_sock_id);
            return SUCCESS;
        }
    }
    
   
}