#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <string>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <xbt/log.h>

#include "asynchronous_aggregator.hpp"
#include "../../network_managers/nm.hpp"
#include "../../../protocol.hpp"
#include "../../../utils/utils.hpp"


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_asynchronous_aggregator, "Messages specific for this example");

using namespace std;
using namespace protocol;

AsynchronousAggregator::AsynchronousAggregator(
    std::unordered_map<std::string, std::string> *args, node_name name) : Aggregator(name)
{
    // Parsing arguments
    for (auto &[key, value]: *args)
    {
        switch (str2int(key.c_str()))
        {
            case str2int("proportion_threshold"):
                {
                    XBT_INFO("proportion_threshold=%s", value.c_str());
                    this->proportion_threshold = std::stof(value);
                    break;
                }
            case str2int("is_main_aggregator"):
                {
                    bool ima = std::stoi(value);
                    XBT_INFO("is_main_aggregator=%b", ima);
                    this->is_main_aggregator = ima;
                    break;
                }
            case str2int("number_local_epochs"):
                {
                    int nble = std::stoi(value);
                    XBT_INFO("number_local_epochs=%i", nble);
                    this->number_local_epochs = nble;
                    break;
                }
        }
    }

    delete args;
}

void AsynchronousAggregator::run()
{
    switch (this->state)
    {
        case INITIALIZING:
            {
                this->send_global_model();

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

                // If the operation is a SendLocalModel
                if (auto *op_send_local = get_if<operations::SendLocalModel>(op.get()))
                {
                    this->number_local_models += 1;
                    this->total_number_local_epochs += op_send_local->number_local_epochs_done;

                    if (this->number_local_models >= this->number_client_training * this->proportion_threshold)
                    {
                        XBT_INFO("Received %lu local models, starting aggregation", this->number_local_models);
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
