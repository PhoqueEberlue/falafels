use crate::bridge::ffi::{EventEndExec, EventReceiv, TaskExec, TaskSend};
use crate::bridge::{EventWrapper, TaskWrapper};
use crate::bridge::ffi::NodeInfo;
use crate::moms::MOMEvent;
use crate::roles::{Role, RoleEvent};
use crate::roles::aggregators::simple_aggregator::SimpleAggregator;

pub enum MotherboardTask {
    Exec(TaskExec),
    Send(TaskSend),
}

pub enum MotherboardEvent {
    MotherboardInitialized(),
    EndExec(EventEndExec),
    Receiv(EventReceiv),

    // Temp member when we initialize the enum before giving it to C++
    NotInstanciated,
}

pub enum MotherboardOrMOMEvent {
    Motherboard(MotherboardEvent),
    MOM(MOMEvent)
}

pub enum MotherbroadOrRoleEvent {
    Role(RoleEvent),
    MOM(MOMEvent)
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

impl Motherboard {
    pub fn put_event(&mut self, event_wrapper: EventWrapper) -> TaskWrapper {

        self.role.put_event(MotherboardOrMOMEvent::Motherboard(event_wrapper.event));


        unimplemented!()

    }


}
