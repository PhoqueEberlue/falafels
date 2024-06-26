#include <simgrid/s4u/Engine.hpp>
#include <simgrid/s4u/Mailbox.hpp>
#include <tuple>
#include <unordered_map>
#include <variant>
#include <xbt/log.h>

#include "asynchronous_aggregator.hpp"
#include "../../network_managers/nm.hpp"
#include "../../../protocol.hpp"
#include "../../../utils/utils.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_asynchronous_aggregator, "Messages specific for this example");

AsynchronousAggregator::AsynchronousAggregator(std::unordered_map<std::string, std::string> *args, node_name name) : Aggregator(name)
{
    // Parsing arguments
    for (auto &[key, value]: *args)
    {
        switch (str2int(key.c_str()))
        {
            case str2int("proportion_threshold"):
                XBT_INFO("proportion_threshold=%s", value.c_str());
                this->proportion_threshold = std::stof(value);
                break;
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

                // If the operation is a SendLocalModel
                if (auto *send_local = get_if<Packet::SendLocalModel>(&packet->op))
                {
                    this->number_local_models = 1;
                    this->src_save = packet->src;
                    this->original_src_save = packet->original_src;
                    this->state = AGGREGATING;
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
                    this->send_global_model_to(this->src_save, this->original_src_save);
                    this->number_local_models = 0;
                    this->state = WAITING_LOCAL_MODELS;
                }
                break;
            }
    }
}

void AsynchronousAggregator::send_global_model_to(node_name dst, node_name final_dst)
{
    auto p = new Packet(
        dst, final_dst,
        Packet::SendGlobalModel(
            this->number_local_epochs
        )
    );

    this->mc->put_to_be_sent_packet(p);
}
