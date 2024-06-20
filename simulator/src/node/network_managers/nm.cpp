#include "nm.hpp"
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <format>
#include <memory>
#include <optional>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <simgrid/s4u/Activity.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <vector>
#include <xbt/log.h>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <simgrid/s4u/ActivitySet.hpp>

#include "../../utils/utils.hpp"
#include "../../dot.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_network_manager, "Messages specific for this example");

NetworkManager::NetworkManager(NodeInfo node_info) : my_node_info(node_info)
{
    this->pending_async_put = new simgrid::s4u::ActivitySet();
    this->pending_comm_and_mess_get = new simgrid::s4u::ActivitySet(); 

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(this->my_node_info.name);

    this->registration_requests = new vector<Packet::RegistrationRequest>();
}

NetworkManager::~NetworkManager()
{
    delete this->bootstrap_nodes;
    delete this->pending_async_put;
    delete this->pending_comm_and_mess_get;
    delete this->registration_requests;
}

void NetworkManager::set_bootstrap_nodes(vector<NodeInfo> *nodes)
{
    this->bootstrap_nodes = nodes;
}

void NetworkManager::run()
{ 
    switch (this->state)
    {
        case INITIALIZING:
            switch (this->my_node_info.role)
            {
                case NodeRole::Trainer:
                    this->send_registration_request();
                    this->state = WAITING_REGISTRATION_CONFIRMATION;
                    break;
                case NodeRole::Aggregator:
                    this->state = WAITING_REGISTRATION_REQUEST;
                    break;
            }
            break;
        case WAITING_REGISTRATION_CONFIRMATION:
            {
                auto packet = this->get();

                if (auto *confirmation = get_if<Packet::RegistrationConfirmation>(&packet->op))
                {
                    this->handle_registration_confirmation(*confirmation);
                    this->state = RUNNING;

                    // Initialize the first waiting activities: this should be done one time before going into RUNNING state
                    // Add Comm aysnc get
                    this->pending_comm_and_mess_get->push(this->get_async());

                    // Add Mess async get
                    this->pending_comm_and_mess_get->push(this->mp->get_async_to_be_sent_packet());
                }
                break;
            }
        case WAITING_REGISTRATION_REQUEST:
            {
                if (!this->start_time.has_value())
                    this->start_time = simgrid::s4u::Engine::get_instance()->get_clock();

                const auto time_elapsed = simgrid::s4u::Engine::get_instance()->get_clock() - *this->start_time;
                const auto remaining_time = Constants::REGISTRATION_TIMEOUT - time_elapsed;
                bool ending_state = false;

                // To be honest I'm not even sure this is reachable
                if (remaining_time < 0)
                {
                    XBT_INFO("REACHED THE POINT WHERE REMAINING TIME WENT NEGATIVE");
                    ending_state = true;
                }

                unique_ptr<Packet> packet;

                try { packet = this->get(remaining_time); }
                catch (simgrid::TimeoutException) 
                {
                    ending_state = true;    
                }

                if (ending_state)
                {
                    this->handle_registration_requests();
                    this->state = RUNNING;

                    // Initialize the first waiting activities: this should be done one time before going into RUNNING state
                    // Add Comm aysnc get
                    this->pending_comm_and_mess_get->push(this->get_async());

                    // Add Mess async get
                    this->pending_comm_and_mess_get->push(this->mp->get_async_to_be_sent_packet());
                }
                else
                {
                    if (auto *request = get_if<Packet::RegistrationRequest>(&packet->op))
                    {
                        this->registration_requests->push_back(*request);
                    }
                }

                break;
            }
        case RUNNING:
            {
                auto activity = this->pending_comm_and_mess_get->wait_any();

                // If the activity has type Comm, it means we received a packet from the network
                if (auto comm = boost::dynamic_pointer_cast<simgrid::s4u::Comm>(activity))
                {
                    // Add Comm aysnc get for next run, because the previous one is deleted by wait_any()
                    this->pending_comm_and_mess_get->push(this->get_async());

                    auto p = (Packet *) comm->get_payload();

                    // Can we handle this log better? is it possible to print it with a callback maybe?
                    XBT_INFO("%s <--%s(%lu)--- %s", p->dst.c_str(), p->get_op_name(), p->id, p->src.c_str());

                    // Case where we receive Kill
                    if (auto *kill = get_if<Packet::KillTrainer>(&p->op))
                    {
                        this->state = KILLING;

                        // Cleanly detach each async put
                        for (int i = 0; i < this->pending_async_put->size(); i++)
                            this->pending_async_put->at(i).detach();

                        // Clear all async put before sending and waiting the kill packet
                        this->pending_async_put->clear();

                        //---------------------------------------------
                        // TODO: make this better if possible......
                        // In case we are the hierarchical_nm
                        if (this->my_node_info.name.contains("hierarchical"))
                        {
                            // Send the packet to the cluster_nm
                            std::string cluster_nm_name = this->my_node_info.name;
                            replace_first(cluster_nm_name, "hierarchical_", "");
                            p->dst = cluster_nm_name;
                            p->final_dst = cluster_nm_name;
                            this->send_async(p, true);
                        }

                        // Case where we receive kill packet from hierarchical_nm
                        if (this->my_node_info.role == NodeRole::Aggregator)
                        {
                            this->send_packet(p);
                        }
                        //---------------------------------------------
                    }


                    // Route the packet in any ways. If its a kill packet in some p2p scenario it will be redirected.
                    this->route_packet(p);
                }
                // If the activity has type Mess, it means we received a to be sent packet from the Role via MessageQueue
                else if (auto mess = boost::dynamic_pointer_cast<simgrid::s4u::Mess>(activity))
                {
                    // Add Mess aysnc get for next run, because the previous one is deleted by wait_any()
                    this->pending_comm_and_mess_get->push(this->mp->get_async_to_be_sent_packet());

                    auto p = (Packet *) mess->get_payload();

                    // Case where we send kill to someone else
                    if (auto *kill = get_if<Packet::KillTrainer>(&p->op))
                    {
                        this->state = KILLING;

                        // TODO: Duplicate with the if above, might refactor
                        // Cleanly detach each async put
                        for (int i = 0; i < this->pending_async_put->size(); i++)
                            this->pending_async_put->at(i).detach();

                        // Clear all async put before sending and waiting the kill packet
                        this->pending_async_put->clear();

                    }

                    this->send_packet(p);
                }
                break;
            }
        case KILLING:
            {
                std::string my_node_name = this->my_node_info.name;

                // TODO: make this better if possible......
                if (this->my_node_info.name.contains("hierarchical"))
                {
                    replace_first(my_node_name, "hierarchical_", "");
                }

                // Get the actors running on the current host
                auto actors = simgrid::s4u::Engine::get_instance()->host_by_name(my_node_name)->get_all_actors();

                for (auto actor : actors)
                {
                    // Delete the actor representing the Role process
                    if (actor->get_name().compare(std::format("{}_role", my_node_name)) == 0)
                    {
                        XBT_INFO("Killing actor: %s", actor->get_name().c_str());
                        actor->kill();
                    }
                }

                // Wait to send the kill message with a timeout of 2 in case the two last nodes are trying to send 
                // the kill packet to each other and would cause a blocking situation.
                // So if a node isn't reachable, we consider it is already dead.
                try {
                    this->pending_async_put->wait_all_for(2);
                } catch (simgrid::TimeoutException) {
                    XBT_INFO("Can't redirect KILL, node unreachable");
                } catch (simgrid::NetworkFailureException) {
                    XBT_INFO("Can't redirect KILL, NetworkFailure");
                }

                // Exit self actor, representing the NetworkManager process
                simgrid::s4u::this_actor::exit();
            }
    }
}



