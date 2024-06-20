#include <cstdint>
#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <variant>
#include <xbt/log.h>

#include "simple_aggregator.hpp"
#include "../../../protocol.hpp"
#include "aggregator.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_simple_aggregator, "Messages specific for this example");

SimpleAggregator::SimpleAggregator(std::unordered_map<std::string, std::string> *args, node_name name) : Aggregator(name)
{
    // No args needed
    delete args;
}

void SimpleAggregator::run()
{
    switch (this->state)
    {
        case INITIALIZING:
            {
                auto e = this->mc->get_nm_event();

                // If type of event is ClusterConnected it means that every node have been connected to us
                if (auto *conneted_event = get_if<Mediator::ClusterConnected>(e.get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
        case WAITING_LOCAL_MODELS: 
            {
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
                this->aggregate();

                if (this->check_end_condition())
                {
                    // Stop aggregating and send kills to the trainers
                    this->send_kills();
                    this->print_end_report();
                }
                else
                {
                    this->number_local_models = 0;
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
    }
}
