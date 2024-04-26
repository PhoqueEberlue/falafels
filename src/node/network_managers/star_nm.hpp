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
    std::vector<NodeInfo> *connected_nodes;
public:
    StarNetworkManager(NodeInfo);
    ~StarNetworkManager();
    uint16_t handle_registration_requests();
    void send_registration_request();

    std::unique_ptr<Packet> get_packet(const std::optional<double> &timeout=std::nullopt);
    void broadcast(std::shared_ptr<Packet>, FilterNode, const std::optional<double> &timeout=std::nullopt);
};

#endif // !FALAFELS_CENTRALIZED_NM_HPP
