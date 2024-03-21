#include "decentralized_nm.hpp"
#include <vector>

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

Packet *DecentralizedNetworkManager::get() {
    return this->mailbox->get<Packet>();
}
