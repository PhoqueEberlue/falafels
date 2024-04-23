#include "../../../constants.hpp"
#include <cstdint>
#include <cstdlib>
#include <memory>
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

void Trainer::train(uint8_t number_local_epochs) 
{
    double flops = Constants::LOCAL_MODEL_TRAINING_FLOPS;

    XBT_INFO("Starting local training with flops value `%f` and %i epochs", flops, number_local_epochs);
    for (int i = 0; i < number_local_epochs; i++)
    {
        // XBT_INFO("Epoch %i ====> ...", i);
        simgrid::s4u::this_actor::execute(flops);
    }
}

void Trainer::send_local_model(node_name dst, node_name final_dst)
{
    auto nm = this->get_network_manager();

    auto p = make_shared<Packet>(Packet(
        this->get_network_manager()->get_my_node_name(), final_dst,
        Packet::SendLocalModel()
    ));

    nm->send(p, dst, 10);
}

void Trainer::run()
{
    auto nm = this->get_network_manager();
    std::unique_ptr<Packet> p;
    bool run = true;

    // Wait for the current node to be registered by the Aggregator
    this->get_network_manager()->send_registration_request();

    while (run)
    {
        p = nm->get_packet();

        if (auto *op_glob = get_if<Packet::SendGlobalModel>(&p->op))
        {
            this->train(op_glob->number_local_epochs);

            this->send_local_model(p->src, p->original_src);
        }
        else if (auto *op_kill = get_if<Packet::KillTrainer>(&p->op))
        {
            run = false;
        }
    }

    // Wait last comms before exiting
    nm->wait_last_comms(2);
}
