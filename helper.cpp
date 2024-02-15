#pragma once

#include <random>
#include <string>
#include <openssl/sha.h>
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include "bencode.hpp"
#include "global.hpp"

void SHA1Create(std::string& data,int len,unsigned char* dest)
{
    
    SHA_CTX shactx;
    SHA1_Init(&shactx);
    SHA1_Update(&shactx, data.c_str(), len);
    SHA1_Final(dest, &shactx);

    //SHA1(data, len, dest);


}


// void parseMetaFile(tracker_info& trk_info,std::stringstream& buffer,std::string& info_str)
// {
//     mode=SINGLE;
//     auto value= std::get<bencode::dict>(bencode::decode(buffer));
//     trk_info.announce=std::get<bencode::string>(value.find("announce")->second);
//     trk_info.pieceLength=std::get<bencode::integer>(std::get<bencode::dict>((value.find("info")->second)).find("piece length")->second);
//     trk_info.name=std::get<bencode::string>(std::get<bencode::dict>((value.find("info")->second)).find("name")->second);
//     trk_info.length=std::get<bencode::integer>(std::get<bencode::dict>((value.find("info")->second)).find("length")->second);
//     trk_info.pieces=std::get<bencode::string>(std::get<bencode::dict>((value.find("info")->second)).find("pieces")->second);
//     trk_info.no_of_pieces=ceil((double)trk_info.length/trk_info.pieceLength);
//     info_str = bencode::encode(std::get<bencode::dict>((value.find("info")->second)));
//     int readable=trk_info.announce.find(':',trk_info.announce.find(':')+1);
//     //std::cout<<trk_info.announce<<std::endl;
//     trk_info.port =std::stoi(trk_info.announce.substr(readable+1,trk_info.announce.find('/',readable)-readable-1));
    
// }

void parseMetaFile(tracker_info& trk_info,std::stringstream& buffer,std::string& info_str)
{
    
    auto value= std::get<bencode::dict>(bencode::decode(buffer));
    
    //Common to bith Single File Mode and Multiple File Mode
    trk_info.pieceLength=std::get<bencode::integer>(std::get<bencode::dict>(value["info"])["piece length"]);
    trk_info.pieces=std::get<bencode::string>(std::get<bencode::dict>(value["info"])["pieces"]);
    
    trk_info.length=0;
    info_str = bencode::encode(std::get<bencode::dict>((value["info"])));
    
    try{

        trk_info.announce=std::get<std::string>(value["announce"]);
        int readable=trk_info.announce.find(':',trk_info.announce.find(':')+1);
        trk_info.port =std::stoi(trk_info.announce.substr(readable+1,trk_info.announce.find('/',readable)-readable-1));
        
        for(auto i:std::get<bencode::list>(value["announce-list"]))
        {
            trk_info.announce_list.emplace_back(std::get<bencode::string>(std::get<bencode::list>(i)[0]));
            readable=trk_info.announce_list.back().find(':',trk_info.announce_list.back().find(':')+1);
            trk_info.port_list.emplace_back(std::stoi(trk_info.announce_list.back().substr(readable+1,trk_info.announce_list.back().find('/',readable)-readable-1)));
                
        } 

    }
    
   catch(const std::exception& e)
   {
    std::cout<<"No Announce list present\n";
   }



    //Figure Out if it is Single File Mode or Multiple File Mode

    try {

        mode=MULTIPLE;

        
        std::cout<<"Multiple File Mode\n";
        
        for(auto i:std::get<bencode::list>(std::get<bencode::dict>(value["info"])["files"]))
        {
            
            trk_info.length_list.emplace_back(std::get<bencode::integer>(std::get<bencode::dict>(i)["length"]));
            trk_info.length+=trk_info.length_list.back();
            std::vector<std::string> temp;            
            for(auto w:std::get<bencode::list>(std::get<bencode::dict>(i)["path"]))
            {
                temp.emplace_back(std::get<bencode::string>(w));
            }
            trk_info.path_list.emplace_back(temp);
        }
        trk_info.name=std::get<bencode::string>(std::get<bencode::dict>(value["info"])["name"]);
        

    }

    catch(const std::exception& e)
   {
    mode=SINGLE;
    std::cout<<"Single File Mode\n";
    trk_info.name=std::get<bencode::string>(std::get<bencode::dict>(value["info"])["name"]);
    trk_info.length=std::get<bencode::integer>(std::get<bencode::dict>(value["info"])["length"]);
       
       
   }
    trk_info.no_of_pieces=ceil((double)trk_info.length/trk_info.pieceLength);
    
}
void createRandomName(std::string& name)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis('a','z');
    std::generate(name.begin(), name.end(), std::bind(dis, gen));

}


void writePieceToFile( std::vector<std::fstream*>& file, int position,std::string& piece,tracker_info& trk)
{
    no_of_blocks=16;
    
    if(mode==SINGLE)
    {
        file[0]->seekp(position*BLOCKSIZE*16,std::ios::beg);
        file[0]->write(piece.c_str(),piece.size());
    }
    else
    {
        long long start=position*no_of_blocks*BLOCKSIZE;        
        long long end = start+piece.size()-1;
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
        while(piece.size()!=0)
        {
            
            if(end<trk.len_culmulative[start_file+1])
            {
                
                file[start_file]->seekp(start,std::ios::beg);
                file[start_file]->write(piece.c_str(),piece.size());
                piece=piece.substr(piece.size());
            }
            else{
                
                file[start_file]->seekp(start,std::ios::beg);
                file[start_file]->write(piece.substr(0,trk.length_list[start_file]-start).c_str(),trk.length_list[start_file]-start);
                piece=piece.substr(trk.length_list[start_file]-start);
                start_file++;
                start=0;

            }
            
        }

    }
}
    

void dirfilecreation(tracker_info& trk_info,std::vector<std::fstream*>& file)

{
       
    
    if (mode == SINGLE)
    {   
        (void) std::ofstream(trk_info.name, std::ostream::app);
        std::fstream* file_write = new std::fstream();
        file_write->open(trk_info.name);
        if(!file_write)
        {
            std::cout<<"File failed to open!\n";
        }
        else{
            std::cout<<"FILE SUCCESSFULLY OPEN\n";
            file.emplace_back(file_write);
        }
        

    }
    else
    {
        for(auto a:trk_info.path_list)
        {
            std::string path;
            for(int i=0;i<a.size()-1;i++)
            {
                path+=a.at(i)+"/";
            }
            if(trk_info.name!=a[0])
            {
                path=trk_info.name+"/"+path;
            }
            
            
            std::filesystem::create_directories(path);
            (void) std::ofstream(path+a.back(), std::ostream::app);
            std::fstream* file_write = new std::fstream();
            file_write->open(path+a.back());
            file.emplace_back(file_write);
        }
        trk_info.len_culmulative.emplace_back(0);
        for(auto a: trk_info.length_list)
        {
            trk_info.len_culmulative.emplace_back(a+trk_info.len_culmulative.back());
        }

    }
    
}
