#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
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
    /** Operation for the application layer. */
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

    /** Operation stored as a nice string. Automatically assigned when calling the constructor */
    std::string op_string;

    /** Possible values contained in a Packet. The actual value is indicated by the Operation enum. */
    const union Data {
        /** REGISTRATION_REQUEST: the node that the aggregator should register. */
        NodeInfo node_to_register; 
        /* REGISTRATION_CONFIRMATION: list of nodes attributed by the aggregator. */
        std::vector<NodeInfo> *node_list; 
        /** SEND_GLOBAL_MODEL: number of local epochs the trainer should perform. */
        uint8_t number_local_epochs;
    } *data;

    /** Compute the simulated size of a packet by following the pointer in the data union */
    uint64_t get_packet_size();
 
    /** Const src and dst */
    const node_name original_src;
    const node_name final_dst;

    /** Writable src and dst, usefull in a peer-to-peer scenario */
    node_name src;
    node_name dst;

    /** Unique packet identifier */
    packet_id id = 0;

    /** Clone a packet WITHOUT cloning the data union itself */
    Packet *clone();

    Packet(node_name src, node_name dst, Operation op, Data *data=nullptr);

    ~Packet();
private:
    /** Method that generates a nice representation of Operation enum with colors. */
    static std::string operation_to_string(Packet::Operation op);

    /** Use to generate new packet ids */
    static inline packet_id total_packet_number = 0;

    /** "cache" variable to prevent computing the packet's size multiple times */
    uint64_t packet_size = 0;
};

#endif // !FALAFELS_PROTOCOL_HPP
