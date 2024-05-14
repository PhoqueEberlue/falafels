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

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

Trainer::Trainer(std::unordered_map<std::string, std::string> *args, node_name name) 
{
    this->my_node_name = name;

    // No arguments yet
    delete args;
}

bool Trainer::train() 
{
    double flops = Constants::LOCAL_MODEL_TRAINING_FLOPS;

    // if uninitialized
    if (this->training_activity == nullptr)
        this->training_activity = simgrid::s4u::this_actor::exec_async(flops);

    // if current training activity is finished
    if (this->training_activity->test())
    {
        // if we need to perform more local epoch
        if (this->current_local_epoch < this->number_local_epochs)
        {
            this->current_local_epoch += 1;

            // XBT_INFO("Starting local training with flops value `%f` and %i epochs", flops, number_local_epochs);

            XBT_INFO("Epoch %i ====> ...", this->current_local_epoch);
            this->training_activity = simgrid::s4u::this_actor::exec_async(flops);
        }
        else
        {
            this->current_local_epoch = 0;
            // Return true when the last activity have been finished
            return true;
        }
    }

    return false;
}

void Trainer::send_local_model(node_name dst, node_name final_dst)
{
    this->mc->put_to_be_sent_packet(
        Packet(
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
            // If event available
            if (auto e = this->mc->get_nm_event())
            {
                // If type of event is NodeConnected it means that our node is connected :)
                if (auto *conneted_event = get_if<Mediator::NodeConnected>(e->get()))
                {
                    this->state = WAITING_GLOBAL_MODEL;
                }
            }
            break;
        case WAITING_GLOBAL_MODEL:
            // If packet have been received 
            if (auto packet = this->mc->get_received_packet())
            {
                // If the operation is a SendGlobalModel
                if (auto *op_glob = get_if<Packet::SendGlobalModel>(&(*packet)->op))
                {
                    // Get the source to be able to send the local model later
                    this->dst = (*packet)->src;
                    this->final_dst = (*packet)->original_src;

                    // Set the number of local epochs
                    this->number_local_epochs = op_glob->number_local_epochs;
                    this->state = TRAINING;
                }
            }
            break;
        case TRAINING:
            // If the training activity has finished (start it if not launched)
            if (this->train())
            {
                this->send_local_model(this->dst, this->final_dst);
                this->state = WAITING_GLOBAL_MODEL;
            }
            break;
    }
}