unique_ptr<Packet> NetworkManager::get(const optional<double> timeout)
{
    unique_ptr<Packet> p;

    if (timeout.has_value())
        p = this->mailbox->get_unique<Packet>(*timeout);
    else
        p = this->mailbox->get_unique<Packet>();

    XBT_INFO("%s <--%s(%lu)--- %s", p->dst.c_str(), p->get_op_name(), p->id, p->src.c_str());
    return p;
}

simgrid::s4u::CommPtr NetworkManager::get_async()
{
    return this->mailbox->get_async();
}

void NetworkManager::send_packet(Packet *p)
{
    if (p->broadcast)
    {
        this->broadcast(p);
    }
    else
    {
        this->send_async(p);
    }

    // Send and broadcast should clone the pointer internally, so we can delete the packet here.
    // delete p;
}

void NetworkManager::send_async(Packet *p, bool is_redirected)
{
    auto p_clone = p->clone();
    p_clone->src = this->get_my_node_name();
    p_clone->dst = p->dst;

    // Only write original source when sending packets created by the current node.
    if (!is_redirected)
    {
        p_clone->original_src = this->get_my_node_name();
        XBT_INFO("%s ---%s(%lu)--> %s", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
    }
    else
    {
        p_clone->original_src = p->original_src;
        XBT_INFO("%s ---%s(%lu)--> %s [REDIRECT]", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
    }

    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(p_clone->dst);

    DOTGenerator::get_instance().add_to_state(
        simgrid::s4u::Engine::get_instance()->get_clock(), 
        std::format("{} -> {} [label=\"{}\", style=dotted];", p_clone->src, p_clone->dst, p_clone->get_op_name())
    );


    auto comm = receiver_mailbox->put_async(p_clone, p_clone->get_packet_size());
    
    this->pending_async_put->push(comm);
}
