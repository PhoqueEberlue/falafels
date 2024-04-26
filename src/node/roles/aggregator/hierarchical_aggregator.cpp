#include <format>
#include <memory>
#include <simgrid/s4u.hpp>
#include <xbt/log.h>
#include "hierarchical_aggregator.hpp"
#include "../../../utils/utils.hpp"
#include "simple_aggregator.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_hierachical_aggregator, "Messages specific for this example");

HierarchicalAggregator::HierarchicalAggregator(std::unordered_map<std::string, std::string> *args) : SimpleAggregator(args)
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

    // Arguments are deleted by the parent class
    // delete args
}

void HierarchicalAggregator::run()
{
    auto my_node_info = this->get_network_manager()->get_my_node_info();

    // Create a new node name to be used for the mailbox that will be receiving and sending to the central aggregator
    my_node_info.name = format("hierarchical-{}", my_node_info.name);
    this->central_nm = make_unique<StarNetworkManager>(my_node_info);

    // Set the centra_aggregator_name as bootstrap node so we can register to it when the hierarchical_aggregator is run.
    this->central_nm->set_bootstrap_nodes(
        new vector<NodeInfo>({ 
            NodeInfo { .name=this->central_aggregator_name, .role=NodeRole::Aggregator } 
        })
    );

    // Register self to the centralized aggregator.
    this->central_nm->send_registration_request();

    // Wait for the trainers to register.
    this->number_client_training = 
        this->get_network_manager()->handle_registration_requests();
 


    auto current_sim_time = simgrid::s4u::Engine::get_instance()->get_clock();


    unique_ptr<Packet> p;

    while (simgrid::s4u::Engine::get_instance()->get_clock() < current_sim_time + Constants::DURATION_TRAINING_PHASE)
    {
        p = this->central_nm->get_packet();

        // If global model received from the central aggregator
        if (auto *op_glob = get_if<Packet::SendGlobalModel>(&p->op))
        {
            // Resend to the trainers of our cluster
            this->send_global_model();
            uint64_t number_local_models = this->wait_local_models();
            this->aggregate(number_local_models);
            this->send_model_to_central_aggregator(p->src);
        }
    } 

    this->send_kills();

    this->print_end_report();
}

void HierarchicalAggregator::send_model_to_central_aggregator(node_name dst)
{
    // Send as if it was a local model (which is the case in theory?).
    auto p = make_shared<Packet>(Packet(
        this->central_nm->get_my_node_name(), dst,
        Packet::SendLocalModel()
    ));

    this->central_nm->send(p, dst, 10);
}
