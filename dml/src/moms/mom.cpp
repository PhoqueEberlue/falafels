#include "../../include/mom.hpp"

XBT_LOG_NEW_DEFAULT_CATEGORY(s4u_mom, "Messages specific for this example");

using namespace std;
using namespace protocol;

void MOM::if_target_put_op(unique_ptr<Packet> p)
{
    // Check if the packet is targeted to our node's role
    if ((*p->target_filter)(&this->my_node_info))
    {
        // If so, put the packet's operation
        this->mp->put_received_operation(p->op);
    }
}
