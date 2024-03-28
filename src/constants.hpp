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

    /** Number of flops for aggregating one local model with the global one. */
    inline static double GLOBAL_MODEL_AGGREGATING_FLOPS = 1000000.0;

    /** Number of flops for training a local model. */
    inline static double LOCAL_MODEL_TRAINING_FLOPS = 1000000.0;
};

#endif // !CONSTANTS_HPP
