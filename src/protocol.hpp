#ifndef FALAFELS_PROTOCOL_HPP
#define FALAFELS_PROTOCOL_HPP

#include <cstdint>
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
    /* --------------------- Operation and their data to be stored in variant --------------------- */
    /* Aggregator operations */
    struct RegistrationConfirmation 
    { 
        std::shared_ptr<std::vector<NodeInfo>> node_list; // list of nodes attributed by the aggregator.
        static constexpr std::string_view op_name = "REGISTRATION_CONFIRMATION"; // "\x1B[33mREGISTRATION_CONFIRMATION\033[0m\0";
    };
    struct SendGlobalModel
    {
        uint8_t number_local_epochs; // number of local epochs the trainer should perform.
        static constexpr std::string_view op_name = "SEND_GLOBAL_MODEL"; // "\x1B[34mSEND_GLOBAL_MODEL\033[0m\0";
    };
    struct KillTrainer 
    {
        static constexpr std::string_view op_name = "KILL_TRAINER"; // "\x1B[31mKILL_TRAINER\033[0m\0";
    };

    /* Trainer operations */
    struct RegistrationRequest 
    { 
        NodeInfo node_to_register; // the node that the aggregator should register.
        static constexpr std::string_view op_name = "REGISTRATION_REQUEST"; // \x1B[33mREGISTRATION_REQUEST\033[0m\0";
    };
    struct SendLocalModel 
    {
        static constexpr std::string_view op_name = "SEND_LOCAL_MODEL"; // "\x1B[32mSEND_LOCAL_MODEL\033[0m\0";
    };
    /* -------------------------------------------------------------------------------------------- */

    // Tool to use lambdas in std::visit, see: https://en.cppreference.com/w/cpp/utility/variant/visit
    template<class... Ts>
    struct overloaded : Ts... { using Ts::operator()...; };

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
    const node_name original_src;
    const node_name final_dst;

    /** Writable src and dst, usefull in a peer-to-peer scenario */
    node_name src;
    node_name dst;

    /** Unique packet identifier */
    packet_id id = 0;

    /** Clone a packet. Note that pointers in the data variant are also cloned, thus the pointed value will be accessible
     * both by the cloned packet and the original one. */
    Packet *clone();

    Packet(node_name src, node_name dst, Operation op);

    ~Packet() {}
private:

    /** Use to generate new packet ids */
    static inline packet_id total_packet_number = 0;

    /** "cache" variable to prevent computing the packet's size multiple times */
    uint64_t packet_size = 0;
};

#endif // !FALAFELS_PROTOCOL_HPP
