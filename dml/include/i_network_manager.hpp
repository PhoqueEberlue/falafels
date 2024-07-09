/* Network Manager */
#ifndef DML_NETWORK_MANAGER_HPP
#define DML_NETWORK_MANAGER_HPP

#include <memory>
#include <optional>
#include <vector>
#include "./protocol.hpp"
#include "./i_mediator_producer.hpp"

class INetworkManager 
{
protected: 
    /** Vector of nodes to connect to when launching the NetworkManager. Used by Trainers that knows in advance 
        one of the aggregators */
    std::vector<protocol::NodeInfo> *bootstrap_nodes;

    /** NodeInfo of the Node controlling the NetworkManager */
    protocol::NodeInfo my_node_info;

    /** Save of the registration requests to be used during the WAITING_REGISTRATION_REQUEST state */
    std::vector<protocol::operations::RegistrationRequest> *registration_requests;

    /** Save of the start time of the aggregator */
    std::optional<double> start_time;

public:  
    INetworkManager() {};
    virtual ~INetworkManager();

    /** Get NetworkManager's NodeInfo */
    protocol::NodeInfo get_my_node_info() { return this->my_node_info; }

    /** Utility to get my node name quicker */
    protocol::node_name get_my_node_name() { return this->my_node_info.name; }

    /** Set bootstrap_nodes */
    void set_bootstrap_nodes(std::vector<protocol::NodeInfo> *nodes);
 
    /** Classic send from the current node to another one. If is_redirected is set to true, the original source wont be overwritten */
    void send_async(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false);

    void kill_role_actor();

    /** Blocking get a Packet from the Network */
    std::unique_ptr<protocol::Packet> get(const std::optional<double> timeout=std::nullopt);

    /** Async get a Packet from the Network */
    AsyncWrapper get_async();

    void init_run_activities();

    void clear_async_puts();

    void if_target_put_op(std::unique_ptr<protocol::Packet> p);
};

#endif // !DML_NETWORK_MANAGER_HPP
