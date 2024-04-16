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
    const enum Operation
    { 
        SEND_GLOBAL_MODEL, 
        SEND_LOCAL_MODEL,
        KILL_TRAINER,
    } op;

    node_name src;
    node_name dst;

    const node_name original_src;
    const node_name final_dst;

    /* Unique packet identifier */
    packet_id id = 0;

    /** 
     * Additionnal arguments passed as a pointer. This is unrealistic for a real distributed use case.
     * That's why sizeof() shouldn't be used to compute the size of the Packet. Instead use compute_packet_size().
     */
    std::unordered_map<std::string, std::string> *args = nullptr;
    
    std::string op_string;

    Packet(Operation op, node_name src, node_name dst) : op(op), original_src(src), final_dst(dst)
    { 
        this->op_string = operation_to_string(this->op);
        this->src = this->original_src;
        this->id = this->total_packet_number;
        this->total_packet_number += 1;
    }

    Packet(Operation op, node_name src, node_name dst, std::unordered_map<std::string, std::string> *args) : op(op), original_src(src), final_dst(dst), args(args)
    {
        this->op_string = operation_to_string(this->op);
        this->src = this->original_src;
        this->dst = this->final_dst;
        this->id = this->total_packet_number;
        this->total_packet_number += 1;
    }

    void incr_ref_count();
    void decr_ref_count();

    uint64_t get_packet_size();

    Packet *clone();

    // Make the destructor private so we have to call decr_ref_count instead. NOTE: temporary disabled.
    ~Packet();
private:
    static std::string operation_to_string(Packet::Operation op);

    // Use to generate new packet ids
    static inline packet_id total_packet_number = 0;

    uint64_t packet_size = 0;

    // Counting the number of references to this object, by default at 1 when instanciated.
    uint16_t ref_count = 1;

};

#endif // !FALAFELS_PROTOCOL_HPP
