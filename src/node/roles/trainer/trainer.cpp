#include "../../../constants.hpp"
#include <xbt/log.h>
#include "trainer.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

Trainer::Trainer() {}

void Trainer::train() 
{
    double flops = constants::trainer::LOCAL_EPOCH_FLOPS;
    XBT_INFO("Starting local training with flops value: %f", flops);
    simgrid::s4u::this_actor::execute(flops);
}

void Trainer::send_local_model(node_name dest)
{
    auto nm = this->get_network_manager();

    Packet *res_p = new Packet { .op=Packet::Operation::SEND_LOCAL_MODEL, .src=nm->get_my_node_name() };

    XBT_INFO("%s ---%s--> %s", nm->get_my_node_name().c_str(), operation_to_str(res_p->op), dest.c_str());

    nm->put_timeout(res_p, dest, constants::MODEL_SIZE_BYTES, 10);
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
                    this->train();
                    this->send_local_model(p->src);
                    break;
                }
            case Packet::Operation::KILL_TRAINER:
                {
                    run = false;
                    break;
                }
        }

        // Delete current packet
        delete p;
    }

    XBT_INFO("Exiting");
    simgrid::s4u::this_actor::exit();
}
