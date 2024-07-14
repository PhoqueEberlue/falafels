pub mod aggregators;
pub mod trainers;

use crate::bridge::ffi::TaskExec;
use crate::motherboard::MotherboardOrMOMEvent;
use crate::protocol::filters::NodeFilter;
use crate::protocol::operations::Operation;

pub trait Role {
    // fn get_role_type(&self) -> RoleEnum;
    fn put_event(&mut self, mb_event: MotherboardOrMOMEvent) -> Option<RoleEvent>;

    // To be used by the implemented to add execution tasks
    fn add_task(&mut self, task: TaskExec);

    // To be used by the Motherboard to get execution tasks of the implemented
    fn pop_task(&mut self) -> Option<TaskExec>;
}

pub enum RoleEvent {
    ToBeSentPacket(ToBeSentPacket)
}

pub struct ToBeSentPacket {
    filter: NodeFilter,
    op: Operation,
}
