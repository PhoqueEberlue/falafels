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
    inline static double REGISTRATION_TIMEOUT = 2.0;

    /* -------------------------- SIMULATION ENDING CONDITIONS -------------------------- */
    /*                        Exactly one condition should be defined                     */

    /** Duration in seconds for the training phase. -1.0 when the feature isn't used */
    inline static double END_CONDITION_DURATION_TRAINING_PHASE = -1.0;

    /** Number of global epochs before the simulation ends. 0 when the feature isn't used */
    inline static uint64_t END_CONDITION_NUMBER_GLOBAL_EPOCHS = 10;
};

#endif // !CONSTANTS_HPP
