#include <cstdint>
#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <variant>
#include <xbt/log.h>

#include "simple_aggregator.hpp"
#include "../../../protocol.hpp"
#include "../../../utils/utils.hpp"
#include "aggregator.hpp"


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_simple_aggregator, "Messages specific for this example");

using namespace std;
using namespace protocol;

SimpleAggregator::SimpleAggregator(std::unordered_map<std::string, std::string> *args, node_name name) : Aggregator(name)
{
    // Parsing arguments
    for (auto &[key, value]: *args)
    {
        switch (str2int(key.c_str()))
        {
            case str2int("is_main_aggregator"):
                bool ima = std::stoi(value);
                XBT_INFO("is_main_aggregator=%b", ima);
                this->is_main_aggregator = ima;
                break;
        }
    }

    delete args;
}

void SimpleAggregator::run()
{
    switch (this->state)
    {
        case INITIALIZING:
            {

                if (this->first_global_model)
                {
                    this->send_global_model();
                    this->first_global_model = false;
                }

                // Then wait for the event that tells us the number of connected client
                auto e = this->mc->get_nm_event();

                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::ClusterConnected>(e.get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
        case WAITING_LOCAL_MODELS: 
            {
                auto op = this->mc->get_received_operation();

                // If the packet's operation is a SendLocalModel
                if (auto *op_send_local = get_if<operations::SendLocalModel>(op.get()))
                {
                    this->number_local_models += 1;
                    this->total_number_local_epochs += op_send_local->number_local_epochs_done;
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
                this->aggregate();

                // Only check end condition as MainAggregator
                if (this->get_role_type() == NodeRole::MainAggregator 
                    && this->check_end_condition())
                {
                    this->print_end_report();
                    // Stop aggregating and send kills to the trainers
                    this->send_kills();
                }
                else
                {
                    this->send_global_model();
                    this->number_local_models = 0;
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
    }
}
