mod star_mom;

use crate::motherboard::{MotherbroadOrRoleEvent, TaskSend};
use crate::protocol::operations::Operation;

pub enum MOMEvent {
    OperationReceived(Operation),
    NodeConnected(),
    // Contains the number of connected clients
    ClusterConnected(u64),
}

pub trait MOM {
    fn put_event(&mut self, mb_event: MotherbroadOrRoleEvent) -> Option<MOMEvent>;

    // To be used by the implemented to add execution tasks
    fn add_task(&mut self, task: TaskSend);

    // To be used by the Motherboard to get execution tasks of the implemented
    fn pop_task(&mut self) -> Option<TaskSend>;
}
