#pragma once
#include <string>
#include <iostream>
#include <chrono>
#include <bitset>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include "global.hpp"
#include "trackerurlcreation.cpp"
#include "messagesCreate.cpp"


int processMessage(std::string& message,peer_info& data_peer,bitfield& bit_data,std::vector<std::fstream*>& file,tracker_info& trk)
{

    int messagelen;
    int n;
    char buffer_peer[4096];
    std::string peer_recv_str="";
    std::string peer_send_str="";
    int id;

    while(message.size()!=0)
    {
        messagelen = byte_to_number(message.substr(0,4));
        id = message[4];
        
        if(messagelen+4>message.size())
        {
            return FAILURE;
        }
        if(messagelen==0)
        {
        std::cout<<"Keep-Alive Message\n";
        //make current time to timer counter 
        data_peer.latest_comms=std::chrono::system_clock::now();
        
        }
        else
        {
            switch(id)
            {
                break;
            //choke
            case CHOKE:
                std::cout<<"Choke Message\n";
                data_peer.choked_client=1;
                
                break;
            //unchoke
            case UNCHOKE:
                std::cout<<"Unchoke Message\n";
                data_peer.choked_client=0;
                break;
            //interested
            case INTERESTED:
                std::cout<<"Interested Message\n";
                data_peer.interested_peer=1;
                break;
            //not interested
            case NOT_INTERESTED:
                std::cout<<"Not-Interested Message\n";
                data_peer.interested_peer=0;
                break;
            //have
            case HAVE: // Done sorta
                std::cout<<"Have Message\n";
                data_peer.pieces_have[byte_to_number(message.substr(5,4))] = 1;

                break;
            //bitfield
            case BITFIELD: //Done sorta 
            {
                std::cout<<"Bitfield Message\n";
                if((messagelen-1)!=bit_data.bitfield_len)
                {
                    std::cout<<"Bitfield Length does not match error\n";
                    return BITFIELDFAILURE;
                }
                int j=0;
                int flag=0;
                std::string temp;
                for (int i=5;i<messagelen+5-1;i++)
                    //for (int i=0;i<messagelen;i++)
                    {
                        temp=std::bitset<8>(message[i]).to_string();
                        //std::cout<<i-4<<" : "<<message[i]<<std::endl;
                        for(int z=0;z<8;z++)
                        {
                            data_peer.pieces_have[j]=temp[z]-'0';
                            j++;
                            if(j==data_peer.pieces_have.size())
                            {
                                flag=1;
                            }
                        }

                        if(flag==1)
                            break;
                    }
                
            }
            
            break;
            //request
        case REQUEST: //To do
        {
            std::cout<<"Request Message\n";
            std::string request_str;
            if((byte_to_number(message.substr(5,4)) == trk.no_of_pieces-1) && 
            (byte_to_number(message.substr(9,4)) == (((trk.length%trk.pieceLength)/BLOCKSIZE)*BLOCKSIZE)))
            {
                int block=trk.length%BLOCKSIZE==0?BLOCKSIZE:trk.length%BLOCKSIZE;
                request_str=createPieceMessage(file,trk,byte_to_number(message.substr(5,4)),byte_to_number(message.substr(9,4)),
                no_of_blocks,block);
                
            }
            else{
                request_str=createPieceMessage(file,trk,byte_to_number(message.substr(5,4)),byte_to_number(message.substr(9,4)),no_of_blocks,BLOCKSIZE);
            }
            
            if(send(data_peer.peer_sock_id,request_str.c_str(), request_str.size(),0 )!=-1)
            
            {       
                std::cout<<"Sent data to peer successfully"<<std::endl;
                data_peer.uploaded_peer++;
               /* peer_recv_str="";
                while ((n = recv(data_peer.peer_sock_id, buffer_peer, sizeof(buffer_peer), 0)) > 0)
                {   
                    std::cout<<"Received data from peer : ";
                    peer_recv_str.append(buffer_peer, n);
                }
                std::cout<<peer_recv_str.size()<<std::endl;
                std::cout<<peer_recv_str<<std::endl;PIECE
                */
            } 
            else{
                std::cout<<"Failed to send to peer\n";
            }      
        
        
        }

            break;
        //piece
        case PIECE:  //To do 
        {
            int blo_no=byte_to_number(message.substr(9,4))/BLOCKSIZE;
           // std::cout<<"Piece Message "<<byte_to_number(message.substr(5,4))<<std::endl;
           data_peer.block_placeholder[blo_no] = message.substr(13,messagelen-9);

        }
            break;
        //cancel
        case CANCEL: //Not sure what to do 
        {
            std::cout<<"Cancel Message, Do Nothing I guess?\n";
        }
            break;
        //port))
        case PORT: //for DHT tracker probably not required 
        {
            std::cout<<"Port Message Received what do?\n";
        }
            break;
        default:
            {
                std::cout<<"Message not part of the protocol being sent\n";
                return FAILURE;
            }
            
            break;
        }

    }

        //std::cout<<"Message Size: "<<message.size()<<std::endl;
        message=message.substr(4+messagelen);
       

    }
    data_peer.latest_comms=std::chrono::system_clock::now();
    return SUCCESS;
}