#include "node.hpp"

NodeInfo *Node::get_node_info()
{ 
    return new NodeInfo { .name=this->role->get_network_manager()->get_my_node_name(), .role=this->role->get_role_type() }; 
}
