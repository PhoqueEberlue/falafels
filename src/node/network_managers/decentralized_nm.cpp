#include <simgrid/Exception.hpp>
#include <vector>
#include "decentralized_nm.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_decentralized_nm, "Messages specific for this example");

DecentralizedNetworkManager::DecentralizedNetworkManager(node_name name) {
    this->my_node_name = name;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

DecentralizedNetworkManager::~DecentralizedNetworkManager() {};

std::vector<node_name> DecentralizedNetworkManager::get_node_names_filter(std::function<NodeInfo*(bool)>)
{
    return std::vector<node_name>();
}

void DecentralizedNetworkManager::put(Packet *packet, node_name name, uint64_t simulated_size_in_bytes) {
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    receiver_mailbox->put(packet, simulated_size_in_bytes);
}

bool DecentralizedNetworkManager::put_timeout(Packet *packet, node_name name, uint64_t simulated_size_in_bytes, uint64_t timeout)
{
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    try 
    {
        receiver_mailbox->put(packet, simulated_size_in_bytes, timeout);
        return true;
    } 
    catch (simgrid::TimeoutException) 
    {
        XBT_INFO("Timeout, couldn't send message from %s to %s", this->get_my_node_name().c_str(), name.c_str());
        return false;
    }
}

Packet *DecentralizedNetworkManager::get() {
    return this->mailbox->get<Packet>();
}
