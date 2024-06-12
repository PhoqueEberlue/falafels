/* Aggregator */
#ifndef FALAFELS_AGGREGATOR_HPP
#define FALAFELS_AGGREGATOR_HPP

#include "../role.hpp"
#include <cstdint>
#include <simgrid/forward.h>
#include <simgrid/s4u/Exec.hpp>

class Aggregator : public Role 
{
protected:
    /** Value indicating the number of local epochs that the aggregator will ask the trainers to do. */
    uint8_t number_local_epochs = 3;

    /** Value indicating the number of global epochs achieved by the aggregator */
    uint16_t number_global_epochs = 0;

    /** Number of local model aggregated, used to compute the global number of epochs. */
    uint64_t total_aggregated_models = 0;

    /** The actual number of trainers */
    uint16_t number_client_training = 0;

    /** Number of the local models collected at a moment in time */
    uint64_t number_local_models = 0;    

    /** Number of aggregated models on one aggregation task */
    uint64_t current_number_aggregated_models = 0;

    /** Time when the aggregator has been initialized */
    double initialization_time;

    /** Boolean indicating if the Aggregator still has activities to perform */
    bool still_has_activities = true;

    /**
     * Launch one aggregation step in parallel, sharing activities among the Host's cores.
     */
    void launch_one_aggregation(); 

    /** 
     * Launch the aggregating activity or test if the current one has finished.
     * @return true when aggregation is finished, otherwise false.
     */
    bool aggregate();

    /** 
     * Sends the global model with a broadcast. It should sent it to every connected nodes of our cluster.
     */
    void send_global_model();

    /**
     * Sends kills request with a broadcast. Used at the end of the simulation to terminate the connected nodes.
     */
    void send_kills();

    /**
     * Print a report with some number of the process
     */
    void print_end_report();

    /**
     * Checks if the training phase should stop
     */
    bool check_end_condition();
public:
    Aggregator(node_name name);
    virtual ~Aggregator() {} 

    NodeRole get_role_type() { return NodeRole::Aggregator; };

    /* --- Functions to be implemented by the children classes --- */
    virtual void run() = 0;
    /* ----------------------------------------------------------- */
};

#endif // !FALAFELS_AGGREGATOR_HPP
