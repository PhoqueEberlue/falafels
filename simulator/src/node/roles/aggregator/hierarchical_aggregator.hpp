/* Aggregator */
#ifndef FALAFELS_HIERARCHICAL_AGGREGATOR_HPP
#define FALAFELS_HIERARCHICAL_AGGREGATOR_HPP

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

    node_name central_aggregator_name;

    std::unique_ptr<StarNetworkManager> central_nm;

    void send_model_to_central_aggregator();

    void setup_central_nm();  
public:
    HierarchicalAggregator(std::unordered_map<std::string, std::string> *args, node_name name);
    ~HierarchicalAggregator() {};
    void run() override;
};

#endif // !FALAFELS_HIERARCHICAL_AGGREGATOR_HPP
