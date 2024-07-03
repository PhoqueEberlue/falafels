#include <cstdint>
#include <format>
#include <memory>
#include <optional>
#include <simgrid/Exception.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <variant>
#include <vector>
#include <xbt/asserts.h>
#include <xbt/log.h>

#include "hierarchical_nm.hpp"
#include "../../dot.hpp"
#include "nm.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_hierarchical_nm, "Messages specific for this example");

using namespace std;
using namespace protocol;

HierarchicalNetworkManager::HierarchicalNetworkManager(NodeInfo node_info) : NetworkManager(node_info)
{
    this->connected_nodes = new vector<NodeInfo>();
}

HierarchicalNetworkManager::~HierarchicalNetworkManager() 
{
    delete this->connected_nodes;
};

void HierarchicalNetworkManager::run()
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
                case NodeRole::MainAggregator:
                    this->state = WAITING_REGISTRATION_REQUEST;
                    break;
                case NodeRole::Aggregator:
                    xbt_die("The HierarchicalNetworkManager can't have a secondary Aggregator, only a Main one.");
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

                    // Only the trainer (The hierarchical aggregators under a fake identity) can receive a Kill
                    if (auto *kill = get_if<operations::Kill>(&p->op))
                    {
                        this->state = KILLING;
                        this->clear_async_puts();

                        // Redirect to the main node upon receiving kill
                        std::string hierarchical_aggregator_name = this->my_node_info.name;
                        replace_first(hierarchical_aggregator_name, "hierarchical_", "");
                        p->dst = hierarchical_aggregator_name;
                        this->send_async(p, true);
                        break;
                    }

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
                        this->clear_async_puts();
                    }

                    // Send to Central Aggregator if we're hierarchical node
                    // Or send to hierarchical nodes if we're the Central Aggregator
                    this->broadcast(p);
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

void HierarchicalNetworkManager::handle_registration_requests()
{
    xbt_assert(this->my_node_info.role == NodeRole::MainAggregator);

    if (Constants::GENERATE_DOT_FILES)
    {
        DOTGenerator::get_instance().add_to_cluster(
            std::format("cluster-{}", this->my_node_info.name),
            std::format("{} [label=\"{}\", color=green]", this->my_node_info.name, this->my_node_info.name)
        );
    }

    for (auto request : *this->registration_requests)
    {
        this->connected_nodes->push_back(request.node_to_register); 

        if (Constants::GENERATE_DOT_FILES)
        {
            DOTGenerator::get_instance().add_to_cluster(
                std::format("cluster-{}", this->my_node_info.name),
                std::format("{} [label=\"{}\", color=yellow]", request.node_to_register.name, request.node_to_register.name)
            );

            DOTGenerator::get_instance().add_to_cluster(
                std::format("cluster-{}", this->my_node_info.name),
                std::format("{} -> {} [color=green]", this->my_node_info.name, request.node_to_register.name)
            );
        }

        auto node_list = vector<NodeInfo>();
        node_list.push_back(this->my_node_info);

        auto res_p = make_unique<Packet>(Packet(
            request.node_to_register.name, request.node_to_register.name,
            operations::RegistrationConfirmation(
                make_shared<vector<NodeInfo>>(node_list)
            )
        ));

        this->send_async(res_p);
    }

    this->mp->put_nm_event(
        new Mediator::Event {
            Mediator::ClusterConnected { .number_client_connected=(uint16_t)this->connected_nodes->size() }
        }
    );
}

void HierarchicalNetworkManager::send_registration_request()
{
    // This assert is'nt true in the case of hierarchical aggregator...
    // xbt_assert(this->my_node_info.role == NodeRole::Trainer);

    // Take the first bootstrap node, in a centralized topology we should have only one anyways.
    auto bootstrap_node = this->bootstrap_nodes->at(0);

    auto p = make_unique<Packet>(Packet(
        bootstrap_node.name, bootstrap_node.name,
        operations::RegistrationRequest(
            this->my_node_info
        )
    ));

    this->send_async(p);
}


void HierarchicalNetworkManager::handle_registration_confirmation(const operations::RegistrationConfirmation &confirmation)
{
    for (auto node: *confirmation.node_list)
    {
        this->connected_nodes->push_back(node);
    }

    this->mp->put_nm_event(
        new Mediator::Event { Mediator::NodeConnected {} }
    );
}

void HierarchicalNetworkManager::broadcast(const unique_ptr<Packet> &p, bool is_redirected)
{
    for(auto node_info : *this->connected_nodes)
    {
        p->dst = node_info.name;
        this->send_async(p);
    }
}

void HierarchicalNetworkManager::handle_kill_phase()
{
    // Wait to sent to kill packet to everyone on the network
    // This includes:
    // Central NM -> Hierarchical NM 
    // and 
    // Hierarchical NM -> real NM
    this->pending_async_put->wait_all();
}
