/* Aggregator */
#ifndef FALAFELS_EQUALIZER_AGGREGATOR_HPP
#define FALAFELS_EQUALIZER_AGGREGATOR_HPP

#include "aggregator.hpp"

/**
 * Aggregator that send variable workloads to the trainers depending on their response time.
 * Slow clients will receive training request with more number of local epochs.
 * /!\ if the client is slow because of network connection it will be useless... We should take that into account.
 * We may ping-pong to estimate ms in order to verify that the speed issue isn't caused by the connection.
 */
class EqualizerAggregator : public Aggregator
{
private:
    uint32_t total_number_clients;
    void send_global_model_to_available_trainers();
    uint64_t wait_local_models();
    void send_kills();
public:
    EqualizerAggregator(float proportion_threshold);
    void run();
};

#endif // !FALAFELS_EQUALIZER_AGGREGATOR_HPP
