#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <xbt/log.h>

#include "constants.hpp"

typedef std::string node_name;
typedef uint64_t packet_id;

enum class NodeRole
{
    Aggregator,
    Trainer,
    // Proxy
};

struct NodeInfo
{
    node_name name;
    NodeRole role;
};

using NodeFilter = std::function<bool(NodeInfo*)>;

namespace Filters {
    static bool trainers(NodeInfo *node_info)
    {
        return node_info->role == NodeRole::Trainer;
    }

    static bool trainers_and_aggregators(NodeInfo *node_info)
    {
        return node_info->role == NodeRole::Trainer || 
               node_info->role == NodeRole::Aggregator;
    }
}

class Packet 
{
public: 
    /* --------------------- Operation and their data to be stored in variant --------------------- */
    /* Aggregator operations */
    struct RegistrationConfirmation 
    { 
        std::shared_ptr<std::vector<NodeInfo>> node_list; // list of nodes attributed by the aggregator.
        // static constexpr std::string_view op_name = "REGISTRATION_CONFIRMATION\0";
        static constexpr std::string_view op_name = "\x1B[33mREGISTRATION_CONFIRMATION\033[0m\0";
    };
    struct SendGlobalModel
    {
        uint8_t number_local_epochs; // number of local epochs the trainer should perform.
        // static constexpr std::string_view op_name = "SEND_GLOBAL_MODEL\0";
        static constexpr std::string_view op_name = "\x1B[34mSEND_GLOBAL_MODEL\033[0m\0";
    };
    struct KillTrainer 
    {
        // static constexpr std::string_view op_name = "KILL_TRAINER\0";
        static constexpr std::string_view op_name = "\x1B[31mKILL_TRAINER\033[0m\0";
    };

    /* Trainer operations */
    struct RegistrationRequest 
    { 
        NodeInfo node_to_register; // the node that the aggregator should register.
        // static constexpr std::string_view op_name = "REGISTRATION_REQUEST\0";
        static constexpr std::string_view op_name = "\x1B[33mREGISTRATION_REQUEST\033[0m\0";
    };
    struct SendLocalModel 
    {
        // static constexpr std::string_view op_name = "SEND_LOCAL_MODEL\0";
        static constexpr std::string_view op_name = "\x1B[32mSEND_LOCAL_MODEL\033[0m\0";
    };
    /* -------------------------------------------------------------------------------------------- */ 

    // Definition of our Operation variant
    typedef std::variant<
        RegistrationConfirmation,
        SendGlobalModel,
        KillTrainer,
        RegistrationRequest,
        SendLocalModel
    > Operation;
    /** Compute the simulated size of a packet by following the pointers stored in the data union */
    uint64_t get_packet_size();

    /** Get the printable name of Packet's Operation */
    const char* get_op_name() const;
    
    const Operation op;

    /** Const src and dst */
    node_name original_src;
    node_name final_dst;

    /** Writable src and dst, usefull in a peer-to-peer scenario */
    node_name src;
    node_name dst;

    /** Flag indicating if the packet should be broadcasted */
    const bool broadcast;

    /** NodeFilter used when broadcast flag is enabled */
    const std::optional<NodeFilter> filter;

    /** Unique packet identifier */
    packet_id id = 0;

    /** Clone a packet. Note that pointers in the data variant are also cloned, thus the pointed value will be accessible
     * both by the cloned packet and the original one. */
    Packet *clone();

    /** Packet constructor both final and intermediate destination, used for peer-to-peer communications with several hops */
    Packet(node_name dst, node_name final_dst, Operation op);

    /** Broadcast packet constructor taking NodeFilter instead of concrete destinations */
    Packet(NodeFilter filter, Operation op);

    ~Packet() {}
private:
    /** Intialize fields of a packet */
    void init();

    /** Use to generate new packet ids */
    static inline packet_id total_packet_number = 0;

    /** "cache" variable to prevent computing the packet's size multiple times */
    uint64_t packet_size = 0;
};

#endif // !FALAFELS_PROTOCOL_HPP
