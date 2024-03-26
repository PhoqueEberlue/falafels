#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <string>
#include <xbt/log.h>
#include <unordered_map>

typedef std::string node_name;

enum class NodeRole {
    Aggregator,
    Trainer,
    Proxy
};


struct NodeInfo
{
    node_name name;
    NodeRole role;
};

// TODO: Provide utility to compute the size of a packet
struct Packet 
{
    enum Operation
    { 
        SEND_GLOBAL_MODEL, 
        SEND_LOCAL_MODEL,
        KILL_TRAINER,
    } op;

    const std::string src;

    /** 
     * Additionnal arguments passed as a pointer. This is unrealistic for a real distributed use case.
     * That's why sizeof() shouldn't be used to compute the size of the Packet.
     */
    const std::unordered_map<std::string, std::string> *args = nullptr;
};

// It would be interesting to generate this function with a generative macro
static const char* operation_to_str(Packet::Operation op)
{
    switch (op) {
        case Packet::Operation::SEND_GLOBAL_MODEL:
            return "\x1B[34mSEND_GLOBAL_MODEL\033[0m";
        case Packet::Operation::SEND_LOCAL_MODEL:
            return "\x1B[32mSEND_LOCAL_MODEL\033[0m";
        case Packet::Operation::KILL_TRAINER:
            return "\x1B[31mKILL_TRAINER\033[0m";
    }
}

#endif // !FALAFELS_PROTOCOL_HPP
