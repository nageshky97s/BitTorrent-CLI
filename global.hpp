#pragma once
#include<string>
#include<vector>
#include<chrono>
#include <unordered_map>
#include <deque>
#include <mutex>
#define BLOCKSIZE 16384
#define QUEUESIZE 16
#define SINGLE 1
#define MULTIPLE 2

enum Status {FAILURE =-2222,BITFIELDFAILURE=-2000,unresolvedss=0,resolvedss =1,SUCCESS=100001};
enum MessageVal {KEEP_ALIVE=-1,CHOKE,UNCHOKE ,INTERESTED,NOT_INTERESTED,HAVE,BITFIELD,REQUEST,PIECE,CANCEL,PORT};

struct tracker_info
{   
    std::string pieces;
    long long pieceLength;
    long long length;
    std::string name;
    std::string announce;
    int no_of_pieces;
    int track_interval;
    int port;
    std::chrono::time_point<std::chrono::system_clock> latestcall;
    std::vector<std::string> announce_list;
    std::vector<int> port_list;
    std::vector<long long>length_list;
    std::vector<std::vector<std::string>>path_list;
    std::vector<long long> len_culmulative;
};

struct url_params
{
    unsigned char info_hash[20] ; 
    int port;
    long long uploaded;
    long long downloaded;
    int compact;
    long left;
};



struct bitfield
{
   int bitfield_len;
   int spare_bits;
};

static int no_of_blocks;
static std::string peerid_string(20,'*');
static int mode;
std::mutex threadlock;

struct peer_info
{
    int peer_sock_id;
    int downloaded_peer=0;
    int uploaded_peer=0;
    std::vector<int> pieces_have;
    int choked_peer;
    int interested_peer;
    int choked_client;
    int interested_client;
    std::chrono::time_point<std::chrono::system_clock> latest_comms;
    std::vector<std::string> block_placeholder;
    int no_of_havesent;
};

struct pieces
{
    std::vector<std::string> pieces_hash_vec;
    std::vector<int> pieces_disk;
    std::deque<int> pieces_to_download;
    std::unordered_map<std::string, int> peer_ip_port_map; // Bunched it with this struct even though it does not belong here 
    std::vector<int> havemesslist;

};