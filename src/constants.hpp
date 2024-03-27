#ifndef CONSTANTS_HPP
#define CONSTANTS_HPP

#include <cstdint>

class Constants
{
public:
    inline static uint64_t MODEL_SIZE_BYTES = 2000;
    inline static double GLOBAL_MODEL_AGGREGATING_FLOPS = 1000000.0;
    inline static double LOCAL_MODEL_TRAINING_FLOPS = 1000000.0;
};

#endif // !CONSTANTS_HPP
