#pragma once
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <netdb.h>
#include <bitset>

#include "global.hpp"
#include "trackerurlcreation.cpp"
#include "messagesCreate.cpp"
#include "helper.cpp"


int processMessageServer(std::string& message,peer_info& data_peer,bitfield& bit_data,std::vector<std::fstream*>& file,tracker_info& trk,pieces pie,std::string& ret_str,int& pd)
{

    int messagelen;
    int id;   
    ret_str="";
     
    while(message.size()!=0)
    {
        messagelen = byte_to_number(message.substr(0,4));
        int id = message[4];
        
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
                ret_str+=createLen1Message(UNCHOKE);
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
                pd++;

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
                            if(data_peer.pieces_have[j]==1)
                                pd++;
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
            if(pie.pieces_disk[byte_to_number(message.substr(5,4))]==1)
            {
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

            }
            else{
                request_str=createKeepAlive();
            }
             
            
            ret_str+=request_str;    
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
