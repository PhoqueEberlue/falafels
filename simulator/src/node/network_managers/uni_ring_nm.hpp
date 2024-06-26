/* Unidirectional Ring Network Manager */
#ifndef FALAFELS_UNI_RING_NM_HPP
#define FALAFELS_UNI_RING_NM_HPP

#include "nm.hpp"
#include <cstdint>
#include <memory>
#include <simgrid/forward.h>
#include <vector>

class UniRingNetworkManager : public NetworkManager
{
private:
    /** Left neighbour of the current node */
    protocol::NodeInfo left_node;
    
    bool cluster_connected_have_been_sent = false;

    /** Redirect a packet to a neighbour of the ring */
    void redirect(const std::unique_ptr<protocol::Packet> &p);
public:
    UniRingNetworkManager(protocol::NodeInfo);
    ~UniRingNetworkManager();

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
