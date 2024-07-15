use crate::protocol::operations::Operation;
use crate::motherboard::{MotherboardOrMOMEvent, TaskSend};

pub enum MOMEvent {
    OperationReceived(Operation),
    NodeConnected(),
    // Contains the number of connected clients
    ClusterConnected(u64),
}

pub trait MOM {
    // fn get_role_type(&self) -> RoleEnum;
    fn put_event(&self, mb_event: MotherboardOrMOMEvent) -> Option<MOMEvent>;

    // To be used by the implemented to add execution tasks
    fn add_task(&mut self, task: TaskSend);

    // To be used by the Motherboard to get execution tasks of the implemented
    fn pop_task(&mut self) -> Option<TaskSend>;
}
