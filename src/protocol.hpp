#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <cstdint>
#include <string>
#include <xbt/log.h>
#include <unordered_map>
#include "constants.hpp"

typedef std::string node_name;
typedef uint64_t packet_id;

enum class NodeRole
{
    Aggregator,
    Trainer,
    Proxy
};


struct NodeInfo
{
    node_name name;
    NodeRole role;
};

class Packet 
{
public:
    /* Operation for the application layer */
    const enum Operation
    { 
        /* Aggregator operations */
        SEND_GLOBAL_MODEL, 
        KILL_TRAINER,
        REGISTRATION_CONFIRMATION,
        /* Trainer operations */
        SEND_LOCAL_MODEL,
        REGISTRATION_REQUEST,
    } op;

    /* Operation stored as a nice string */
    std::string op_string;

    /* Writable src and dst, usefull in a peer to peer scenario */
    node_name src;
    node_name dst;

    /* Const src and dst */
    const node_name original_src;
    const node_name final_dst;

    NodeInfo *node_info = nullptr; 

    /* Unique packet identifier */
    packet_id id = 0;

    /** 
     * Additionnal arguments passed as a pointer. This is unrealistic for a real distributed use case.
     * That's why sizeof() shouldn't be used to compute the size of the Packet. Instead use compute_packet_size().
     */
    std::unordered_map<std::string, std::string> *args = nullptr;
    

    uint64_t get_packet_size();

    Packet *clone();

    Packet(Operation op, node_name src, node_name dst);
    Packet(Operation op, node_name src, node_name dst, std::unordered_map<std::string, std::string> *args);
    ~Packet();
private:
    static std::string operation_to_string(Packet::Operation op);

    // Use to generate new packet ids
    static inline packet_id total_packet_number = 0;

    uint64_t packet_size = 0;
};

#endif // !FALAFELS_PROTOCOL_HPP
