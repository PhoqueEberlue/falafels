use crate::bridge::ffi::{TaskExec, TaskSend, EventEndExec, EventEndSend};
use crate::bridge::{EventWrapper, TaskWrapper};
use crate::protocol::NodeInfo;

pub enum Task {
    Exec(TaskExec),
    Send(TaskSend),
    None,
}

pub enum Event {
    EndExec(EventEndExec),
    EndSend(EventEndSend),
    NotInstanciated,
}

pub struct Motherboard {
    my_node_info: NodeInfo
}

pub fn new_motherboard(node_info: NodeInfo) -> Box<Motherboard> {
    Box::new(Motherboard { my_node_info: node_info })
}

impl Motherboard {
    pub fn put_event(&mut self, event_wrapper: EventWrapper) -> TaskWrapper {

        unimplemented!()

    }


}
