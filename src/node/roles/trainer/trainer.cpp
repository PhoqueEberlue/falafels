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
        XBT_INFO("Epoch %i ====> ...", i);
        simgrid::s4u::this_actor::execute(flops);
    }
}

void Trainer::send_local_model(node_name dest)
{
    auto nm = this->get_network_manager();

    Packet *res_p = new Packet { .op=Packet::Operation::SEND_LOCAL_MODEL, .src=nm->get_my_node_name() };

    XBT_INFO("%s ---%s--> %s", nm->get_my_node_name().c_str(), operation_to_str(res_p->op), dest.c_str());

    nm->put_timeout(res_p, dest, 10);
}

void Trainer::run()
{
    auto nm = this->get_network_manager();
    Packet *p;
    bool run = true;

    while (run)
    {
        p = nm->get();
        XBT_INFO("%s <--%s--- %s", nm->get_my_node_name().c_str(), operation_to_str(p->op), p->src.c_str());

        switch (p->op)
        {
            case Packet::Operation::SEND_GLOBAL_MODEL:
                {
                    std::string t = p->args->at("number_local_epochs");
                    uint8_t number_local_epochs = (uint8_t) atoi(t.c_str());

                    this->train(number_local_epochs);
                    this->send_local_model(p->src);
                    break;
                }
            case Packet::Operation::KILL_TRAINER:
                {
                    run = false;
                    break;
                }
        }

        // Delete current packet with potential arguments
        if (p->args) delete p->args;
        delete p;
    }

    XBT_INFO("Exiting");
    simgrid::s4u::this_actor::exit();
}
