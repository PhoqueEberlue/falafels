pub mod asynchronous_aggregator;
pub mod simple_aggregator;

use crate::{bridge::ffi::TaskExec, motherboard::MotherboardTask, roles::Role};

use super::RoleEvent;

struct AggregatorBase {
    number_local_epochs: u8,
    number_global_epochs: u16,
    total_aggregated_models: u64,
    number_client_training: u32,
    number_local_models: u32,
    total_number_local_epochs: u64,
    initialization_time: f64,
    is_main_aggregator: bool,
    aggregating_task_sent: bool,
}

impl AggregatorBase {
    fn new() -> Self {
        AggregatorBase {
            number_local_epochs: 3,
            number_global_epochs: 0,
            total_aggregated_models: 0,
            number_client_training: 65535,
            number_local_models: 0,
            total_number_local_epochs: 0,
            initialization_time: 0.0,     // Set actual initialization time
            is_main_aggregator: false,
            aggregating_task_sent: false,
        }
    }

    /// Create an aggregation task
    fn create_aggregation_task(&self) -> TaskExec {
        MotherboardTask::Exec(TaskExec { yep: 45 });
        unimplemented!()
    }

    fn create_send_global_model_event(&self) -> RoleEvent {
        unimplemented!()
    }

    fn send_kills(&self) {
        unimplemented!()
    }

    fn print_end_report(&self) {
        unimplemented!()
    }

    fn check_end_condition(&self) -> bool {
        unimplemented!()
    }
}

// Is this needed?
// pub trait AggregatorTrait: Role {
// 
// }
