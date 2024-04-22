/* Centralized Network Manager */
#ifndef FALAFELS_CENTRALIZED_NM_HPP
#define FALAFELS_CENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <vector>

class StarNetworkManager : public NetworkManager 
{
private:
    ~StarNetworkManager();
    std::vector<NodeInfo> *connected_nodes;
public:
    StarNetworkManager(NodeInfo);
    uint16_t handle_registration_requests();
    void send_registration_request();
    std::unique_ptr<Packet> get();
    std::unique_ptr<Packet> get(double timeout);
    uint16_t broadcast(Packet *packet, FilterNode filter);
    uint16_t broadcast(Packet *packet, FilterNode filter, uint64_t timeout);
    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
    // void wait_last_comms() {}
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
