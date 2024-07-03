/* Aggregator */
#ifndef FALAFELS_HIERARCHICAL_AGGREGATOR_HPP
#define FALAFELS_HIERARCHICAL_AGGREGATOR_HPP

#include <cstdint>
#include <memory>

#include "aggregator.hpp"
#include "../../network_managers/star_nm.hpp"

class HierarchicalAggregator : public Aggregator 
{
private:
    using State = enum
    {
        INITIALIZING_CENTRAL,
        INITIALIZING_CLUSTER,
        WAITING_GLOBAL_MODEL,
        WAITING_LOCAL_MODELS,
        AGGREGATING,
    };

    /** State of the Aggregator */
    State state = INITIALIZING_CENTRAL;

    std::unique_ptr<MediatorConsumer> central_mc;

    protocol::node_name central_aggregator_name;

    /** Current number of local epochs performed by our cluster */
    uint64_t current_number_local_epochs_cluster;

    bool first_global_model = true;

    void send_model_to_central_aggregator();

    void setup_central_nm();  
public:
    HierarchicalAggregator(std::unordered_map<std::string, std::string> *args, protocol::node_name name);
    ~HierarchicalAggregator() {};
    void run() override;
};

#endif // !FALAFELS_HIERARCHICAL_AGGREGATOR_HPP
