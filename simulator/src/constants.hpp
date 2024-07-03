#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

/**
 * Class storing global constants that can are used in the whole program.
 * The class cannot be instanciated and only expose static fields.
 * The values in each constant are defined by default but can be changed at runtime from the config loader.
 */
class Constants
{
public:
    /** Simulated model size that will be sent through the network as a packet. */
    inline static uint64_t MODEL_SIZE_BYTES = 2000;

    /** Number of flops for aggregating ONE local model with the global one. */
    inline static double GLOBAL_MODEL_AGGREGATING_FLOPS = 1000000.0;

    /** Number of flops for training a local model. */
    inline static double LOCAL_MODEL_TRAINING_FLOPS = 1000000.0;

    /** Timeout for the registration phase */
    inline static double REGISTRATION_TIMEOUT = 4.0;

    /** Wether or not we should generate graph of the communications */ 
    inline static bool GENERATE_DOT_FILES = false;
    /* -------------------------- SIMULATION ENDING CONDITIONS -------------------------- */
    /*                        Exactly one condition should be defined                     */

    /** Duration in seconds for the training phase. 0.0 when the feature isn't used */
    inline static double END_CONDITION_DURATION_TRAINING_PHASE = 0.0;

    /** Number of rounds (number of aggregation phase) before the simulation ends. 0 when the feature isn't used */
    inline static uint64_t END_CONDITION_NUMBER_ROUNDS = 0;

    /** Total number of local epochs before the simulation ends. 0 when the feature isn't used */
    inline static uint64_t END_CONDITION_TOTAL_NUMBER_LOCAL_EPOCHS = 0;
    /* ---------------------------------------------------------------------------------- */
};

#endif // !CONSTANTS_HPP
