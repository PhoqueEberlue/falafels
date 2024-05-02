#include "../../../constants.hpp"
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <simgrid/s4u/Actor.hpp>
#include <unordered_map>
#include <variant>
#include <xbt/log.h>
#include "trainer.hpp"

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

Trainer::Trainer(std::unordered_map<std::string, std::string> *args) 
{
    // No arguments yet
    delete args;
}

/** 
 * Launch the training activity or test if the current one has finished.
 * Performs number_local_epochs training before return true.
 */
bool Trainer::train() 
{
    // if current training activity is finished
    if (this->training_activity->test())
    {
        // if we need to perform more local epoch
        if (this->current_local_epoch < this->number_local_epochs)
        {
            this->current_local_epoch += 1;
            double flops = Constants::LOCAL_MODEL_TRAINING_FLOPS;

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
    auto p = make_shared<Packet>(Packet(
        dst, final_dst,
        Packet::SendLocalModel()
    ));

    this->put_to_be_sent_packet(p);
}

bool Trainer::run()
{
    switch (this->state)
    {
        case WAITING_GLOBAL_MODEL:
            if (auto packet = this->get_received_packet())
            {
                if (auto *op_glob = get_if<Packet::SendGlobalModel>(&(*packet)->op))
                {
                    this->dst = (*packet)->dst;
                    this->final_dst = (*packet)->final_dst;
                    this->number_local_epochs = op_glob->number_local_epochs;
                    this->state = TRAINING;
                }
                else if (get_if<Packet::KillTrainer>(&(*packet)->op))
                {
                    // Stop trainer execution
                    return false;
                }
            }
            break;
        case TRAINING:
            if (this->train())
            {
                this->send_local_model(this->dst, this->final_dst);
                this->state = WAITING_GLOBAL_MODEL;
            }
            break;
    }

    return true;
}
