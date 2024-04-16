#include "../../../constants.hpp"
#include <cstdint>
#include <cstdlib>
#include <unordered_map>
#include <xbt/log.h>
#include "trainer.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

Trainer::Trainer(std::unordered_map<std::string, std::string> *args) 
{
    // No arguments yet
    delete args;
}

void Trainer::train(uint8_t number_local_epochs) 
{
    double flops = Constants::LOCAL_MODEL_TRAINING_FLOPS;

    XBT_INFO("Starting local training with flops value per epoch: %f", flops);
    for (int i = 0; i < number_local_epochs; i++)
    {
        // XBT_INFO("Epoch %i ====> ...", i);
        simgrid::s4u::this_actor::execute(flops);
    }
}

void Trainer::send_local_model(node_name dst, node_name final_dst)
{
    auto nm = this->get_network_manager();

    Packet *p = new Packet(Packet::Operation::SEND_LOCAL_MODEL, this->get_network_manager()->get_my_node_name(), final_dst);

    nm->send(p, dst, 10);

    // Delete our own reference of the packet
    p->decr_ref_count();
}

void Trainer::run()
{
    auto nm = this->get_network_manager();
    Packet *p;
    bool run = true;

    while (run)
    {
        p = nm->get();

        switch (p->op)
        {
            case Packet::Operation::SEND_GLOBAL_MODEL:
                {
                    std::string t = p->args->at("number_local_epochs");
                    uint8_t number_local_epochs = (uint8_t) atoi(t.c_str());

                    node_name source = p->src;
                    node_name original_src = p->original_src;
                    // p->decr_ref_count();

                    this->train(number_local_epochs);
                    this->send_local_model(source, original_src);
                    break;
                }
            case Packet::Operation::KILL_TRAINER:
                {
                    run = false;
                    // p->decr_ref_count();
                    break;
                }
            case Packet::Operation::SEND_LOCAL_MODEL:
                {
                    // p->decr_ref_count();
                    // Ignore, the packet is automatically redirected when using a decentralized nm i.e. ring nm.
                    break;
                }
        }

        delete p;
    }

    nm->wait_last_comms();

    XBT_INFO("Exiting");
    simgrid::s4u::this_actor::exit();
}
