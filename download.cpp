#pragma once
#include <fstream>
#include<iostream>
#include "bencode.hpp"
#include<typeinfo>
#include <openssl/sha.h>
#include <cstring>
#include <algorithm>
#include <functional>
#include <iostream>
#include <fstream>
#include <iterator>
#include <random>
#include <vector>
#include <cmath>
#include <bitset>
#include <unordered_map>
#include <deque>
#include <thread>
#include <mutex>
#include <signal.h>
#include <chrono>

#include "trackerurlcreation.cpp"
#include "global.hpp"
#include "tcpconnection.cpp"
#include "nonblocktcp.cpp"
#include "messagesCreate.cpp"
#include "helper.cpp"
#include "tracker.cpp"
#include "peer.cpp"
#include "server.cpp"


void download_torrent()
{


// Open torrent file which contains info about the torrent to parse
   
    
    std::ifstream t("debian.torrent");    
    
    
    std::stringstream buffer;
    buffer << t.rdbuf();   
    
    std::string info_encode_str;
    

    tracker_info trk_info;
    url_params url_info;
    bitfield bitdata;
    pieces p;
    parseMetaFile(trk_info,buffer,info_encode_str);
    
    no_of_blocks=trk_info.pieceLength/BLOCKSIZE;// GLOBAL variable for no of blocks in a piece
    p.pieces_disk.resize(trk_info.no_of_pieces,0);

    bitdata.bitfield_len= ceil((double)ceil((double)trk_info.length/trk_info.pieceLength)/8);
    bitdata.spare_bits = ceil((double)ceil((double)trk_info.length/trk_info.pieceLength)/8)*8-ceil((double)trk_info.length/trk_info.pieceLength);

    std::cout<<"Bitfield Length: "<<bitdata.bitfield_len<<std::endl;
    std::cout<<"No of spare bits = "<<bitdata.spare_bits<<std::endl;
    std::cout<<"Piece Length= "<<trk_info.pieceLength<<std::endl<<"Length: "<<trk_info.length<<std::endl;
    std::cout<<"No of pieces = "<<trk_info.no_of_pieces<<"  "<<ceil((double)trk_info.length/trk_info.pieceLength)/8<<std::endl;
    

    for(int i=0;i<trk_info.no_of_pieces;i++)p.pieces_to_download.push_back(i);

    

    SHA1Create(info_encode_str,info_encode_str.size(),url_info.info_hash);

    for(int i=0;i<trk_info.pieces.size();i=i+20)
    {  
    p.pieces_hash_vec.emplace_back(trk_info.pieces.substr(i,20));
    
    }
    createRandomName(peerid_string);
    url_info.downloaded=0;
    url_info.uploaded=0;
    url_info.compact=0;
    url_info.port=6969; // This is the port to which other peers while use to connect to our client and is sent as part of the annouce message
    url_info.left=trk_info.length; //This has to be updated after each piece has been successfully downloaded

    std::vector<std::fstream*> files;
    dirfilecreation(trk_info,files);   

     std::vector<std::thread> threads;
    
    signal(SIGPIPE, SIG_IGN);
    int sockfdserv =  createServer(url_info.port);
    
   
     trk_info.latestcall = std::chrono::system_clock::now()-std::chrono::seconds(901);
    

    // int sockpeer = accept(sockfdserv,0,0);
    //  if(sockpeer<0) std::cout<<"Error while accepting connection\n";
    // else uploadToPeer(url_info,bitdata,files,trk_info,p,sockpeer);
   
    bool test2= url_info.downloaded!=trk_info.no_of_pieces;
    
    while(true)
    {
       
         if(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now()-trk_info.latestcall).count()>900 &&
         url_info.downloaded!=trk_info.no_of_pieces)//900000 ms is 15min in seconds
        {
            
             handleTracker(peerid_string, url_info, trk_info, p.peer_ip_port_map);
             trk_info.latestcall = std::chrono::system_clock::now();
            
             for(auto itr=p.peer_ip_port_map.begin();itr!=p.peer_ip_port_map.end();itr++)
            {
            threads.push_back(std::thread(downloadFromPeer,std::ref(trk_info),std::ref(url_info),std::ref(bitdata),itr->second,itr->first,
            std::ref(files),std::ref(p)));
            }
            p.peer_ip_port_map.clear();
        }
        
    }

    for (auto &th : threads)
    {
    th.join();
    }
    
    

   

    
   
    
    // std::cout<<"DOWNLOAD FINISHED KABOOM KA-SHA\n";


    //  for(auto itr=p.peer_ip_port_map.begin();itr!=p.peer_ip_port_map.end();itr++)
    // {
    //     downloadFromPeer(trk_info,url_info,bitdata,itr->second,itr->first,files,p);
    
    // }
    for(auto a:files)
    {
        a->close();
        free(a);
    }
        
    std::cout<<"SUCCESS!!!\n"<<"Total Downloaded: "<<url_info.downloaded<<std::endl<<trk_info.no_of_pieces;


}






















