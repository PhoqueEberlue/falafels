#include "nm.hpp"
#include <simgrid/Exception.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_network_manager, "Messages specific for this example");

void NetworkManager::send(Packet *packet, node_name name)
{
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);
    packet->incr_ref_count();
    receiver_mailbox->put(packet, packet->get_packet_size());
}

bool NetworkManager::send_timeout(Packet *packet, node_name name, uint64_t timeout)
{
    try 
    {
        auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);
        packet->incr_ref_count();
        receiver_mailbox->put(packet, packet->get_packet_size(), timeout);
        return true;
    } 
    catch (simgrid::TimeoutException) 
    {
        packet->decr_ref_count();
        XBT_INFO("Timeout, couldn't send message from %s to %s", this->get_my_node_name().c_str(), name.c_str());
        return false;
    }
}

Packet *NetworkManager::get()
{
    return this->mailbox->get<Packet>();
}
