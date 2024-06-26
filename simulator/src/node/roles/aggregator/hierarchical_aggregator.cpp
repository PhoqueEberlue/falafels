#include <format>
#include <memory>
#include <simgrid/s4u.hpp>
#include <xbt/log.h>
#include "hierarchical_aggregator.hpp"
#include "../../../utils/utils.hpp"
#include "../../node.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_hierachical_aggregator, "Messages specific for this example");

HierarchicalAggregator::HierarchicalAggregator(std::unordered_map<std::string, std::string> *args, node_name name) : Aggregator(name)
{
    for (auto &[key, value]: *args)
    {
        switch (str2int(key.c_str()))
        {
            case str2int("central_aggregator_name"):
                XBT_INFO("central_aggregator_name=%s", value.c_str());
                this->central_aggregator_name = value;
                break;
        }
    } 

    this->setup_central_nm();

    delete args;
}

void HierarchicalAggregator::setup_central_nm()
{
    // Create a new node name to be used for the mailbox that will be receiving and sending to the central aggregator
    auto new_node_name = format("hierarchical_{}", this->my_node_name);

    // Pretend we are a trainer to be able to register to the central aggregator
    auto my_node_info = NodeInfo { .name = new_node_name, .role = NodeRole::Trainer};
        
    auto central_nm = new StarNetworkManager(my_node_info);

    // Set the central_aggregator_name as bootstrap node so we can register to it when the hierarchical_aggregator is run.
    central_nm->set_bootstrap_nodes(
        new vector<NodeInfo>({ 
            NodeInfo { .name=this->central_aggregator_name, .role=NodeRole::Aggregator } 
        })
    );

    auto central_mc = make_unique<MediatorConsumer>(new_node_name);
    auto central_mp = make_unique<MediatorProducer>(new_node_name);

    this->central_mc = std::move(central_mc);

    central_nm->set_mediator_producer(std::move(central_mp));

    auto e = simgrid::s4u::Engine::get_instance();

    simgrid::s4u::Actor::create(
        std::format("{}_nm_hierarchical", this->my_node_name), e->host_by_name(this->my_node_name), &Node::run_network_manager, central_nm 
    );
}

void HierarchicalAggregator::run()
{
    switch (this->state)
    {
        case INITIALIZING_CENTRAL:
            {
                // Waiting connection on the central aggregator 
                auto e = this->central_mc->get_nm_event();

                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::NodeConnected>(e.get()))
                {
                    this->state = INITIALIZING_CLUSTER;
                }

                break;
            }
        case INITIALIZING_CLUSTER:
            {
                // Waiting connections of the nodes on our own cluster
                auto e = this->mc->get_nm_event();

                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::ClusterConnected>(e.get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    this->state = WAITING_GLOBAL_MODEL;
                }
                break;
            }
        case WAITING_GLOBAL_MODEL:
            {
                // Waiting global model from the central aggregator
                auto packet = this->central_mc->get_received_packet();

                // If the operation is a SendGlobalModel
                if (auto *op_glob = get_if<Packet::SendGlobalModel>(&packet->op))
                {
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
                // Note that the hierarchical aggregator receives kill packet from the central_nm
                break;
            }
        case WAITING_LOCAL_MODELS: 
            {
                // If a packet have been received
                auto packet = this->mc->get_received_packet();

                // If the packet's operation is a SendLocalModel
                if (auto *send_local = get_if<Packet::SendLocalModel>(&packet->op))
                {
                    this->number_local_models += 1;
                    XBT_INFO("nb local models: %lu", this->number_local_models);

                    if (this->number_local_models >= this->number_client_training)
                    {
                        this->state = AGGREGATING;
                    }
                }
                break;
            }
        case AGGREGATING:
            {
                // If the aggregating activity has finished (start it if not launched)
                this->aggregate();

                this->number_local_models = 0;
                this->send_model_to_central_aggregator();
                this->state = WAITING_GLOBAL_MODEL;
            }
            break;
    }
}

void HierarchicalAggregator::send_model_to_central_aggregator()
{
    // Send as if it was a local model (which is the case in theory?).
    auto p = new Packet(
        this->central_aggregator_name, this->central_aggregator_name,
        Packet::SendLocalModel()
    );

    this->central_mc->put_to_be_sent_packet(p);
}
