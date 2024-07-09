#include "./s_network_manager.hpp"
#include <boost/smart_ptr/intrusive_ptr.hpp>
#include <format>
#include <memory>
#include <optional>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <simgrid/s4u/Activity.hpp>
#include <simgrid/s4u/Actor.hpp>
#include <simgrid/s4u/Engine.hpp>
#include <variant>
#include <vector>
#include <xbt/log.h>
#include <xbt/asserts.h>
#include <xbt/ex.h>
#include <simgrid/s4u/ActivitySet.hpp>

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_s_network_manager, "Messages specific for this example");

using namespace std;
using namespace protocol;

SNetworkManager::SNetworkManager(NodeInfo node_info) : my_node_info(node_info)
{
    this->pending_async_put = new simgrid::s4u::ActivitySet();
    this->pending_comm_and_mess_get = new simgrid::s4u::ActivitySet(); 

    // Initializing our mailbox
    this->mailbox = simgrid::s4u::Mailbox::by_name(this->my_node_info.name);

    this->registration_requests = new vector<operations::RegistrationRequest>();
}

SNetworkManager::~SNetworkManager()
{
    delete this->bootstrap_nodes;
    delete this->pending_async_put;
    delete this->pending_comm_and_mess_get;
    delete this->registration_requests;
}

void SNetworkManager::set_bootstrap_nodes(vector<NodeInfo> *nodes)
{
    this->bootstrap_nodes = nodes;
}

unique_ptr<Packet> SNetworkManager::get(const optional<double> timeout)
{
    unique_ptr<Packet> p;
 
    if (timeout.has_value())
        try
        {
            p = this->mailbox->get_unique<Packet>(*timeout);
        }
        catch (simgrid::TimeoutException)
        {
            throw protocol::exceptions::TimeoutException("");
        }
    else
        p = this->mailbox->get_unique<Packet>();

    XBT_INFO("%s <--%s(%lu)--- %s", p->dst.c_str(), p->get_op_name(), p->id, p->src.c_str());
    return p;
}

simgrid::s4u::CommPtr SNetworkManager::get_async()
{
    return this->mailbox->get_async();
}

void SNetworkManager::send_async(const std::unique_ptr<Packet> &p, bool is_redirected)
{
    auto p_clone = p->clone();
    p_clone->src = this->get_my_node_name();
    p_clone->dst = p->dst;

    // Only write original source when sending packets created by the current node.
    if (!is_redirected)
    {
        p_clone->original_src = this->get_my_node_name();
        XBT_INFO("%s ---%s(%lu)--> %s", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
    }
    else
    {
        p_clone->original_src = p->original_src;
        XBT_INFO("%s ---%s(%lu)--> %s [REDIRECT]", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
    }

    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(p_clone->dst);

    auto comm = receiver_mailbox->put_async(p_clone, p_clone->get_packet_size());

    comm->set_name(p_clone->dst);
    
    this->pending_async_put->push(comm);
}

void SNetworkManager::init_run_activities()
{
    // Initialize the first waiting activities: this should be done one time before going into RUNNING state
    // Add Comm aysnc get
    this->pending_comm_and_mess_get->push(this->get_async());

    // Add Mess async get
    this->pending_comm_and_mess_get->push(this->mp->get_async_to_be_sent_packet());
}

void SNetworkManager::clear_async_puts()
{
    // Cleanly detach each async put
    for (int i = 0; i < this->pending_async_put->size(); i++)
        this->pending_async_put->at(i).detach();

    // Clear all async put before sending and waiting the kill packet
    this->pending_async_put->clear();
}
