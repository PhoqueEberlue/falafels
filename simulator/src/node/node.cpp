#include "node.hpp"
#include "mediator/mediator_producer.hpp"
#include "network_managers/nm.hpp"
#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <utility>
#include <vector>
#include <xbt/log.h>
#include <simgrid/s4u/Comm.hpp>
#include <simgrid/s4u/Exec.hpp>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_node, "Messages specific for this example");

Node::Node(unique_ptr<Role> r, unique_ptr<NetworkManager> nm)
{
    this->role = std::move(r);
    this->network_manager = std::move(nm);

    // Creates queues to enable communication between Role and NM.
    auto received_packets = make_shared<queue<unique_ptr<Packet>>>();
    auto to_be_sent_packets = make_shared<queue<shared_ptr<Packet>>>();
    auto nm_events = make_shared<queue<unique_ptr<Mediator::Event>>>();
    auto node_activities = new simgrid::s4u::ActivitySet();

    this->node_activities = node_activities;

    auto mc = make_unique<MediatorConsumer>(received_packets, to_be_sent_packets, nm_events, node_activities);
    auto mp = make_unique<MediatorProducer>(received_packets, to_be_sent_packets, nm_events, node_activities);

    this->role->set_mediator_consumer(std::move(mc));
    this->network_manager->set_mediator_producer(std::move(mp));
}

void Node::run()
{
    // Run Role and NetworkManager one time to populate node_activities and setup the automatons.
    this->network_manager->run();
    this->role->run();

    while(true)
    {
        if (this->node_activities->size() > 0)
        {
            auto completed_activity = this->node_activities->wait_any();

            // A Communication activity comes from a NetworkManager
            if (boost::dynamic_pointer_cast<simgrid::s4u::Comm>(completed_activity))
            {
                // So we can call the NetworkManager to wake up and work with the finished activity
                if (!this->network_manager->run())
                {
                    // Breaks when NetworkManager run() returns false
                    break;
                }

                // Then run the Role because it may have received a packet from the NetworkManager
                this->role->run();
            }
            // A Execution activity comes from a Role
            else if (boost::dynamic_pointer_cast<simgrid::s4u::Exec>(completed_activity))
            {
                // Exactly the opposite as the previous if
                this->role->run();

                if (!this->network_manager->run())
                {
                    // Breaks when NetworkManager run() returns false
                    break;
                }
            }
        }
        else
        {
            this->role->run();

            if (!this->network_manager->run())
            {
                // Breaks when NetworkManager run() returns false
                break;
            }
        }
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
