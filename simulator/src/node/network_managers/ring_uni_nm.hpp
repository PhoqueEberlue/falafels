/* Unidirectional Ring Network Manager */
#ifndef FALAFELS_UNI_RING_NM_HPP
#define FALAFELS_UNI_RING_NM_HPP

#include "nm.hpp"
#include <memory>
#include <simgrid/forward.h>

class RingUniNetworkManager : public NetworkManager
{
private:
    /** Left neighbour of the current node */
    protocol::NodeInfo left_node;
    
    bool cluster_connected_have_been_sent = false;

    /** Redirect a packet to a neighbour of the ring */
    void redirect(const std::unique_ptr<protocol::Packet> &p);
public:
    RingUniNetworkManager(protocol::NodeInfo);
    ~RingUniNetworkManager();

    // See nm.hpp for documentation
    void run();
    void handle_registration_requests();
    void send_registration_request();
    void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation);
    void send_to_neighbour(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false);
    void broadcast(const std::unique_ptr<protocol::Packet> &packet, bool is_redirected=false) {}
    void handle_kill_phase();
};


#endif // !FALAFELS_UNI_RING_NM_HPP
