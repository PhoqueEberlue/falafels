#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <cstdint>
#include <string>
#include <xbt/log.h>
#include <unordered_map>
#include "constants.hpp"

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

    /** 
     * Additionnal arguments passed as a pointer. This is unrealistic for a real distributed use case.
     * That's why sizeof() shouldn't be used to compute the size of the Packet. Instead use compute_packet_size().
     */
    const std::unordered_map<std::string, std::string> *args = nullptr;
};


/**
 * Compute the simulated size of a packet:
 * - The "real" memory used in the structure
 * - The simulated memory, such as MODEL_SIZE_BYTES
 *
 * @param Packet pointer
 * @return The simulated size in bytes.
 */
static const uint64_t compute_packet_size(Packet *p)
{
    uint64_t result = sizeof(Packet::Operation) + p->src.size();

    // Add the model size if its a send model operation
    if (p->op == Packet::SEND_GLOBAL_MODEL || p->op == Packet::SEND_LOCAL_MODEL)
        result += constants::MODEL_SIZE_BYTES;

    if (p->args)
    {
        for (const auto&[key, value]: *p->args)
        {
            result += key.size() + value.size();
        }
    }

    return result;
}


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
