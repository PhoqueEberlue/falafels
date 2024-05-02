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

    auto received_packets = make_shared<queue<unique_ptr<Packet>>>();
    auto to_be_sent_packets = make_shared<queue<shared_ptr<Packet>>>();

    this->role->set_queues(received_packets, to_be_sent_packets);
    this->network_manager->set_queues(received_packets, to_be_sent_packets);
}

Node::~Node()
{
}


void Node::run()
{
    while(true)
    {
        // break when the role has been killed
        if(!this->role->run())
            break;

        this->network_manager->run();
    }
}

NodeInfo Node::get_node_info()
{ 
    return this->network_manager->get_my_node_info();
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
