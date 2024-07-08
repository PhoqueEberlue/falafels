#include <format>
#include <memory>
#include <simgrid/s4u.hpp>
#include "hierarchical_aggregator.hpp"
#include "../../moms/hierarchical_mom.hpp"
#include "../../utils/utils.hpp"
#include "../../node.hpp"


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_hierachical_aggregator, "Messages specific for this example");

using namespace std;
using namespace protocol;

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
            case str2int("is_main_aggregator"):
                bool ima = std::stoi(value);
                XBT_INFO("is_main_aggregator=%b", ima);
                this->is_main_aggregator = ima;
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
        
    auto central_nm = new HierarchicalNetworkManager(my_node_info);

    // Set the central_aggregator_name as bootstrap node so we can register to it when the hierarchical_aggregator is run.
    central_nm->set_bootstrap_nodes(
        new vector<NodeInfo>({ 
            NodeInfo { .name=this->central_aggregator_name, .role=NodeRole::MainAggregator } 
        })
    );

    auto central_mc = make_unique<IMediatorConsumer>(new_node_name);
    auto central_mp = make_unique<IMediatorProducer>(new_node_name);

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

                // First prepare sending the first global model because some topology need to do
                // it first before in order to receive the ClusterConnected event
                if (this->first_global_model)
                {
                    // Waiting global model from the central aggregator
                    auto op = this->central_mc->get_received_operation();

                    // If the operation is a SendGlobalModel
                    if (auto *op_glob = get_if<operations::SendGlobalModel>(op.get()))
                    {
                        this->send_global_model();
                    }

                    this->first_global_model = false;
                }

                // Waiting connections of the nodes on our own cluster
                auto e = this->mc->get_nm_event();

                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::ClusterConnected>(e.get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    // skip directly to waiting local models as we already sent the first one
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
        case WAITING_GLOBAL_MODEL:
            {
                // Waiting global model from the central aggregator
                auto op = this->central_mc->get_received_operation();

                // If the operation is a SendGlobalModel
                if (auto *op_glob = get_if<operations::SendGlobalModel>(op.get()))
                {
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
        case WAITING_LOCAL_MODELS: 
            {
                // If a packet have been received
                auto op = this->mc->get_received_operation();

                // If the packet's operation is a SendLocalModel
                if (auto *send_local = get_if<operations::SendLocalModel>(op.get()))
                {
                    this->number_local_models += 1;
                    this->total_number_local_epochs += send_local->number_local_epochs_done;
                    this->current_number_local_epochs_cluster += send_local->number_local_epochs_done;

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

                this->send_model_to_central_aggregator();

                // Reset numbers
                this->number_local_models = 0;
                this->current_number_local_epochs_cluster = 0;
                this->state = WAITING_GLOBAL_MODEL;
            }
            break;
    }
}

void HierarchicalAggregator::send_model_to_central_aggregator()
{
    // Send as if it was a local model (which is the case in theory?).
    this->central_mc->put_async_to_be_sent_packet(
        filters::aggregators,
        operations::SendLocalModel(this->current_number_local_epochs_cluster)
    );
}
