#include "centralized_nm.hpp"
#include <simgrid/s4u/Mailbox.hpp>
#include <vector>


CentralizedNetworkManager::CentralizedNetworkManager(std::vector<NodeInfo*> *nodes, node_name name) {
    this->set_boostrap_nodes(nodes);
    this->my_node_name = name;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

CentralizedNetworkManager::~CentralizedNetworkManager() {};

std::vector<node_name> CentralizedNetworkManager::get_node_names_filter(std::function<bool(NodeInfo*)>filter)
{
    std::vector<node_name> result = {};

    for(auto node_info : *this->get_boostrap_nodes()) {
        if (filter(node_info)) {
            result.push_back(node_info->name);
        }
    }

    return result;
}

void CentralizedNetworkManager::send(Packet *packet, node_name name) {
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    receiver_mailbox->put(packet, sizeof(*packet));
}

Packet *CentralizedNetworkManager::get() {
    return this->mailbox->get<Packet>();
}
