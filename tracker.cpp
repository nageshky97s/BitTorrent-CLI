#pragma once
#include <cstring> 
#include <unordered_map>
#include <algorithm>
#include <string>
#include <random>
#include "bencode.hpp"
#include "global.hpp"
#include "trackerurlcreation.cpp"
#include "tcpConnection.cpp"
#include "udpConnection.cpp"
#include "helper.cpp"


void udpTracker(std::string& peerid_string,url_params& url_info,tracker_info& trk_info,std::unordered_map<std::string,int>& peer_ip_port,std::string address)
{

    struct sockaddr_in servaddr;
    socklen_t len;    
    std::string send_mess_udp;
    std::string recv_mess_udp;
    int n;
    char buffer_udp[4096];
    std::string ip;
    int portno;

    int event = 0;
    int num_want =-1 ;
    int ipee =0;
    int key =0 ;
    
    
    std::cout<<"The Address: "<<address.substr(address.find('/')+2,address.find(':',address.find(':')+1)-address.find(':')-3)<<std::endl;
    std::cout<<"The Port: "<<address.substr(address.find(':',address.find(':')+1)+1,address.find('/',address.find(':',address.find(':')+1))-address.find(':',address.find(':')+1)-1)
    <<std::endl;

    int sockudp=udpConnect(address.substr(address.find('/')+2,address.find(':',address.find(':')+1)-address.find(':')-3)
    ,stoi(address.substr(address.find(':',address.find(':')+1)+1,address.find('/',address.find(':',address.find(':')+1))-address.find(':',address.find(':')+1)-1)),
    unresolvedss,servaddr);
    

    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<uint32_t> dist( std::numeric_limits<uint32_t>::min(), std::numeric_limits<uint32_t>::max());

   
    uint32_t trans_id = dist(mt);
    uint32_t action =0;
    uint64_t prot_id = 0x41727101980;
    std::string conn_id;

    struct timeval timeout;
    timeout.tv_sec =1;
    timeout.tv_usec = 0;
    
    
    send_mess_udp = number_to_8byte(prot_id)+number_to_byte(action)+number_to_byte(trans_id) ;
    std::cout<<"Trans num: "<< number_to_byte(trans_id)<<std::endl;
    std::cout<<send_mess_udp<<std::endl<<send_mess_udp.size()<<std::endl;
    
    if(sendto(sockudp,send_mess_udp.c_str(),send_mess_udp.size(),0, (const struct sockaddr *) &servaddr,  
            sizeof(servaddr))!=-1)
    {
        std::cout<<"Message Sent\n";
    }
    else{
        std::cout<<"Failed to send message, tryign next Tracker\n";
        
        return;
    }
    if (setsockopt(sockudp, SOL_SOCKET, SO_RCVTIMEO,&timeout,sizeof(timeout)) < 0) {
    perror("Error");
    }
    
    n=recvfrom(sockudp,buffer_udp,sizeof(buffer_udp),0, (struct sockaddr *) &servaddr,&len);
    if(n>0){
        recv_mess_udp.append(buffer_udp,n);
    }
    else{
        std::cout<<"No data received, next Tracker\n";
    }
    
    std::cout<<"Recv Message: "<<recv_mess_udp<<std::endl;
    std::cout<<"Recv message size: "<<recv_mess_udp.size()<<std::endl; 

    if(recv_mess_udp.size()>=16 && recv_mess_udp.substr(0,4)==number_to_byte(action) && recv_mess_udp.substr(4,4)==number_to_byte(trans_id))
    {
        
        trans_id = dist(mt);
        conn_id = recv_mess_udp.substr(8,8);
        action = 1;
        send_mess_udp = conn_id+number_to_byte(action)+number_to_byte(trans_id)+std::string(reinterpret_cast<char*>(url_info.info_hash))+peerid_string+
        number_to_8byte(url_info.downloaded)+number_to_8byte(url_info.left)
        +number_to_8byte(url_info.uploaded)+number_to_byte(event)+number_to_byte(ipee)+number_to_byte(key)+number_to_byte(num_want)+number_to_2byte(url_info.port); 
        std::cout<<"Announce message size: "<<send_mess_udp.size()<<std::endl;
        if(sendto(sockudp,send_mess_udp.c_str(),send_mess_udp.size(),0, (const struct sockaddr *) &servaddr,  
        sizeof(servaddr))!=-1)
        {
            std::cout<<"Message Sent\n";
        }
        else{
            std::cout<<"Failed to send message\n";
            close(sockudp);
            return;
        }

        recv_mess_udp = "";
        n=recvfrom(sockudp,buffer_udp,sizeof(buffer_udp),0, (struct sockaddr *) &servaddr,&len);
         if(n>0){
        recv_mess_udp.append(buffer_udp,n);
          }
         else{
            std::cout<<"No data received, next Tracker\n";
            }
        
        std::cout<<"Recv Message: "<<recv_mess_udp<<std::endl;
        std::cout<<"Recv message size: "<<recv_mess_udp.size()<<std::endl;

        if(recv_mess_udp.size()>=20 && recv_mess_udp.substr(0,4)==number_to_byte(action) && recv_mess_udp.substr(4,4)==number_to_byte(trans_id))
        {
            std::cout<<"Interval: "<<byte_to_number(recv_mess_udp.substr(8,4))<<std::endl;
            std::cout<<"No of leeches : "<<byte_to_number(recv_mess_udp.substr(12,4))<<std::endl;
            std::cout<<"No of seeds: "<<byte_to_number(recv_mess_udp.substr(16,4))<<std::endl;
            recv_mess_udp = recv_mess_udp.substr(20);
            int z=0;
            while(z+6<=recv_mess_udp.size())
            {
               
                    ip = std::to_string(((unsigned char)recv_mess_udp[z]))+'.'+
                    std::to_string((unsigned char)recv_mess_udp[z+1])+'.'+std::to_string((unsigned char)recv_mess_udp[z+2])+'.'+
                    std::to_string((unsigned char)recv_mess_udp[z+3]);
                    portno = int((unsigned char)recv_mess_udp[z+4]<<8|(unsigned char)recv_mess_udp[z+5]);
                    if(peer_ip_port.find(ip)==peer_ip_port.end())
                        peer_ip_port.insert({ip,portno});
                    z+=6;

                
            }
            
            
        }
        else{
            std::cout<<"Problem with Tracker\n";
            close(sockudp);
            return;
        }


        
    }
    else{
            std::cout<<"Problem with Tracker\n";
            close(sockudp);
            return;
        }

    close(sockudp);




}

