#include <cstdint>
#include <format>
#include <memory>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <xbt/log.h>
#include <simgrid/s4u/Engine.hpp>

#include "uni_ring_nm.hpp"
#include "../../dot.hpp"
#include "nm.hpp"


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_uni_ring_nm, "Messages specific for this example");

using namespace std;
using namespace protocol;

UniRingNetworkManager::UniRingNetworkManager(NodeInfo node_info) : NetworkManager(node_info) {}

UniRingNetworkManager::~UniRingNetworkManager() {}

// static filters::NodeFilter broadcast_op_table_uni_ring(const operations::Operation &op)
// {
//     return std::visit(overloaded {
//         [](operations::SendGlobalModel) -> filters::NodeFilter 
//         {
//             return filters::everyone;
//         },
//         [](operations::Kill) -> filters::NodeFilter
//         {
//             return filters::trainers_and_aggregators;
//         },
//         [](operations::SendLocalModel) -> filters::NodeFilter 
//         {
//             return filters::everyone;
//         },
//         [](operations::RegistrationConfirmation) -> filters::NodeFilter
//         {
//             xbt_die("RegistrationConfirmation shoudn't be coming from the Role as a to be sent packet");
//         },
//         [](operations::RegistrationRequest) -> filters::NodeFilter 
//         {
//             xbt_die("RegistrationRequest shoudn't be coming from the Role as a to be sent packet");
//         }
//     }, op);
// }

void UniRingNetworkManager::run()
{
    switch (this->state)
    {
        case INITIALIZING:
            switch (this->my_node_info.role)
            {
                // Both secondary Aggregators and Trainers send a request to the MainAggregator
                case NodeRole::Aggregator:
                case NodeRole::Trainer:
                    this->send_registration_request();
                    this->state = WAITING_REGISTRATION_CONFIRMATION;
                    break;
                case NodeRole::MainAggregator:
                    this->state = WAITING_REGISTRATION_REQUEST;
                    break;
            }
            break;
        case WAITING_REGISTRATION_CONFIRMATION:
            {
                auto packet = this->get();

                if (auto *confirmation = get_if<operations::RegistrationConfirmation>(&packet->op))
                {
                    this->handle_registration_confirmation(*confirmation);
                    this->state = RUNNING;

                    this->init_run_activities();
                }
                break;
            }
        case WAITING_REGISTRATION_REQUEST:
            {
                if (!this->start_time.has_value())
                    this->start_time = simgrid::s4u::Engine::get_instance()->get_clock();

                const auto time_elapsed = simgrid::s4u::Engine::get_instance()->get_clock() - *this->start_time;
                const auto remaining_time = Constants::REGISTRATION_TIMEOUT - time_elapsed;

                try 
                { 
                    // try to wait for incoming RegistrationRequests
                    auto packet = this->get(remaining_time); 

                    if (auto *request = get_if<operations::RegistrationRequest>(&packet->op))
                    {
                        this->registration_requests->push_back(*request);
                    }
                }
                catch (simgrid::TimeoutException) 
                {
                    // when timeout, handle registrations and change state
                    this->handle_registration_requests();
                    this->state = RUNNING;

                    this->init_run_activities();
                }
                break;
            }
        case RUNNING:
            {
                auto activity = this->pending_comm_and_mess_get->wait_any();

                // If the activity has type Comm, it means we received a packet from the network
                if (auto comm = boost::dynamic_pointer_cast<simgrid::s4u::Comm>(activity))
                {
                    // Reload Comm aysnc get for next run, because the previous one is deleted by wait_any()
                    this->pending_comm_and_mess_get->push(this->get_async());

                    auto p = std::unique_ptr<Packet>((Packet *) comm->get_payload());

                    // Can we handle this log better? is it possible to print it with a callback maybe?
                    XBT_INFO("%s <--%s(%lu)--- %s", p->dst.c_str(), p->get_op_name(), p->id, p->src.c_str());

                    // Case where we receive Kill, no matter our role
                    if (auto *kill = get_if<operations::Kill>(&p->op))
                    {
                        this->state = KILLING;
                        this->clear_async_puts();                        

                        // We do not redirect if our neighbour is a MainAggregator because it kills itself
                        if (this->left_node.role != NodeRole::MainAggregator)
                            this->send_to_neighbour(p, true);
                    }
                    // Case where a Trainer receives a packet
                    else if (this->my_node_info.role == NodeRole::Trainer)
                    {
                        // Increment the number of hops to track the number of Trainers in the ring
                        p->increment_hops();
                        // Always redirect packets as a Trainer
                        this->send_to_neighbour(p, true);
                    }
                    // Case where MainAggregator or Aggregator receives a packet
                    else 
                    {
                        p->seal_hops = true; // Seal the counter whenever the first Aggregator is met
                        
                        if (auto *global_model = get_if<operations::SendGlobalModel>(&p->op))
                        {
                            // Check if this global_model was originally sent by this Aggregator
                            if (p->original_src == this->get_my_node_name())
                            {
                                if (!this->cluster_connected_have_been_sent)
                                {
                                    // We then know how much trainers were in the ring thanks to nb_hops
                                    XBT_INFO("Sending ClusterConnected");
                                    this->mp->put_nm_event(
                                        new Mediator::Event {
                                            Mediator::ClusterConnected { .number_client_connected=(uint16_t)p->get_nb_hops() }
                                        }
                                    );

                                    // Prevent the event from being sent again
                                    this->cluster_connected_have_been_sent = true;
                                }
                            }
                            // If it wasn't sent by us we route it so it continues to the orginal sender
                            else
                            {
                                // Aggregator only redirects SendGlobalModel that wasn't sent by itself
                                this->send_to_neighbour(p, true);
                            }
                        }
                    }

                    // if the packet is targeted to our Role, put the packet's operation
                    this->if_target_put_op(std::move(p));
                }
                // If the activity has type Mess, it means we received a to be sent packet from the Role via MessageQueue
                else if (auto mess = boost::dynamic_pointer_cast<simgrid::s4u::Mess>(activity))
                {
                    // Reload Mess aysnc get for next run, because the previous one is deleted by wait_any()
                    this->pending_comm_and_mess_get->push(this->mp->get_async_to_be_sent_packet());

                    auto p = std::unique_ptr<Packet>((Packet *) mess->get_payload());

                    // Case where we send kill to someone else
                    if (auto *kill = get_if<operations::Kill>(&p->op))
                    {
                        this->state = KILLING;
                        // Clear all other async put in order to be able to wait only the kill send
                        this->clear_async_puts();                        
                    }

                    this->send_to_neighbour(p);
                }
                break;
            }
        case KILLING:
            {
                this->kill_role_actor();
                this->handle_kill_phase();
                simgrid::s4u::this_actor::exit();
            }
    }
}

void UniRingNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::MainAggregator, "Only MainAggregator is allowed to handle registration requests");

    auto aggregator_list = queue<operations::RegistrationRequest>();
    auto trainer_list = queue<operations::RegistrationRequest>();

    for (auto req : *this->registration_requests)
    {
        switch (req.node_to_register.role)
        {
            case NodeRole::Trainer:
                trainer_list.push(req);
                break;
            case NodeRole::Aggregator:
                aggregator_list.push(req);
                break;
            case NodeRole::MainAggregator:
                xbt_assert(false, "MainAggregator cannot subscribe to another MainAggregator because there should be only one.");
                break;
        }
    }

    const uint32_t nb_trainers = trainer_list.size();
    // We, the MainAggregator already count for one
    const uint32_t nb_aggregators = 1 + aggregator_list.size();

    const uint32_t nb_trainers_between_each_aggregator = (uint32_t)nb_trainers / nb_aggregators;
    uint32_t remainder = nb_trainers % nb_aggregators;

    // ---------- Creating connections ----------
    int nb_trainer_emplaced = 0;

    auto final_list = vector<operations::RegistrationRequest>();

    for (int i = 0; i < this->registration_requests->size(); i++)
    {
        // We start by placing remainder trainers when nb_trainers isn't divisible by nb_aggregators
        if (remainder > 0)
        {
            remainder--;
            final_list.push_back(trainer_list.front());
            trainer_list.pop();
        }
        // Then place exactly nb_trainers_between_each_aggregator
        else if (nb_trainer_emplaced < nb_trainers_between_each_aggregator)
        {
            nb_trainer_emplaced++;
            final_list.push_back(trainer_list.front());
            trainer_list.pop();
        }
        // Then place one aggregator and go back to previous if
        else
        {
            nb_trainer_emplaced = 0;
            final_list.push_back(aggregator_list.front());
            aggregator_list.pop();
        }
        
    }

    // ----------- Sending confirmations ----------
    // The first node will be our neigbour
    this->left_node = final_list.at(0).node_to_register;
    
    for (int i = 0; i < final_list.size(); i++)
    {
        if (final_list.at(i).node_to_register.role == NodeRole::Trainer)
            XBT_INFO("T");
        else
            XBT_INFO("A");

        NodeInfo neigbour_info;

        // last node is conneted to the MainAggregator (this)
        if (i == final_list.size() - 1)
            neigbour_info = this->my_node_info;
        else
            neigbour_info = final_list.at(i + 1).node_to_register;

        auto neigbours = vector<NodeInfo>();
        neigbours.push_back(neigbour_info);

        auto res_p = make_unique<Packet>(Packet(
            final_list.at(i).node_to_register.name, // Send the packet to the current node
            final_list.at(i).node_to_register.name, // Send the packet to the current node
            operations::RegistrationConfirmation(
                make_shared<vector<NodeInfo>>(neigbours)
            )
        ));

        // Sending the packet
        this->send_async(res_p);
    }
    // --------------------------------------------- 
}

void UniRingNetworkManager::send_registration_request()
{
    xbt_assert(this->my_node_info.role == NodeRole::Trainer || this->my_node_info.role == NodeRole::Aggregator, 
               "MainAggregator isn't supposed to be calling send_registration_request()");

    // Take the first bootstrap node. TODO: handle multiple bootstrap nodes??
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = make_unique<Packet>(Packet(
        bootstrap_node.name, bootstrap_node.name, 
        operations::RegistrationRequest(
            this->my_node_info
        )
    ));

    // Send the request
    this->send_async(p);
}

void UniRingNetworkManager::handle_registration_confirmation(const operations::RegistrationConfirmation &confirmation)
{
    XBT_INFO("Succesfully registered");

    auto bootstrap_node = this->bootstrap_nodes->at(0);

    this->left_node = confirmation.node_list->at(0);

    this->mp->put_nm_event(
        new Mediator::Event { Mediator::NodeConnected {} }
    );
}

void UniRingNetworkManager::send_to_neighbour(const unique_ptr<Packet> &p, bool is_redirected) 
{
    p->dst = this->left_node.name;
    this->send_async(p, is_redirected);
}


void UniRingNetworkManager::handle_kill_phase()
{
    // Only wait if the last node isn't a MainAggregator, because this last will already be killed anyways
    if (this->pending_async_put->size() > 0)
        this->pending_async_put->wait_all();
}
