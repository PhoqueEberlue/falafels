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

AsynchronousAggregator::AsynchronousAggregator(std::unordered_map<std::string, std::string> *args)
{
    this->initialization_time = simgrid::s4u::Engine::get_instance()->get_clock();

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
    if (!this->still_has_activities)
        return;

    // Stop aggregating and send kills to the trainers
    if (simgrid::s4u::Engine::get_instance()->get_clock() > this->initialization_time + Constants::DURATION_TRAINING_PHASE)
    {
        this->send_kills();
        this->print_end_report();
        this->still_has_activities = false;
        return;
    }

    switch (this->state)
    {
        case INITIALIZING:
            if (auto e = this->get_nm_event())
            {
                if (auto *conneted_event = get_if<NetworkManager::ClusterConnected>(e->get()))
                {
                    this->number_client_training = conneted_event->number_client_connected;
                    this->send_global_model();
                    this->state = WAITING_LOCAL_MODELS;
                }
            }
            break;
        case WAITING_LOCAL_MODELS:
            if (auto packet = this->get_received_packet())
            {
                if (auto *send_local = get_if<Packet::SendLocalModel>(&(*packet)->op))
                {
                    this->number_local_models = 1;
                    this->src_save = (*packet)->src;
                    this->original_src_save = (*packet)->original_src;
                    this->state = AGGREGATING;
                }
            }
            break;
        case AGGREGATING:
            if (this->aggregate())
            {
                this->send_global_model_to(this->src_save, this->original_src_save);
                this->number_local_models = 0;
                this->state = WAITING_LOCAL_MODELS;
            }
            break;
    }
}

/* Sends the global model to every start_nodes */
void AsynchronousAggregator::send_global_model_to(node_name dst, node_name final_dst)
{
    auto p = Packet(
        dst, final_dst,
        Packet::SendGlobalModel(
            this->number_local_epochs
        )
    );

    this->put_to_be_sent_packet(p);
}
