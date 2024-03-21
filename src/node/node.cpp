#include "node.hpp"

Node::Node(Role *r, NetworkManager *nm)
{
    this->role = r;
    this->role->set_network_manager(nm);
}

Node::~Node()
{
    delete this->role->get_network_manager();
    delete this->role;
}

NodeInfo *Node::get_node_info()
{ 
    // Instanciate and return NodeInfo struct
    return new NodeInfo { .name=this->role->get_network_manager()->get_my_node_name(), .role=this->role->get_role_type() }; 
}

void Node::set_role(Role *r) { 
    // if we are changing to a new role
    if (!this->role)
    {
        // Reassign the network manager to the new role 
        r->set_network_manager(this->role->get_network_manager());

        // And delete previous role
        delete this->role;
    }

    this->role = r; 
}
