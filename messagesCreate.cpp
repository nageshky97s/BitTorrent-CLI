#pragma once
#include<string>
#include<iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include "trackerurlcreation.cpp"
#include "global.hpp"


std::string createKeepAlive()
{
    char message[4];
    message[0]=message[1]=message[2]=message[3]=(char)0;
    std::string temp(message,4);
    return temp;
}

std::string createLen1Message(int i)
{
   
    char message[5];
    message[0]=message[1]=message[2]=(char)0;
    message[3]=(char)1;
    message[4]=(char)i;
    std::string temp(message,5);
    
    return temp;
}



std::string createHaveMessage(int index){
    char id = HAVE;
    return number_to_byte(5)+id+number_to_byte(index);
}

std::string createRequestMessage(int index,int begin, int len)
{
    return number_to_byte(13)+static_cast<char>(REQUEST)+number_to_byte(index)+number_to_byte(begin)+number_to_byte(len);
}
std::string createCancelMessasge(int index,int begin, int len)
{
    return number_to_byte(13)+static_cast<char>(CANCEL)+number_to_byte(index)+number_to_byte(begin)+number_to_byte(len);
    
}


std::string createPortMessage(int port)// the structure of this message is different 4-byte big endian, 1 byte id, 2 byte port
{

return number_to_byte(3)+static_cast<char>(PORT)+number_to_2byte(port);

}

std::string createBitfieldMessage(int spare_bits, std::vector<int>& pieces) // sent immediately after the handshake is completed,not required of the client has no pieces
{

std::string length= number_to_byte(ceil((double)pieces.size()/8)+1);//+1 is the id length
std::string temp;
std::string out="";
for(int i=0;i<pieces.size()/8;i++)
    {
        temp="";
        temp=std::to_string(pieces[8*i])+std::to_string(pieces[8*i+1])+std::to_string(pieces[8*i+2])+std::to_string(pieces[8*i+3])+
        std::to_string(pieces[8*i+4])+std::to_string(pieces[8*i+5])+std::to_string(pieces[8*i+6])+std::to_string(pieces[8*i+7]);
        out+=static_cast<char>(std::stoull(temp, 0, 10)) ;
        
    }
    if(spare_bits)
    {
        temp="";
        for(int i=0;i<8-spare_bits;i++)
        {
            temp=std::to_string(pieces[pieces.size()-1-i])+temp;
        }
        
        temp.insert(temp.size(), spare_bits, '0');
       
        out+=static_cast<char>(std::stoull(temp, 0, 10)) ;
    }



    return length + static_cast<char>(BITFIELD)+ out;
}




std::string createPieceMessage(std::vector<std::fstream*>& file,tracker_info& trk,int ind, int begin,int num_block,int blocklen) // we explicitly send block length not use BLOCKSIZE because of the variable last block/piece
{

    
    
    if(mode==SINGLE)
    {
        char data[blocklen];
        file[0]->seekg(ind*num_block*BLOCKSIZE+begin,std::ios::beg);
        file[0]->read(data,blocklen);
        std::string temp(data,blocklen);
        return number_to_byte(9+blocklen)+static_cast<char>(PIECE)+number_to_byte(ind)+number_to_byte(begin)+temp;

    }
    else{

        std::string temp;
        long long start=ind*num_block*BLOCKSIZE+begin; 
        long long end = start + blocklen-1;
        int start_file;
        for(int i=0;i<trk.length_list.size();i++)
        {
            if(start<trk.len_culmulative[i+1])
                {
                    start_file=i;
                    break;
                }
        }
        
        start=start-trk.len_culmulative[start_file];
        
        long long counter=0;
        while(counter<blocklen)
        {
            if(end<trk.len_culmulative[start_file+1])
            {
                char data[blocklen-counter];
                file[start_file]->seekg(start,std::ios::beg);
                file[start_file]->read(data,blocklen-counter);
                temp+=std::string(data,blocklen-counter);
                counter+=blocklen-counter;
            }

            else{
                char data[trk.length_list[start_file]-start];
                file[start_file]->seekg(start,std::ios::beg);
                file[start_file]->read(data,trk.length_list[start_file]-start);
                temp+=std::string(data,trk.length_list[start_file]-start);
                counter+=trk.length_list[start_file]-start;
                start=0;
                start_file++;
                
            }
            
        }
        
        return number_to_byte(9+blocklen)+static_cast<char>(PIECE)+number_to_byte(ind)+number_to_byte(begin)+temp;

    }
    
}
