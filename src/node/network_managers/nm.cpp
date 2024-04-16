#include "nm.hpp"
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <xbt/log.h>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_network_manager, "Messages specific for this example");

void NetworkManager::send(Packet *packet, node_name dst)
{
    XBT_INFO("%s ---%s--> %s", this->my_node_name.c_str(), packet->op_string.c_str(), dst.c_str());
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(dst);
    // packet->incr_ref_count();


    auto new_p = packet->clone();
    new_p->src = this->my_node_name;
    new_p->dst = dst;

    receiver_mailbox->put(new_p, new_p->get_packet_size());
}

bool NetworkManager::send(Packet *packet, node_name dst, uint64_t timeout)
{
    XBT_INFO("%s ---%s--> %s (timeout: %lu)", this->my_node_name.c_str(), packet->op_string.c_str(), dst.c_str(), timeout);

    Packet *new_p;

    try 
    {
        auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(dst);
        // packet->incr_ref_count();
        
        new_p = packet->clone();
        new_p->src = this->my_node_name;
        new_p->dst = dst;

        receiver_mailbox->put(new_p, new_p->get_packet_size(), timeout);
        return true;
    } 
    catch (simgrid::TimeoutException) 
    {
        delete new_p;
        // packet->decr_ref_count();
        XBT_INFO("Timeout, couldn't send message from %s to %s", this->get_my_node_name().c_str(), dst.c_str());
        return false;
    }
}

simgrid::s4u::CommPtr NetworkManager::send_async(Packet *packet, node_name dst)
{
    XBT_INFO("%s ---%s--> %s", this->my_node_name.c_str(), packet->op_string.c_str(), dst.c_str());

    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(dst);
    // packet->incr_ref_count();

    auto comm =  receiver_mailbox->put_async(packet, packet->get_packet_size());
    return comm;
}

