use crate::bridge::ffi::NodeInfo;
use crate::moms::MOMEvent;
use crate::protocol::packet::Packet;
use crate::roles::aggregators::simple_aggregator::SimpleAggregator;
use crate::roles::{Role, RoleEvent};

#[derive(Debug, Clone)]
pub enum KindExec {
    Aggregating,
    Training,
}

pub struct TaskExec {
    pub kind: KindExec,
}

pub struct TaskSend {
    pub packet: Packet,
}

pub enum MotherboardEvent {
    /// First emitted event after motherboard initialization
    MotherboardInitialized,
    /// When an Exec Task finished
    TaskExecDone,
    /// When a Send Task have been done
    TaskSendDone,
    /// Packet have been received by the network
    PacketReceived,
}

pub enum MotherboardOrMOMEvent {
    Motherboard(MotherboardEvent),
    MOM(MOMEvent),
}

pub enum MotherbroadOrRoleEvent {
    Role(RoleEvent),
    MOM(MOMEvent),
}

pub struct Motherboard {
    my_node_info: NodeInfo,
    role: Box<dyn Role>,
}

pub fn new_motherboard(node_info: NodeInfo) -> Box<Motherboard> {
    Box::new(Motherboard {
        my_node_info: node_info,
        role: Box::new(SimpleAggregator::new()),
    })
}

// impl Motherboard {
//     pub fn put_event(&mut self, event_wrapper: EventWrapper) -> TaskWrapper {
//         self.role.put_event(MotherboardOrMOMEvent::Motherboard(event_wrapper.event));
//         unimplemented!()
//     }
// }
