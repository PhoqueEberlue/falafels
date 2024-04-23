/* Decentralized Network Manager */
#ifndef FALAFELS_DECENTRALIZED_NM_HPP
#define FALAFELS_DECENTRALIZED_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <memory>
#include <simgrid/forward.h>
#include <simgrid/s4u/ActivitySet.hpp>
#include <vector>

class RingNetworkManager : public NetworkManager
{
private:
    NodeInfo left_node;
    NodeInfo right_node;
    simgrid::s4u::ActivitySetPtr pending_comms;
    std::vector<packet_id> received_packets;
    ~RingNetworkManager();

    void redirect(std::unique_ptr<Packet>&);
public:
    RingNetworkManager(NodeInfo);
    uint16_t handle_registration_requests();
    void send_registration_request();

    std::unique_ptr<Packet> get_packet(const std::optional<double> &timeout=std::nullopt);
    void broadcast(std::shared_ptr<Packet>, FilterNode, const std::optional<double> &timeout=std::nullopt);

    void set_bootstrap_nodes(std::vector<NodeInfo*> *nodes);
    // void wait_last_comms();
};

#endif // !FALAFELS_DECENTRALIZED_NM_HPP
