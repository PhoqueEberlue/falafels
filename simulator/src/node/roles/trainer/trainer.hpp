/* Trainer */
#ifndef FALAFELS_TRAINER_HPP
#define FALAFELS_TRAINER_HPP

#include "../role.hpp"
#include <cstdint>
#include <simgrid/forward.h>
#include <simgrid/s4u/ActivitySet.hpp>
#include <unordered_map>
#include <simgrid/s4u/Activity.hpp>

class Trainer : public Role 
{
protected:
    using State = enum
    {
        WAITING_GLOBAL_MODEL,
        TRAINING,
    };

    /** State of the Trainer */
    State state = WAITING_GLOBAL_MODEL;

    /** The total number of local epochs to perform */
    uint8_t number_local_epochs = 0;

    /** Simgrid ActivitySet containing the training tasks */
    simgrid::s4u::ActivitySet *training_activities;

    /** Run and wait the training activities in parallel. */
    void train();

    /** Send the local model to aggregator(s) */
    void send_local_model();
public:
    Trainer(std::unordered_map<std::string, std::string> *args, protocol::node_name);
    ~Trainer() { delete this->training_activities; };

    /** Run one step of the trainer. */
    void run();

    protocol::NodeRole get_role_type() { return protocol::NodeRole::Trainer; };
};

#endif //! FALAFELS_TRAINER_HPP
