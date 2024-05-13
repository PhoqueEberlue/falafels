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
    /** Left neighbour of the current node */
    NodeInfo left_node;
    
    /** Right neighbour of the current node */
    NodeInfo right_node;

    /** Saving the list of received packet ids to be able to discard duplicates */
    std::vector<packet_id> *received_packet_ids;

    /** Return wether the given packet_id have been already received */
    bool is_duplicated(const packet_id);
    
    /** Redirect a packet to a neighbour of the ring */
    void redirect(std::unique_ptr<Packet>&);
public:
    RingNetworkManager(NodeInfo);
    ~RingNetworkManager();

    // See nm.hpp for documentation
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const Packet::RegistrationConfirmation &confirmation);
    void route_packet(std::unique_ptr<Packet> packet);
    void broadcast(std::shared_ptr<Packet>);
};

#endif // !FALAFELS_DECENTRALIZED_NM_HPP
