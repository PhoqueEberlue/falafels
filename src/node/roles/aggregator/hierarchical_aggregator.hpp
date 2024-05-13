/* Aggregator */
#ifndef FALAFELS_HIERARCHICAL_AGGREGATOR_HPP
#define FALAFELS_HIERARCHICAL_AGGREGATOR_HPP

#include <memory>

#include "aggregator.hpp"
#include "../../network_managers/star_nm.hpp"

class HierarchicalAggregator : public Aggregator 
{
private:
    node_name central_aggregator_name;
    std::unique_ptr<StarNetworkManager> central_nm;
    void send_model_to_central_aggregator(node_name dst);
public:
    HierarchicalAggregator(std::unordered_map<std::string, std::string> *args);
    ~HierarchicalAggregator() {};
    void run() override;
};

#endif // !FALAFELS_HIERARCHICAL_AGGREGATOR_HPP