void httpTracker(std::string& peerid_string,url_params& url_info,tracker_info& trk_info,std::unordered_map<std::string,int>& peer_ip_port,std::string annouce_str)
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

    std::string url=create_url(url_info,peerid_string,annouce_str);
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
    //Check for success message, if successful continue else return 
    if(!(findStringIC(announce_string,"200 OK")))
    {
        std::cout<<"Problem with tracker, did not receive SUCCESS Message and peer list\n";
        return;
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
   

    

    while(z+6<=peer_list_str.size())
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


void handleTracker(std::string& peerid_string,url_params& url_info,tracker_info& trk_info,std::unordered_map<std::string,int>& peer_ip_port)
{
    
    if(std::find(trk_info.announce_list.begin(),trk_info.announce_list.end(),trk_info.announce)==trk_info.announce_list.end())
    {
        trk_info.announce_list.emplace_back(trk_info.announce);
    }
    for(auto a:trk_info.announce_list)
    {
        if(a.find("udp")!=std::string::npos)
        {
            udpTracker(peerid_string,url_info,trk_info,peer_ip_port,a);
        }
        else if(a.find("http")!=std::string::npos){

            httpTracker(peerid_string,url_info,trk_info,peer_ip_port,a);
        }
    }
    
    if(peer_ip_port.size()==0)
    {
        std::cout<<"No Peers obtained, might be an issue with the tracker \n";
        exit(0);
    }
   
      
}