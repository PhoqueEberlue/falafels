#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <string>
#include <xbt/log.h>

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

struct Packet 
{
    enum Operation
    { 
        SEND_GLOBAL_MODEL, 
        SEND_LOCAL_MODEL,
        KILL_TRAINER,
    } op;

    const std::string src;
};

// It would be interesting to generate this function with a generative macro
static const char* operation_to_str(Packet::Operation op)
{
    switch (op) {
        case Packet::Operation::SEND_GLOBAL_MODEL:
            return "SEND_GLOBAL_MODEL";
        case Packet::Operation::SEND_LOCAL_MODEL:
            return "SEND_LOCAL_MODEL";
        case Packet::Operation::KILL_TRAINER:
            return "KILL_TRAINER";
    }
}

#endif // !FALAFELS_PROTOCOL_HPP
