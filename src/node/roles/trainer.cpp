#include "../../constants.hpp"
#include "trainer.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_trainer, "Messages specific for this example");

Trainer::Trainer() {}

void Trainer::run()
{
    Packet *p = this->get_network_manager()->get();

    while (p->op != Packet::Operation::KILL_TRAINER)
    {
        XBT_INFO("Received message: %s", operation_to_str(p->op));

        if (p->op == Packet::Operation::SEND_GLOBAL_MODEL)
        {
            double flops = constants::trainer::LOCAL_EPOCH_FLOPS;
            XBT_INFO("Starting local training with flops value: %f", flops);

            simgrid::s4u::this_actor::execute(flops);

            // Retrieving src's mailbox
            auto source_mailbox = simgrid::s4u::Mailbox::by_name(p->src);

            Packet *res_p = new Packet { .op=Packet::Operation::SEND_LOCAL_MODEL, .src=this->get_network_manager()->get_my_host_name() };

            XBT_INFO("Sending local model to: %s", p->src.c_str());
            source_mailbox->put(res_p, constants::MODEL_SIZE_BYTES);
        }

        // Delete current packet and wait for another one 
        delete p;
        p = this->get_network_manager()->get();
    }

    // Delete kill packet 
    delete p;

    simgrid::s4u::this_actor::exit();
}
