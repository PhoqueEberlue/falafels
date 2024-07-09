#include "./i_mediator_producer.hpp"
#include "./i_network_manager.hpp"
#include "./i_common.hpp"

class MOM
{
public:
    MOM(protocol::NodeInfo node_info) : my_node_info(node_info)
    {
        this->registration_requests = new std::vector<protocol::operations::RegistrationRequest>();
    }

    ~MOM()
    {
        delete this->bootstrap_nodes;
        delete this->registration_requests;
    }

    /** Run the main execution function of the NetworkManager */
    virtual void run() = 0;

    void set_mediator_producer(std::unique_ptr<IMediatorProducer> mp) { this->mp = std::move(mp); };

    std::unique_ptr<IMediatorProducer> mp;

    std::unique_ptr<INetworkManager> nm;

    std::unique_ptr<ICommon> common;

    /** Vector of nodes to connect to when launching the NetworkManager. Used by Trainers that knows in advance 
        one of the aggregators */
    std::vector<protocol::NodeInfo> *bootstrap_nodes;

    /** NodeInfo of the Node controlling the NetworkManager */
    protocol::NodeInfo my_node_info;

    /** Save of the registration requests to be used during the WAITING_REGISTRATION_REQUEST state */
    std::vector<protocol::operations::RegistrationRequest> *registration_requests;

    /** TODO: currently only use by aggregator: this is a bad habit */
    std::optional<double> start_time;

    void if_target_put_op(std::unique_ptr<protocol::Packet> p);

private:
    /** Handle the registration regquests by creating the network links and sending confirmations to the connected nodes */
    virtual void handle_registration_requests() = 0;

    /** Send a registration request to one of our bootstrap node */
    virtual void send_registration_request() = 0;

    /** Handle a registration confirmation by connecting to the received list of node */
    virtual void handle_registration_confirmation(const protocol::operations::RegistrationConfirmation &confirmation) = 0;

    /** Broadcast a packet to the node of our cluster matching the filter contained in the packet */
    virtual void broadcast(const std::unique_ptr<protocol::Packet> &p, bool is_redirected=false) = 0;

    /** This function decides wether it should give the packet to the (local) Role (by sending it through received_packets queue)
        and/or if it should redirect it to another Node. */
    // virtual void route_packet(std::unique_ptr<protocol::Packet> p) = 0;

    virtual void handle_kill_phase() = 0;
};
