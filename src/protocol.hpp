#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <cstdint>
#include <string>
#include <xbt/log.h>
#include <unordered_map>
#include "constants.hpp"

typedef std::string node_name;

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

    const node_name src;

    /** 
     * Additionnal arguments passed as a pointer. This is unrealistic for a real distributed use case.
     * That's why sizeof() shouldn't be used to compute the size of the Packet. Instead use compute_packet_size().
     */
    const std::unordered_map<std::string, std::string> *args = nullptr;
    
    std::string op_string;

    Packet(Operation op, node_name src) : op(op), src(src)
    { 
        this->op_string = operation_to_string(this->op);
    }

    Packet(Operation op, node_name src, std::unordered_map<std::string, std::string> *args) : op(op), src(src), args(args)
    {
        this->op_string = operation_to_string(this->op);
    }

    void incr_ref_count();
    void decr_ref_count();

    uint64_t get_packet_size();

private:
    static std::string operation_to_string(Packet::Operation op);
    uint64_t packet_size = 0;

    // Counting the number of references to this object, by default at 1 when instanciated.
    uint16_t ref_count = 1;

    // Make the destructor private so we have to call decr_ref_count instead
    ~Packet();
};

#endif // !FALAFELS_PROTOCOL_HPP
