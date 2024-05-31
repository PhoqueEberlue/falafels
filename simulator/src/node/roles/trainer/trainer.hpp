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
        INITIALIZING,
        WAITING_GLOBAL_MODEL,
        TRAINING,
    };

    /** State of the Trainer */
    State state = INITIALIZING;

    // TODO: find a better way to pass arguments between states
    // The destination we will send our local model
    node_name dst;
    node_name final_dst;

    /** Tracks the current number of local epoch done by the current trainer */
    uint8_t current_local_epoch = 0;

    /** The total number of local epochs to perform */
    uint8_t number_local_epochs = 0;

    /** Simgrid ActivitySet containing the training tasks */
    simgrid::s4u::ActivitySet *training_activities;

    /**
     * Launch one trainer epoch in parallel, sharing activities among the Host's cores.
     */
    void launch_one_epoch();

    /** 
     * Launch the training activity or test if the current one has finished.
     * @return true if number_local_epochs have been performed, false otherwise.
     */
    bool train();

    /** Send the local model to (dst, final_dst) */
    void send_local_model(node_name dst, node_name final_dst);
public:
    Trainer(std::unordered_map<std::string, std::string> *args, node_name);
    ~Trainer() {};

    /** Run one step of the trainer. */
    void run();

    NodeRole get_role_type() { return NodeRole::Trainer; };
};

#endif //! FALAFELS_TRAINER_HPP
