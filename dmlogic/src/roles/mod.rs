pub mod aggregators;
pub mod trainers;

use crate::motherboard::{MotherboardOrMOMEvent, MotherboardTask};
use crate::protocol::filters::NodeFilter;
use crate::protocol::operations::Operation;

pub trait Role {
    // fn get_role_type(&self) -> RoleEnum;
    fn put_event(&mut self, mb_event: MotherboardOrMOMEvent) -> Option<RoleEvent>;

    // To be used by the Motherboard to get execution tasks of the implemented
    fn pop_task(&mut self) -> Option<MotherboardTask>;
}

pub enum RoleEvent {
    ToBeSentPacket { filter: NodeFilter, op: Operation },
}
