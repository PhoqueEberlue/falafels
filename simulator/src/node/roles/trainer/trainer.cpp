#include "../../../constants.hpp"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <unordered_map>
#include <variant>
#include <xbt/log.h>
#include "trainer.hpp"
#include "simgrid/s4u/Exec.hpp"
#include "simgrid/s4u/Host.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

Trainer::Trainer(std::unordered_map<std::string, std::string> *args, node_name name) 
{
    this->my_node_name = name;
    this->training_activities = new simgrid::s4u::ActivitySet();

    // No arguments yet
    delete args;
}

void Trainer::train() 
{
    double flops = Constants::LOCAL_MODEL_TRAINING_FLOPS;
    
    int nb_core = simgrid::s4u::this_actor::get_host()->get_core_count();
    double total_nb_flops_per_epoch = (flops / nb_core) * this->number_local_epochs;

    XBT_DEBUG("(flops / nb_core) * nb_local_epochs = total_nb_flops_per_epoch <-> (%f / %i) * %u = %f",
              flops, nb_core, this->number_local_epochs, total_nb_flops_per_epoch);
    
    // TODO: maybe actually use simgrid functions to launch in parallel???
    // Launch exactly nb_core parallel tasks
    for (int i = 0; i < nb_core; i++)
    {
        auto exec = simgrid::s4u::this_actor::exec_async(total_nb_flops_per_epoch);
        this->training_activities->push(exec);
    }

    this->training_activities->wait_all();
}

void Trainer::send_local_model(node_name dst, node_name final_dst)
{
    this->mc->put_to_be_sent_packet(
        new Packet(
            dst, final_dst,
            Packet::SendLocalModel()
        )
    );
}

void Trainer::run()
{
    switch (this->state)
    {
        case INITIALIZING:
            {
                auto e = this->mc->get_nm_event();

                // If type of event is NodeConnected it means that our node is connected :)
                if (auto *conneted_event = get_if<Mediator::NodeConnected>(e.get()))
                {
                    this->state = WAITING_GLOBAL_MODEL;
                }
                break;
            }
        case WAITING_GLOBAL_MODEL:
            {
                auto packet = this->mc->get_received_packet();

                // If the operation is a SendGlobalModel
                if (auto *op_glob = get_if<Packet::SendGlobalModel>(&packet->op))
                {
                    // Get the source to be able to send the local model later
                    XBT_INFO("src: %s", packet->src.c_str());
                    XBT_INFO("original_src: %s", packet->original_src.c_str());
                    this->dst = packet->src;
                    this->final_dst = packet->original_src;

                    // Set the number of local epochs
                    this->number_local_epochs = op_glob->number_local_epochs;
                    this->state = TRAINING;
                }
                break;
            }
        case TRAINING:
            {
                this->train();
                this->send_local_model(this->dst, this->final_dst);
                this->state = WAITING_GLOBAL_MODEL;
                break;
            }
    }
}
