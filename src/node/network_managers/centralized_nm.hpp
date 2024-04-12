/* Centralized Network Manager */
#ifndef FALAFELS_CENTRALIZED_NM_HPP
#define FALAFELS_CENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>

class CentralizedNetworkManager : public NetworkManager 
{
private:
    ~CentralizedNetworkManager();
public:
    CentralizedNetworkManager(node_name);
    uint16_t broadcast(Packet *packet, FilterNode filter);
    uint16_t broadcast_timeout(Packet *packet, FilterNode filter, uint64_t timeout);
    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
