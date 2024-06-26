#include <cstdint>
#include <format>
#include <optional>
#include <variant>
#include <xbt/asserts.h>
#include <xbt/log.h>

#include "protocol.hpp"
#include "utils/utils.hpp"


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_protocol, "Messages specific for this example");

using namespace std;
using namespace protocol;
using namespace protocol::operations;

Packet::Packet(node_name dst, node_name final_dst, Operation op) : 
    dst(dst), final_dst(final_dst), op(op), broadcast(false)
{ 
    this->init();
}

Packet::Packet(filters::NodeFilter target_filter, Operation op) : 
    target_filter(target_filter), op(op), broadcast(true)
{
    this->init();
}

void Packet::init()
{
    this->id = this->total_packet_number;
    this->total_packet_number += 1;
}

/**
 * Compute the simulated size of a packet:
 * - The "real" memory used in the structure
 * - The simulated memory, such as MODEL_SIZE_BYTES
 *
 * @return The simulated size in bytes.
 */
uint64_t Packet::get_packet_size()
{
    // If the packet size haven't been computed yet.
    if (this->packet_size == 0) 
    {
        uint64_t result = 
            sizeof(char) * 32 + // Size of op name
            this->src.size() +
            this->dst.size() +
            this->original_src.size() +
            this->final_dst.size() +
            sizeof(this->id);

        std::visit(overloaded {
            [result](RegistrationConfirmation op) mutable
            {
                result += sizeof(NodeInfo) * op.node_list->size();
            },
            [result](SendGlobalModel op) mutable
            {
                result += Constants::MODEL_SIZE_BYTES + sizeof(uint8_t);
            },
            [result](Kill op) mutable
            {
                // No arguments...
            },
            [result](RegistrationRequest op) mutable
            {
                result += sizeof(NodeInfo);
            },
            [result](SendLocalModel op) mutable
            {
                result += Constants::MODEL_SIZE_BYTES;
            }
        }, this->op);

        this->packet_size = result;
    }

    return this->packet_size;
}

const char *Packet::get_op_name() const
{
    return std::visit(
        // Match every variant type because they all have op_name field
        [](auto op) -> const char *
        {
            return op.op_name.data();
        }, 
        this->op
    );
}

void Packet::increment_hops()
{
    if (!this->seal_hops)
        this->nb_hops++;
}

// void Packet::attach_broadcast_filter(BroadcastOpTable f)
// {
//     this->broadcast_filter = f(this->op);
// }

/**
 * Clone a packet
 */
Packet *Packet::clone()
{
    Packet *res;

    if (this->target_filter)
    {
        res = new Packet(*this->target_filter, this->op);
    }
    else
    {
        res = new Packet(this->dst, this->final_dst, this->op);
    }

    res->id = this->id;
    res->packet_size = this->packet_size;
    res->nb_hops = this->nb_hops;
    res->seal_hops = this->seal_hops;
    res->broadcast_filter = this->broadcast_filter;

    // Decrement the total packet number because a clone isn't considered as a new packet
    Packet::total_packet_number -= 1;
    
    return res;
}
