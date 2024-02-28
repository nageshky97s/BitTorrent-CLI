#pragma once
#include <signal.h>
#include<string>
#include<iostream>
#include <deque>
#include <chrono>
#include "global.hpp"
#include "nonblocktcp.cpp"
#include "messageHandling.cpp"
#include "helper.cpp"




void createHandshakeMessage(char* handshake,std::string info_hash,std::string& peerid)
{
   
    const int handshake_size = 1+19+8+20+20;    
    const int protocol_name_offset = 1;
    const int reserved_offset = protocol_name_offset + 19;
    const int info_hash_offset = reserved_offset + 8;
    const int peer_id_offset = info_hash_offset + 20;
    const char prefix = 19;
    const char zero = 0;
    const std::string BitTorrent_protocol = "BitTorrent protocol";
    

    handshake[0] = prefix; // length prefix of the string
    std::copy(BitTorrent_protocol.begin(), BitTorrent_protocol.end(),
            &handshake[protocol_name_offset]); // protocol name

    for(int i=reserved_offset;i<info_hash_offset;i++)
    {
        handshake[i]=zero;
    }
    for(int j=0;j<20;j++)
    {
        handshake[info_hash_offset+j]=info_hash[j];
        handshake[peer_id_offset+j]=peerid[j];
    }

    for(int i=0;i<handshake_size;i++)
    {
        std::cout<<handshake[i];
    }
  

}

bool whileStatus(pieces& p,url_params& u,tracker_info& t)
{
    bool ret_val;
    threadlock.lock();
    if(!p.pieces_to_download.empty() && u.downloaded<t.no_of_pieces)
    ret_val=true;
    else
    ret_val=false;
    threadlock.unlock();
    return ret_val;
}
bool queueIsEmpty(pieces& p)
{
    bool ret_val;
    threadlock.lock();
    if(p.pieces_to_download.empty())
    ret_val=true;
    else
    ret_val=false;
    threadlock.unlock();
    return ret_val;
}


