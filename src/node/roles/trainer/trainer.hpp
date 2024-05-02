/* Trainer */
#ifndef FALAFELS_TRAINER_HPP
#define FALAFELS_TRAINER_HPP

#include "../role.hpp"
#include <cstdint>
#include <simgrid/forward.h>
#include <unordered_map>

class Trainer : public Role 
{
protected:
    using State = enum
    {
        WAITING_GLOBAL_MODEL,
        TRAINING,
    };

    State state;

    // TODO: find a better way to pass arguments between states
    node_name dst;
    node_name final_dst;

    uint8_t current_local_epoch = 0;
    uint8_t number_local_epochs = 0;
    simgrid::s4u::ExecPtr training_activity;

private:
    bool train();
    void send_local_model(node_name dst, node_name final_dst);
public:
    Trainer(std::unordered_map<std::string, std::string> *args);
    ~Trainer() {};
    bool run();
    NodeRole get_role_type() { return NodeRole::Trainer; };
};

#endif //! FALAFELS_TRAINER_HPP
