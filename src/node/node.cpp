#include "node.hpp"
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_node, "Messages specific for this example");

Node::Node(unique_ptr<Role> r, unique_ptr<NetworkManager> nm)
{
    this->role = std::move(r);
    this->role->set_network_manager(std::move(nm));
}

Node::~Node()
{
}

NodeInfo Node::get_node_info()
{ 
    // Instanciate and return NodeInfo struct
    return this->role->get_network_manager()->get_my_node_info();
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
