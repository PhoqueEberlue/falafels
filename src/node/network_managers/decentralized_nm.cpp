#include "decentralized_nm.hpp"
#include <vector>

DecentralizedNetworkManager::DecentralizedNetworkManager(std::vector<NodeInfo*> *nodes, node_name name) {
    this->set_boostrap_nodes(nodes);
    this->my_node_name = name;
    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(name);
}

DecentralizedNetworkManager::~DecentralizedNetworkManager() {};

std::vector<node_name> DecentralizedNetworkManager::get_node_names_filter(std::function<NodeInfo*(bool)>) {}

void DecentralizedNetworkManager::send(Packet *packet, node_name name) {
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(name);

    receiver_mailbox->put(packet, sizeof(*packet));
}

Packet *DecentralizedNetworkManager::get() {
    return this->mailbox->get<Packet>();
}
