#include "nm.hpp"
#include <optional>
#include <simgrid/Exception.hpp>
#include <simgrid/forward.h>
#include <xbt/log.h>

using namespace std;

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_network_manager, "Messages specific for this example");

NetworkManager::~NetworkManager()
{
    delete this->bootstrap_nodes;
}

void NetworkManager::set_bootstrap_nodes(vector<NodeInfo> *nodes)
{
    this->bootstrap_nodes = nodes;
}

void NetworkManager::send(shared_ptr<Packet> packet, node_name dst, const optional<double> &timeout)
{
    auto p_clone          = packet->clone();
    p_clone->src          = this->get_my_node_name();
    p_clone->dst          = dst;
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(p_clone->dst);

    if (timeout)
    {
        try 
        {
            XBT_INFO("%s ---%s(%lu)--> %s (timeout: %f)", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str(), *timeout);
            receiver_mailbox->put(p_clone, p_clone->get_packet_size(), *timeout);
        } 
        catch (simgrid::TimeoutException) 
        {
            XBT_INFO("Timeout, couldn't send message from %s to %s", this->get_my_node_name().c_str(), dst.c_str());
        }
    }
    else
    {
        XBT_INFO("%s ---%s(%lu)--> %s", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());
        receiver_mailbox->put(p_clone, p_clone->get_packet_size());
    }
}

simgrid::s4u::CommPtr NetworkManager::send_async(shared_ptr<Packet> packet, node_name dst)
{
    auto p_clone          = packet->clone();
    p_clone->src          = this->get_my_node_name();
    p_clone->dst          = dst;
    auto receiver_mailbox = simgrid::s4u::Mailbox::by_name(p_clone->dst);

    XBT_INFO("%s ---%s(%lu)--> %s (async)", p_clone->src.c_str(), p_clone->get_op_name(), p_clone->id, p_clone->dst.c_str());

    auto comm =  receiver_mailbox->put_async(p_clone, p_clone->get_packet_size());
    return comm;
}

unique_ptr<Packet> NetworkManager::get(const optional<double> &timeout)
{
    unique_ptr<Packet> p;

    if (timeout)
    {
        p = this->mailbox->get_unique<Packet>(*timeout);
    }
    else 
    {
        p = this->mailbox->get_unique<Packet>();
    }

    XBT_INFO("%s <--%s(%lu)--- %s", this->get_my_node_name().c_str(), p->get_op_name(), p->id, p->src.c_str());
    return p;
}
