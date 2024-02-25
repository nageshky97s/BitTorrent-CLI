#pragma once
#include <cstring> 
#include <unordered_map>
#include "bencode.hpp"
#include "global.hpp"
#include "trackerurlcreation.cpp"
#include "tcpConnection.cpp"

void handleTracker(std::string& peerid_string,url_params& url_info,tracker_info& trk_info,std::unordered_map<std::string,int>& peer_ip_port)
{
    int tracsockid;
    int n;
    std::string ip;
    int portno;
    char buffer_get[4096];
    std::string announce_string;
    int no_of_bytes;
    std::string peer_list_str;  
    int z=0;

    std::string url=create_url(url_info,peerid_string,trk_info.announce);
    std::cout<<url<<std::endl;
   
    tracsockid=tcpConnect(trk_info.port,url.substr(url.find('/')+2,url.find(':',(url.find(':')+1))-url.find('/')-2),unresolvedss);

    // std::string get_http="GET "+url.substr(url.find('/',url.find("//")+2))+" HTTP/1.1\r\nHost: "+url.substr(url.find("//"+2),
    // url.find('/',url.find("//")+2)-url.find("//"+2))+"\r\nConnection: close\r\n\r\n";
    std::string get_http="GET "+url.substr(url.find('/',url.find("//")+2))+" HTTP/1.1\r\nHost: "+url.substr(url.find("//")+2,
    url.find('/',url.find("//")+2)-url.find("//")-2)+"\r\nConnection: close\r\n\r\n";   
    
    
    std::cout<<"HTTP request to tracker:"<< get_http<<std::endl;
    
    if(send(tracsockid,get_http.c_str(), get_http.size(),0 )!=-1)
    {
        std::cout<<"Sent GET request to tracker successfully"<<std::endl;
    }

    while ((n = recv(tracsockid, buffer_get, sizeof(buffer_get), 0)) > 0)
    {
        announce_string.append(buffer_get, n);
    }

    std::cout<<"String Size :"<<announce_string<<std::endl;
    std::cout<<trk_info.port<<std::endl;
    int tempo=announce_string.find(':',announce_string.find("Content-Length"))+2;
    no_of_bytes= stoi(announce_string.substr(tempo,announce_string.find("\r\n",tempo)-tempo));
    std::string peer_data = announce_string.substr(announce_string.find("\r\n\r\n")+4,no_of_bytes);//+4 to skip \r\n\r\n
    close(tracsockid);

    auto peer_decoded = bencode::decode(peer_data);
    
    trk_info.track_interval=std::get<bencode::integer>((std::get<bencode::dict>(peer_decoded)).find("interval")->second);
   
    peer_list_str=std::get<bencode::string>((std::get<bencode::dict>(peer_decoded)).find("peers")->second);
   

    

    while(z<peer_list_str.size())
    {

        ip=std::to_string(((unsigned char)peer_list_str[z]))+'.'+
        std::to_string((unsigned char)peer_list_str[z+1])+'.'+std::to_string((unsigned char)peer_list_str[z+2])+'.'+
        std::to_string((unsigned char)peer_list_str[z+3]);
        portno = int((unsigned char)peer_list_str[z+4]<<8|(unsigned char)peer_list_str[z+5]);
        
        if(peer_ip_port.find(ip)==peer_ip_port.end())
            peer_ip_port.insert({ip,portno});
        
        
        z+=6;
    } 
   
      
}