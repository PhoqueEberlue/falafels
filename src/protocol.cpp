#include "protocol.hpp"
#include <string>
#include <xbt/asserts.h>
#include <xbt/log.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_falafels_protocol, "Messages specific for this example");

/**
 * Compute the simulated size of a packet:
 * - The "real" memory used in the structure
 * - The simulated memory, such as MODEL_SIZE_BYTES
 *
 * @return The simulated size in bytes.
 */
uint64_t Packet::get_packet_size()
{
    if (this->packet_size == 0) 
    {
        uint64_t result = sizeof(Packet::Operation) + this->src.size();

        // Add the model size if its a send model operation
        if (this->op == Packet::SEND_GLOBAL_MODEL || this->op == Packet::SEND_LOCAL_MODEL)
            result += Constants::MODEL_SIZE_BYTES;

        if (this->args)
        {
            for (const auto&[key, value]: *this->args)
            {
                result += key.size() + value.size();
            }
        }
    }

    return this->packet_size;
}

// /!\ WARNING DEPRECATED /!\
// For now we copy every packet and sharing common packets pointers with several nodes is not possible
void Packet::incr_ref_count()
{ 
    this->ref_count += 1; 
} 

// /!\ WARNING DEPRECATED /!\
// For now we copy every packet and sharing common packets pointers with several nodes is not possible
void Packet::decr_ref_count()
{
    this->ref_count -= 1;

    // XBT_INFO("ref count %i", this->ref_count);

    if (this->ref_count == 0)
    {
        // XBT_INFO("Deleting packet %s", this->op_string.c_str());
        delete this;
    }
}

/**
 * Format an the operation field as text with nice colors
 *
 * @return formated text;
 */
std::string Packet::operation_to_string(Packet::Operation op)
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

/**
 * WARNING WE DO NOT COPY THE POINTED VALUES YET
 */
Packet *Packet::clone()
{
    Packet *res;

    if (this->args)
    {
        auto args_copy = new std::unordered_map<std::string, std::string>(*this->args);
        res = new Packet(this->op, this->original_src, this->final_dst, args_copy);
    }
    else 
    {
        res = new Packet(this->op, this->original_src, this->final_dst);
    }

    // Decrement the total packet number because a clone isn't considered as a new packet
    Packet::total_packet_number -= 1;

    // Copy the packet id to the new one
    res->id = this->id;
    res->src = this->src;
    res->dst = this->dst;

    return res;
}

Packet::~Packet()
{
    // Delete potential arguments
    if (this->args) delete this->args;
}
