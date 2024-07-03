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


XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

using namespace std;
using namespace protocol;

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

void Trainer::send_local_model()
{
    this->mc->put_async_to_be_sent_packet(
        filters::aggregators,
        operations::SendLocalModel(this->number_local_epochs)
    );
}

void Trainer::run()
{
    switch (this->state)
    {
        case WAITING_GLOBAL_MODEL:
            {
                auto op = this->mc->get_received_operation();

                // If the operation is a SendGlobalModel
                if (auto *op_glob = get_if<operations::SendGlobalModel>(op.get()))
                {
                    // Set the number of local epochs
                    this->number_local_epochs = op_glob->number_local_epochs;
                    this->state = TRAINING;
                }
                break;
            }
        case TRAINING:
            {
                this->train();
                this->send_local_model();
                this->state = WAITING_GLOBAL_MODEL;
                break;
            }
    }
}
