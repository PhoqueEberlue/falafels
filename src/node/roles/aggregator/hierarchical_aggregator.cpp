#include <format>
#include <memory>
#include <simgrid/s4u.hpp>
#include <xbt/log.h>
#include "hierarchical_aggregator.hpp"
#include "../../../utils/utils.hpp"

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
    auto my_node_name = this->my_node_name;

    // Create a new node name to be used for the mailbox that will be receiving and sending to the central aggregator
    auto new_node_name = format("hierarchical_{}", my_node_name);

    // Pretend we are a trainer to be able to register to the central aggregator
    auto my_node_info = NodeInfo { .name = new_node_name, .role = NodeRole::Trainer};
        
    this->central_nm = make_unique<StarNetworkManager>(my_node_info);

    // Set the centra_aggregator_name as bootstrap node so we can register to it when the hierarchical_aggregator is run.
    this->central_nm->set_bootstrap_nodes(
        new vector<NodeInfo>({ 
            NodeInfo { .name=this->central_aggregator_name, .role=NodeRole::Aggregator } 
        })
    );

    // Creates queues to enable communication between Role and central NM.
    auto received_packets = make_shared<queue<unique_ptr<Packet>>>();
    auto to_be_sent_packets = make_shared<queue<shared_ptr<Packet>>>();
    auto nm_events = make_shared<queue<unique_ptr<Mediator::Event>>>();

    auto central_mc = make_unique<MediatorConsumer>(received_packets, to_be_sent_packets, nm_events);
    auto central_mp = make_unique<MediatorProducer>(received_packets, to_be_sent_packets, nm_events);

    this->central_mc = std::move(central_mc);

    this->central_nm->set_mediator_producer(std::move(central_mp));
}

void HierarchicalAggregator::run()
{
    // Run central_nm in parallel
    this->central_nm->run();

    if (!this->still_has_activities)
        return;

    switch (this->state)
    {
        case INITIALIZING_CENTRAL:
            // Waiting connection on the central aggregator 
            if (auto e = this->central_mc->get_nm_event())
            {
                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::NodeConnected>(e->get()))
                {
                    this->state = INITIALIZING_CLUSTER;
                }
            }
            break;
        case INITIALIZING_CLUSTER:
            // Waiting connections of the nodes on our own cluster
            if (auto e = this->mc->get_nm_event())
            {
                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::ClusterConnected>(e->get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    this->state = WAITING_GLOBAL_MODEL;
                }
            }
            break;
        case WAITING_GLOBAL_MODEL:
            // Waiting global model from the central aggregator
            if (auto packet = this->central_mc->get_received_packet())
            {
                // If the operation is a SendGlobalModel
                if (auto *op_glob = get_if<Packet::SendGlobalModel>(&(*packet)->op))
                {
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
                else if (auto *op_kill = get_if<Packet::KillTrainer>(&(*packet)->op)) 
                {
                    this->send_kills();
                    this->print_end_report();
                    this->still_has_activities = false;
                    break;
                }
            }
            break;
        case WAITING_LOCAL_MODELS: 
            // If a packet have been received
            if (auto packet = this->mc->get_received_packet())
            {
                // If the packet's operation is a SendLocalModel
                if (auto *send_local = get_if<Packet::SendLocalModel>(&(*packet)->op))
                {
                    this->number_local_models += 1;
                    XBT_INFO("nb local models: %lu", this->number_local_models);

                    if (this->number_local_models >= this->number_client_training)
                    {
                        this->state = AGGREGATING;
                    }
                }
            }
            break;
        case AGGREGATING:
            // If the aggregating activity has finished (start it if not launched)
            if (this->aggregate())
            {
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
    auto p = Packet(
        this->central_aggregator_name, this->central_aggregator_name,
        Packet::SendLocalModel()
    );

    this->central_mc->put_to_be_sent_packet(p);
}
