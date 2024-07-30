pub mod star_mom;

use crate::motherboard::{MotherboardTask, MotherbroadOrRoleEvent};
use crate::protocol::operations::Operation;
use crate::protocol::packet::Packet;
use crate::protocol::NodeInfo;

pub enum MOMEvent {
    OperationReceived(Operation),
    NodeConnected,
    // Contains the number of connected clients
    ClusterConnected(u64),
}

pub trait MOM {
    fn put_event(&mut self, mb_event: MotherbroadOrRoleEvent) -> Option<MOMEvent>;

    // To be used by the Motherboard to get execution tasks of the implemented
    fn pop_task(&mut self) -> Option<MotherboardTask>;
}



pub struct MOMBase {
    my_node_info: NodeInfo,
    bootstrap_nodes: Vec<NodeInfo>
}

impl MOMBase {
    pub fn new(my_node_info: NodeInfo, bootstrap_nodes: Vec<NodeInfo>) -> MOMBase {
        MOMBase { my_node_info, bootstrap_nodes }
    }

    /// Checks if the packet is targeted to our node's role and return an OperationReceived event
    /// if its the case, otherwise None.
    pub fn if_target_get_event(&self, packet: Packet) -> Option<MOMEvent> {
        if packet.target_filter.unwrap().filter(&self.my_node_info) {
            Some(MOMEvent::OperationReceived(packet.op))
        } else {
            None
        }
    }
}


