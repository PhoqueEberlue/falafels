/* Aggregator */
#ifndef FALAFELS_AGGREGATOR_HPP
#define FALAFELS_AGGREGATOR_HPP

#include "../role.hpp"
#include <cstdint>
#include <simgrid/forward.h>
#include <simgrid/s4u/Exec.hpp>

class Aggregator : public Role 
{
private:
    simgrid::s4u::ExecPtr training_activity;
protected:
    using State = enum
    {
        INITIALIZING,
        WAITING_LOCAL_MODELS,
        AGGREGATING,
    };

    State state = INITIALIZING;

    /** Value indicating the number of local epochs that the aggregator will ask the trainers to do. */
    uint8_t number_local_epochs = 3;

    /** Value indicating the number of global epochs achieved by the aggregator */
    uint16_t number_global_epochs = 0;

    /** Number of local model aggregated, used to compute the global number of epochs. */
    uint64_t number_aggregated_models = 0;

    /** The actual number of trainers */
    uint16_t number_client_training = 0;

    /** Number of the local models collected at a moment in time */
    uint64_t number_local_models = 0;    

    simgrid::s4u::ExecPtr aggregating_activity = nullptr;

    /** Time when the aggregator has been initialized */
    double initialization_time;

    bool aggregate();
    void print_end_report();
public:
    Aggregator() {}
    virtual ~Aggregator() {}
    virtual void run() = 0;

    void send_global_model();
    void send_kills();
    NodeRole get_role_type() { return NodeRole::Aggregator; };
};

#endif // !FALAFELS_AGGREGATOR_HPP
