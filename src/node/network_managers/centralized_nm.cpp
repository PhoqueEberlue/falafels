#include "centralized_nm.hpp"
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>


CentralizedNetworkManager::CentralizedNetworkManager(node_name name) {
    this->my_node_name = name;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

CentralizedNetworkManager::~CentralizedNetworkManager() {};

std::vector<node_name> CentralizedNetworkManager::get_node_names_filter(std::function<bool(NodeInfo*)>filter)
{
    std::vector<node_name> result = {};

    for(auto node_info : *this->get_bootstrap_nodes()) {
        if (filter(node_info)) {
            result.push_back(node_info->name);
        }
    }

    return result;
}

void CentralizedNetworkManager::put(Packet *packet, node_name name, uint64_t simulated_size_in_bytes) {
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    receiver_mailbox->put(packet, simulated_size_in_bytes);
}

Packet *CentralizedNetworkManager::get() {
    return this->mailbox->get<Packet>();
}
