/* Decentralized Network Manager */
#ifndef FALAFELS_DECENTRALIZED_NM_HPP
#define FALAFELS_DECENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>

class RingNetworkManager : public NetworkManager
{
private:
    NodeInfo *left_node;
    NodeInfo *right_node;
    ~RingNetworkManager();
public:
    RingNetworkManager(node_name);
    uint16_t broadcast(Packet* packet, FilterNode filter);
    uint16_t broadcast_timeout(Packet *packet, FilterNode filter, uint64_t timeout);
    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
};

#endif // !FALAFELS_DECENTRALIZED_NM_HPP