int downloadFromPeer(tracker_info& trk,url_params& url,bitfield& bit_data,int port,std::string hostname,std::vector<std::fstream*>& file,pieces& pstruct)
{
    
    std::chrono::time_point<std::chrono::system_clock> current;
    peer_info peerdata; 
    peerdata.choked_client=1;
    peerdata.interested_peer=0;
    peerdata.interested_client=1;
    peerdata.choked_peer=1;
    peerdata.no_of_havesent=url.downloaded;
    
    peerdata.pieces_have.resize(trk.no_of_pieces);
    peerdata.block_placeholder.resize(no_of_blocks,"");
    std::fill(peerdata.pieces_have.begin(),peerdata.pieces_have.end(),0);
    int n=0; //##
    struct timeval timeout;
    timeout.tv_sec =3;
    timeout.tv_usec = 0;
    std::string peer_recv_str="";
    std::string peer_send_str="";
    std::string block_piece;

    int piece_no;
    char buffer_peer[4096];
    unsigned char block_hash[20] ;
    const int handshake_size = 1+19+8+20+20;
    char handshake[handshake_size];
    int blocker;
    int status;
    int isLastPiece=0;

    createHandshakeMessage(handshake,std::string(reinterpret_cast<char*>(url.info_hash),20),peerid_string);

    peerdata.peer_sock_id=tcpconnectNB(port,hostname);
    if(peerdata.peer_sock_id==FAILURE || peerdata.peer_sock_id ==-1 )
    {
        std::cout<<"Failed to connect to peer\n";
        return FAILURE;
    }
    std::cout<<"Peer_sock: "<<peerdata.peer_sock_id<<std::endl;
    setsockopt(peerdata.peer_sock_id, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
     if(peerdata.peer_sock_id!=FAILURE)
    {
        
        if(send(peerdata.peer_sock_id,handshake, handshake_size,0 )!=-1)
        
        {       
            std::cout<<"Sent data to peer successfully"<<std::endl;
            while ((n = recv(peerdata.peer_sock_id, buffer_peer, sizeof(buffer_peer), 0)) > 0)
            {   
                std::cout<<"Received data from peer : ";
                peer_recv_str.append(buffer_peer, n);
            }
            std::cout<<peer_recv_str.size()<<std::endl;
            std::cout<<peer_recv_str<<std::endl;
            
        } 
        else{
            std::cout<<"Failed to send to peer\n";
            std::cout<<"Failed to connect to the peer\n"; 
            std::cout<<"Closing Connection beacuse peer not responding\n";
           // pstruct.peer_ip_port_map.erase(hostname);
            close(peerdata.peer_sock_id); 
            return FAILURE;
        }  
    }
    else{
        std::cout<<"Failed to connect to the peer\n"; 
        std::cout<<"Closing Connection beacuse peer not responding\n";
       // pstruct.peer_ip_port_map.erase(hostname);
        close(peerdata.peer_sock_id); 
        return FAILURE;
    }
    
    for(int i=0;i<20;i++)
    {
    //std::cout<<peer_recv_str[i+1+8+(peer_recv_str[0])& 0xFFFF]<<"  "<<url.info_hash[i]<<std::endl;
        if(peer_recv_str[i+1+8+(peer_recv_str[0])& 0xFFFF]!=(char)url.info_hash[i])
        {
            std::cout<<"Info Hash Mismatch, need to drop connection"<<std::endl;
            std::cout<<"Closing Connection because of info mismatch \n";
           // pstruct.peer_ip_port_map.erase(hostname);
            close(peerdata.peer_sock_id);
            return FAILURE;
            //exit(0);
        // break;
        }
    
    }

    std::cout<<"Hansdshake successfull\n"<<1+((peer_recv_str[0])& 0xFFFF)<<std::endl;
    timeout.tv_sec =1;
    timeout.tv_usec = 0;
    setsockopt(peerdata.peer_sock_id, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    

    std::string message_string;
    if(peer_recv_str.size()>handshake_size)
    {
        message_string=peer_recv_str.substr(handshake_size);
        std::cout<<message_string.size()<<std::endl;
        switch(processMessage(message_string,peerdata,bit_data,file,trk))
        {
            case BITFIELDFAILURE:
            {
                std::cout<<"Bitfield failure Kaboom\n";
                std::cout<<"Closing Connection Problem with BITFIELD\n ";
                close(peerdata.peer_sock_id);
               // pstruct.peer_ip_port_map.erase(hostname);                
                return FAILURE;
            }
            break;
            case FAILURE:
            {
                std::cout<<"Continuing with communication\n";
            }
            
        }
    }
    else{
        std::cout<<"No bitfield message sent\n";
    }
    
    peer_send_str+=createLen1Message(INTERESTED);
    peer_recv_str="";
    if(send(peerdata.peer_sock_id,peer_send_str.c_str(), peer_send_str.size(),MSG_NOSIGNAL )!=-1)
    {
        
        std::cout<<"Sent data to peer successfully"<<std::endl;
        while ((n = recv(peerdata.peer_sock_id, buffer_peer, sizeof(buffer_peer), 0)) > 0)
        {   
            std::cout<<"Received data from peer : ";
            peer_recv_str.append(buffer_peer, n);
        }

    }else if(errno==EPIPE){std::cout<<" Failed to send INTERESTED MESSAGE\n";
                std::cout<<"Closing Connection Problem with Connection, writing to closed connection \n ";
                close(peerdata.peer_sock_id);
                threadlock.lock();
                pstruct.pieces_to_download.push_back(piece_no);
                threadlock.unlock();
                //pstruct.peer_ip_port_map.erase(hostname); //MAYBE can try again in the next iteration of connections
        
        return FAILURE;}
    
    else{
        std::cout<<" Failed to send INTERESTED MESSAGE\n";
        std::cout<<"Closing Connection Problem with Connection, failed to send interested message\n ";
        close(peerdata.peer_sock_id); 
       // pstruct.peer_ip_port_map.erase(hostname); //MAYBE can try again in the next iteration of connections
        
        return FAILURE;
    }
    switch(processMessage(peer_recv_str,peerdata,bit_data,file,trk))
        {
            case BITFIELDFAILURE:
            {
                std::cout<<"Bitfield failure Kaboom\n";
                std::cout<<"Closing Connection Problem with BITFIELD\n ";
                close(peerdata.peer_sock_id);
               //pstruct.peer_ip_port_map.erase(hostname);
                return FAILURE;
            }
            break;
            case FAILURE:
            {
                std::cout<<"Continuing with communication\n";
               std::cout<<"Bitfield failure Kaboom\n";
               std::cout<<"Closing Connection Problem with BITFIELD\n ";
               close(peerdata.peer_sock_id);
                // pstruct.peer_ip_port_map.erase(hostname);
                return FAILURE;
            }
            
        }
   
    //while(!pstruct.pieces_to_download.empty() && url.downloaded!=trk.no_of_pieces)
    while(whileStatus(pstruct,url,trk))
    
    {  
        //if(pstruct.pieces_to_download.empty()&& url.downloaded==trk.no_of_pieces){std::cout<<"DOWLOAD COMPLETE: "<<url.downloaded<<std::endl;break;}
        if(queueIsEmpty(pstruct)){
            current=std::chrono::system_clock::now();
            std::cout<<"Waiting\n";
            if(std::chrono::duration_cast<std::chrono::milliseconds>(current-peerdata.latest_comms).count()>120000)//120000 ms is 2min in seconds
            {
                std::cout<<"More than 2 minutes without comms, dropping connection with peer\n";
                std::cout<<"Closing Connection lack of comms\n";
                close(peerdata.peer_sock_id);
               // pstruct.peer_ip_port_map.erase(hostname);
                return SUCCESS;        
            }
        }
        else{
        if(peerdata.choked_client==1)
        {
            std::cout<<"Closing Connection because we are choked\n";
            close(peerdata.peer_sock_id);
            //pstruct.peer_ip_port_map.erase(hostname);
            return FAILURE;
        }
       
        block_piece="";
        isLastPiece = 0;
        threadlock.lock();
        piece_no=pstruct.pieces_to_download.front();
        pstruct.pieces_to_download.pop_front();
        threadlock.unlock();

        

        if(peerdata.pieces_have[piece_no]==1)
        {
            std::cout<<"Has " <<piece_no << " piece\n";
            
        }
        else{
            threadlock.lock();
            pstruct.pieces_to_download.push_back(piece_no);
            threadlock.unlock();
            continue;
        }

        
        peer_send_str="";


        for(int i=0;i<no_of_blocks;i=i+QUEUESIZE) 
        {   for(int j=0;j<QUEUESIZE && (i+j)< no_of_blocks;j++) // VANILLA QUEING 
            {
                if(piece_no==trk.no_of_pieces-1 &&(trk.length%trk.pieceLength)!=0 &&
                (i+j+1)==((trk.length%trk.pieceLength)/BLOCKSIZE)+1)
                {   
                    blocker=trk.length%BLOCKSIZE==0?BLOCKSIZE:trk.length%BLOCKSIZE;
                    peer_send_str+=createRequestMessage(piece_no,BLOCKSIZE*(i+j),blocker);
                    isLastPiece==1;
                    break;
                    
                }

                else blocker=BLOCKSIZE;
                peer_send_str+=createRequestMessage(piece_no,BLOCKSIZE*(i+j),blocker);
            }
                
            //std::cout<<"Send Message piece no. "<<byte_to_number(peer_send_str.substr(5,4))<<std::endl;
            threadlock.lock();
            if(peerdata.no_of_havesent!=pstruct.havemesslist.size() && pstruct.havemesslist.size()>peerdata.no_of_havesent )
            {
                int z=peerdata.no_of_havesent;
                for(int i=0;i<pstruct.havemesslist.size()-z;i++)
                {
                    peer_send_str+=createHaveMessage(pstruct.havemesslist.at(pstruct.havemesslist.size()-i-1));
                    peerdata.no_of_havesent++;
                }
            }
            threadlock.unlock();
            if(send(peerdata.peer_sock_id,peer_send_str.c_str(), peer_send_str.size(),MSG_NOSIGNAL )!=-1)
            {
                peer_recv_str="";
               // std::cout<<"Sent data to peer successfully "<<i+1<<" "<<std::endl;
                    while ((n = recv(peerdata.peer_sock_id, buffer_peer, sizeof(buffer_peer), 0)) > 0)
                    {   
                        //std::cout<<"Received block from peer : "<<n<<std::endl;
                        peer_recv_str.append(buffer_peer, n);
                    }
              status =  processMessage(peer_recv_str,peerdata,bit_data,file,trk);          
            }
            else if(errno==EPIPE){std::cout<<" Failed to send INTERESTED MESSAGE\n";
                std::cout<<"Closing Connection Problem with Connection, writing to closed connection \n ";
                close(peerdata.peer_sock_id);
                threadlock.lock();
                pstruct.pieces_to_download.push_back(piece_no);
                threadlock.unlock();
                //pstruct.peer_ip_port_map.erase(hostname); //MAYBE can try again in the next iteration of connections
        
        return FAILURE;}
            else{
                std::cout<<" Failed to send INTERESTED MESSAGE\n";
                std::cout<<"Closing Connection Problem with Connection, failed to send interested message\n ";
                close(peerdata.peer_sock_id);
                threadlock.lock();
                pstruct.pieces_to_download.push_back(piece_no);
                threadlock.unlock();
                //pstruct.peer_ip_port_map.erase(hostname); //MAYBE can try again in the next iteration of connections
        
                return FAILURE;

           }
            if(status == FAILURE || isLastPiece ==1)
            {
                break;
            }
        }
        if(status == FAILURE )
            {
                
                threadlock.lock();
                pstruct.pieces_to_download.push_back(piece_no);
                threadlock.unlock();
                continue;
            }
        for(int i=0;i<peerdata.block_placeholder.size();i++)
        {
            block_piece.append(peerdata.block_placeholder.at(i));

        }
        std::fill(peerdata.block_placeholder.begin(), peerdata.block_placeholder.end(), "");
        std::cout<<"Downloaded Block Size Length: "<<block_piece.size()<<" Piece Length: "<<trk.pieceLength<<std::endl;
       if(piece_no==trk.no_of_pieces-1 &&(trk.length%trk.pieceLength)!=0)
       {
            
            std::cout<<block_piece.size()<<" : "<< blocker<<" "<<((double)(trk.length%trk.pieceLength)/BLOCKSIZE)*BLOCKSIZE+blocker<<"\n";
            if(block_piece.size()!=((trk.length%trk.pieceLength)/BLOCKSIZE)*BLOCKSIZE+blocker)
            {
                
                threadlock.lock();
                pstruct.pieces_to_download.push_back(piece_no);
                threadlock.unlock();
                continue;
            }
       } 
       else{

        if(block_piece.size()!=trk.pieceLength)
        {
            threadlock.lock();
            pstruct.pieces_to_download.push_back(piece_no);
            threadlock.unlock();
            continue;
        }

       }
         

        SHA1Create(block_piece,block_piece.size(),block_hash);
        std::string check(reinterpret_cast<char*>(block_hash),20);
        if(check==pstruct.pieces_hash_vec[piece_no])
        {
            //std::cout<<"Lets go Allez\n";
            writePieceToFile(file,piece_no,block_piece,trk);
            threadlock.lock();
            url.downloaded++;
            pstruct.havemesslist.push_back(piece_no);
            threadlock.unlock();
            pstruct.pieces_disk[piece_no]=1;
            peerdata.downloaded_peer++;
            std::cout<<"DOWLOADED: "<<url.downloaded<<std::endl;
            //SEND OUT A HAVE MESSAGE TO ALL THE PEERS 
        }
        else
        {
        threadlock.lock();
        pstruct.pieces_to_download.push_back(piece_no);
        threadlock.unlock();
        continue;
        
        }
        current=std::chrono::system_clock::now();
        if(std::chrono::duration_cast<std::chrono::milliseconds>(current-peerdata.latest_comms).count()>120000)//120000 ms is 2min in seconds
        {
            std::cout<<"More than 2 minutes without comms, dropping connection with peer\n";
            std::cout<<"Closing Connection lack of comms\n";
            close(peerdata.peer_sock_id);
            //pstruct.peer_ip_port_map.erase(hostname);
            return FAILURE;        
        }
      

    }
    
}
    //SEND NOT INTERESTED MESSAGE
    peer_send_str="";
    threadlock.lock();
    
    if(peerdata.no_of_havesent!=pstruct.havemesslist.size() && pstruct.havemesslist.size()>peerdata.no_of_havesent )
    {
        int z=peerdata.no_of_havesent;
        for(int i=0;i<pstruct.havemesslist.size()-z;i++)
        {
            peer_send_str+=createHaveMessage(pstruct.havemesslist.at(pstruct.havemesslist.size()-i-1));
            peerdata.no_of_havesent++;
        }
    }
    threadlock.unlock();
    peer_send_str+=createLen1Message(NOT_INTERESTED);
    send(peerdata.peer_sock_id,peer_send_str.c_str(), peer_send_str.size(),MSG_NOSIGNAL );
    if(errno==EPIPE){
        std::cout<<" Failed to send INTERESTED MESSAGE\n";
        std::cout<<"Closing Connection Problem with Connection, writing to closed connection \n ";
        close(peerdata.peer_sock_id);
        return FAILURE;
    }
    
    std::cout<<"Closing Connection\n";


    close(peerdata.peer_sock_id);
    return SUCCESS;
}





























