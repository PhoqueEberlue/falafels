/* Decentralized Network Manager */
#ifndef FALAFELS_DECENTRALIZED_NM_HPP
#define FALAFELS_DECENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <memory>
#include <simgrid/forward.h>
#include <vector>

class RingNetworkManager : public NetworkManager
{
private:
    NodeInfo left_node;
    NodeInfo right_node;
    std::vector<packet_id> *received_packets;

    void redirect(std::unique_ptr<Packet>&);

    bool is_duplicated(std::unique_ptr<Packet>&);
public:
    RingNetworkManager(NodeInfo);
    ~RingNetworkManager();
    uint16_t handle_registration_requests();
    void send_registration_request();

    std::unique_ptr<Packet> get_packet(const std::optional<double> &timeout=std::nullopt);
    void broadcast(std::shared_ptr<Packet>, FilterNode, const std::optional<double> &timeout=std::nullopt);

    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
};

#endif // !FALAFELS_DECENTRALIZED_NM_HPP
