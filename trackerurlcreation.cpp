#pragma once
#include<string>
#include <iomanip>
#include <sstream>
#include "global.hpp"


std::string convert_to_hex(std::string data,int len)
{   
    char temp[3];
    std::string var_ret;
    std::string inter;
    for( int i(0) ; i < len; ++i )
     {
    if((data[i]<'a'||data[i]>'z')&&(data[i]<'0'||data[i]>'9')&&(data[i]<'A'||data[i]>'Z')&&(data[i]!='.')&&(data[i]!='-')&&(data[i]!='_')&&(data[i]!='~'))
    {   
        inter="";
        sprintf(temp,"%02x",(unsigned char)data[i]); 
        inter=temp;
        var_ret+=inter;
        
    }
    else{
        var_ret+=data[i];
    }
   
     }
     return var_ret;
}
/*std::string hexStr(std::string data, int len)
{
     std::stringstream ss;
     

     for( int i(0) ; i < len; ++i )
     {
        if((data[i]<'a'||data[i]>'z')&&(data[i]<'0'||data[i]>'9')&&(data[i]<'A'||data[i]>'Z')&&(data[i]!='.')&&(data[i]!='-')&&(data[i]!='_')&&(data[i]!='~'))
        {
         ss << std::hex << std::setw(2) << std::setfill('0') << data[i];
        }
        else{
            ss<<data[i];
        }
     }
         

     return ss.str();
}*/

std::string create_url( url_params& info, std::string peerid,std::string announce)
{
    char temp[3];
    std::string strr;
    std::string message=announce+"?info_hash=";
    
    for (int i = 0; i < 20; i++) {
        
         if((info.info_hash[i]<'a'||info.info_hash[i]>'z')&&(info.info_hash[i]<'0'||info.info_hash[i]>'9')
        &&(info.info_hash[i]<'A'||info.info_hash[i]>'Z')&&(info.info_hash[i]!='.')&&(info.info_hash[i]!='-')&&(info.info_hash[i]!='_')
        &&(info.info_hash[i]!='~'))
        {  
            strr="";
            sprintf(temp,"%02x", info.info_hash[i]); 
            strr=temp;
            message+='%'+strr;
        }
        else{
            message+=info.info_hash[i];
        }
    }
  

    
    message+="&peer_id=";
    for(int i=0;i<20;i++)
    {
        if((peerid[i]<'a'||peerid[i]>'z')&&(peerid[i]<'0'||peerid[i]>'9')
        &&(peerid[i]<'A'||peerid[i]>'Z')&&(peerid[i]!='.')&&(peerid[i]!='-')&&(peerid[i]!='_')
        &&(peerid[i]!='~'))
        {
            strr="";
            sprintf(temp,"%02x", peerid[i]); 
            strr=temp;
            message+='%'+strr;
            
        }
        else{
            message+=peerid[i];
        }
    }
    message+="&port="+std::to_string(info.port)+"&uploaded="+std::to_string(info.uploaded)+"&downloaded="+std::to_string(info.downloaded)
    +"&left="+std::to_string(info.left)+"&compact=0";
    // message+="&port="+std::to_string(info.port)+"&uploaded="+std::to_string(info.uploaded)+"&downloaded="+std::to_string(info.downloaded)
    // +"&left="+std::to_string(info.left)+"&compact=1";
    
    
    return message;
}



int byte_to_number(std::string buffer)
{

    return static_cast<int>(static_cast<unsigned char>(buffer[0]) << 24 |
            static_cast<unsigned char>(buffer[1]) << 16 |
            static_cast<unsigned char>(buffer[2]) << 8 |
            static_cast<unsigned char>(buffer[3]));
}

std::string number_to_byte(int n)
{
     char byte[4];
    byte[3] = n & 0x000000ff;
    byte[2] = ( n & 0x0000ff00 ) >> 8;
    byte[1] = ( n & 0x00ff0000 ) >> 16;
    byte[0] = ( n & 0xff000000 ) >> 24; 

    std::string ret( byte,4);
    return ret;
}

std::string number_to_2byte(int n)
{
     char byte[2];
    byte[1] = n & 0x000000ff;
    byte[0] = ( n & 0x0000ff00 ) >> 8;


    std::string ret( byte,2);
    return ret;
}

std::string number_to_8byte( uint64_t num)
{
    char bytes[8];
    int j=0;
    for(int i=7;i>=0;i--,j++)
    {
        bytes[i] = (num >> (j*8)) &  0xFF;
    }
   
    std::string ret( bytes,8);
    return ret;
}
