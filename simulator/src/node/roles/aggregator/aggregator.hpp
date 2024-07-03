/* Aggregator */
#ifndef FALAFELS_AGGREGATOR_HPP
#define FALAFELS_AGGREGATOR_HPP

#include "../role.hpp"
#include <cstdint>
#include <simgrid/forward.h>
#include <simgrid/s4u/Exec.hpp>
#include <simgrid/s4u/ActivitySet.hpp>

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
    uint16_t number_client_training = 65535; // Set it to max value until we get the actual one

    /** Number of the local models collected at a moment in time */
    uint64_t number_local_models = 0;    

    uint64_t total_number_local_epochs = 0;

    /** Simgrid activity representing the training */
    simgrid::s4u::ActivitySet *aggregating_activities;

    /** Time when the aggregator has been initialized */
    double initialization_time;

    bool is_main_aggregator = false;

    /**
     * Run and wait all aggregations steps in parallel, sharing activities among the Host's cores.
     * Groups all tasks into a big one, which is way more efficient (in terms of simulation runtime) 
     * than executing tasks one by one.
     */
    void aggregate();

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
    Aggregator(protocol::node_name name);
    virtual ~Aggregator() { delete this->aggregating_activities; } 

    protocol::NodeRole get_role_type()
    {
        return this->is_main_aggregator ? protocol::NodeRole::MainAggregator : protocol::NodeRole::Aggregator; 
    };

    /* --- Functions to be implemented by the children classes --- */
    virtual void run() = 0;
    /* ----------------------------------------------------------- */
};

#endif // !FALAFELS_AGGREGATOR_HPP
