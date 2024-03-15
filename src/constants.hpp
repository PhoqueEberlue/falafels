#include <cstdint>

namespace constants
{
    const uint64_t MODEL_SIZE_BYTES = 20000;

    namespace aggregator
    {
        const uint64_t AGGREGATION_FLOPS = 10000000;
    } 

    namespace trainer 
    {
        const uint64_t LOCAL_EPOCH_FLOPS = 10000000;
    }
}
