#include "node.hpp"
#include "mediator/mediator_producer.hpp"
#include "network_managers/nm.hpp"
#include <format>
#include <memory>
#include <simgrid/s4u/Engine.hpp>
#include <utility>
#include <vector>
#include <xbt/log.h>


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_node, "Messages specific for this example");

using namespace std;
using namespace protocol;

Node::Node(Role *r, NetworkManager *nm)
{
    this->role = r;
    this->network_manager = nm;

    // TODO: can simplify that and directly put it into the Role and NetworkManager constructors.
    // Though we might also unify the data and the way ther are passed to them, i.e. the NodeInfo.
    auto mc = make_unique<MediatorConsumer>(this->get_node_info().name);
    auto mp = make_unique<MediatorProducer>(this->get_node_info().name);

    this->role->set_mediator_consumer(std::move(mc));
    this->network_manager->set_mediator_producer(std::move(mp));
}

void Node::run()
{
    node_name name = this->get_node_info().name;
    auto e = simgrid::s4u::Engine::get_instance();

    simgrid::s4u::Actor::create(
        std::format("{}_role", name), e->host_by_name(name), &Node::run_role, this->role
    );

    simgrid::s4u::Actor::create(
        std::format("{}_nm", name), e->host_by_name(name), &Node::run_network_manager, this->network_manager
    );
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
