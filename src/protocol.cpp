#include "protocol.hpp"
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <xbt/asserts.h>
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_protocol, "Messages specific for this example");

Packet::Packet(node_name src, node_name dst, Operation op, const optional<Data> &data) : 
    original_src(src), final_dst(dst), op(op), data(data)
{ 
    this->op_string = operation_to_string(this->op);
    this->id = this->total_packet_number;
    this->total_packet_number += 1;
}


NodeInfo Packet::get_node_to_register()
{
    xbt_assert(holds_alternative<NodeInfo>(*this->data), 
               "Cannot call get_node_to_register() because `shared_ptr<NodeInfo>` wasn't found in the data variant");
    return get<NodeInfo>(*this->data);
}

shared_ptr<vector<NodeInfo>> Packet::get_node_list()
{
    xbt_assert(holds_alternative<shared_ptr<std::vector<NodeInfo>>>(*this->data), 
               "Cannot call get_node_list() because `shared_ptr<vector<NodeInfo>>` wasn't found in the data variant");
    return get<shared_ptr<vector<NodeInfo>>>(*this->data);
}

uint8_t Packet::get_number_local_epochs()
{
    xbt_assert(holds_alternative<uint8_t>(*this->data), 
               "Cannot call get_number_local_epochs() because `uint8_t` wasn't found in the data variant");
    return get<uint8_t>(*this->data);
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
            sizeof(Packet::Operation) + 
            this->original_src.size() +
            this->final_dst.size() +
            sizeof(this->id);

        // TODO: add next_dest and previous_src?

        switch (this->op)
        {
            case Operation::SEND_GLOBAL_MODEL:
                // Size of the model + size of the param
                result += Constants::MODEL_SIZE_BYTES + sizeof(uint8_t);
                break;
            case Operation::SEND_LOCAL_MODEL:
                // Size of the model but no params
                result += Constants::MODEL_SIZE_BYTES;
                break;
            case Operation::REGISTRATION_REQUEST:
                result += sizeof(NodeInfo);
                break;
            case Operation::REGISTRATION_CONFIRMATION:
                // Total size of the list
                result += sizeof(NodeInfo) * this->get_node_list()->size();
                break;

            case Operation::KILL_TRAINER:
                // No additionnal arguments
                break;
        }
    }

    return this->packet_size;
}

/**
 * Format an the operation field as text with nice colors
 *
 * @return formated text;
 */
std::string Packet::operation_to_string(Packet::Operation op)
{
    switch (op)
    {
        case Operation::SEND_GLOBAL_MODEL:
            return "\x1B[34mSEND_GLOBAL_MODEL\033[0m";
        case Operation::SEND_LOCAL_MODEL:
            return "\x1B[32mSEND_LOCAL_MODEL\033[0m";
        case Operation::KILL_TRAINER:
            return "\x1B[31mKILL_TRAINER\033[0m";
        case Operation::REGISTRATION_REQUEST:
            return "\x1B[33mREGISTRATION_REQUEST\033[0m";
        case Operation::REGISTRATION_CONFIRMATION:
            return "\x1B[33mREGISTRATION_CONFIRMATION\033[0m";
    }
}

/**
 * Clone a packet
 */
Packet *Packet::clone()
{
    Packet *res = new Packet(this->original_src, this->final_dst, this->op, this->data);

    // Decrement the total packet number because a clone isn't considered as a new packet
    Packet::total_packet_number -= 1;
    
    return res;
}
