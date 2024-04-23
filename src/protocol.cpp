#include "protocol.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <xbt/asserts.h>
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_protocol, "Messages specific for this example");

Packet::Packet(node_name src, node_name dst, Operation op) : 
    original_src(src), final_dst(dst), op(op)
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
            [result](KillTrainer op) mutable
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
    }

    return this->packet_size;
}

const char *Packet::get_op_name()
{
    return std::visit(overloaded {
        // Match every variant type because they all have op_name field
        [](auto op) -> const char * 
        {
            return op.op_name.data();
        },
    }, this->op);
}

/**
 * Clone a packet
 */
Packet *Packet::clone()
{
    Packet *res = new Packet(this->original_src, this->final_dst, this->op);
    res->id = this->id;
    res->packet_size = this->packet_size;

    // Decrement the total packet number because a clone isn't considered as a new packet
    Packet::total_packet_number -= 1;
    
    return res;
}
