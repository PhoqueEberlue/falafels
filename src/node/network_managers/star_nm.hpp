/* Centralized Network Manager */
#ifndef FALAFELS_CENTRALIZED_NM_HPP
#define FALAFELS_CENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <memory>
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

    std::unique_ptr<Packet> get_packet(const std::optional<double> &timeout=std::nullopt);
    void broadcast(std::shared_ptr<Packet>, FilterNode, const std::optional<double> &timeout=std::nullopt);

    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
    // void wait_last_comms() {}
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
