use std::collections::VecDeque;

use crate::moms::star_mom::StarMom;
use crate::moms::{MOMEvent, MOM};
use crate::protocol::packet::Packet;
use crate::protocol::NodeInfo;
use crate::roles::aggregators::simple_aggregator::SimpleAggregator;
use crate::roles::{Role, RoleEvent};

#[derive(Debug, Clone)]
pub enum KindExec {
    Aggregating,
    Training,
}

pub enum MotherboardTask {
    Exec(KindExec),
    Send(Packet), 
    // Ask the motherboard to send a timeout event in x seconds
    Timeout(f64),
}



pub enum MotherboardEvent {
    /// First emitted event after motherboard initialization
    MotherboardInitialized,
    /// When an Exec Task finished
    TaskExecDone,
    /// When a Send Task have been done
    TaskSendDone,
    TaskTimeoutDone,
    /// Packet have been received by the network
    PacketReceived(Packet)
}

pub enum MotherboardOrMOMEvent {
    Motherboard(MotherboardEvent),
    MOM(MOMEvent),
}

pub enum MotherbroadOrRoleEvent {
    Motherboard(MotherboardEvent),
    Role(RoleEvent),
}

pub struct Motherboard {
    my_node_info: NodeInfo,
    role: Box<dyn Role>,
    mom: Box<dyn MOM>,
    role_event_queue: VecDeque<RoleEvent>,
    mom_event_queue: VecDeque<MOMEvent>,
}

impl Motherboard {
    pub fn new(node_info: NodeInfo) -> Motherboard {
        Motherboard {
            my_node_info: node_info,
            role: Box::new(SimpleAggregator::new()),
            // TODO: add bootstrap nodes
            mom: Box::new(StarMom::new(node_info, vec![])),
            role_event_queue: VecDeque::new(),
            mom_event_queue: VecDeque::new(),
        }
    }

    pub fn put_event(&mut self, event: MotherboardEvent) -> Option<MotherboardTask> {
        self.role.put_event();
        unimplemented!()
    }
}
