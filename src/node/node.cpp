#include "node.hpp"
#include "network_managers/nm.hpp"
#include <memory>
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_node, "Messages specific for this example");

Node::Node(unique_ptr<Role> r, unique_ptr<NetworkManager> nm)
{
    this->role = std::move(r);
    this->network_manager = std::move(nm);

    // Creates queues to enable communication between Role and NM.
    auto received_packets = make_shared<queue<unique_ptr<Packet>>>();
    auto to_be_sent_packets = make_shared<queue<shared_ptr<Packet>>>();
    auto nm_events = make_shared<queue<unique_ptr<NetworkManager::Event>>>();

    this->role->set_queues(received_packets, to_be_sent_packets, nm_events);
    this->network_manager->set_queues(received_packets, to_be_sent_packets, nm_events);
}

void Node::run()
{
    while(true)
    {
        this->role->run();

        // Breaks when NetworkManager run() returns false
        if (!this->network_manager->run())
            break;
    }
}

NodeInfo Node::get_node_info()
{ 
    return this->network_manager->get_my_node_info();
}

void Node::set_bootstrap_nodes(std::vector<NodeInfo> *nodes)
{
    this->network_manager->set_bootstrap_nodes(nodes);
}

// TODO: fix later
// void Node::set_role(Role *r)
// { 
//     // if we are changing to a new role
//     if (!this->role)
//     {
//         // Reassign the network manager to the new role 
//         r->set_network_manager(this->role->get_network_manager());
// 
//         // And delete previous role
//         delete this->role;
//     }
// 
//     this->role = r; 
// }
