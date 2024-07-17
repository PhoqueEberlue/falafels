pub mod asynchronous_aggregator;
pub mod simple_aggregator;

use crate::{
    motherboard::{KindExec, TaskExec},
    protocol::{filters::NodeFilter, operations::Operation},
    roles::Role,
};

use super::RoleEvent;

struct AggregatorBase {
    number_local_epochs: u64,
    number_global_epochs: u64,
    total_aggregated_models: u64,
    number_client_training: u64,
    number_local_models: u64,
    total_number_local_epochs: u64,
    initialization_time: f64,
    is_main_aggregator: bool,
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
            initialization_time: 0.0, // Set actual initialization time
            is_main_aggregator: false,
        }
    }

    /// Create an aggregation task
    fn create_aggregating_task(&self) -> TaskExec {
        TaskExec {
            kind: KindExec::Aggregating,
        }
    }

    fn create_send_global_model_event(&self) -> RoleEvent {
        RoleEvent::ToBeSentPacket {
            filter: NodeFilter::Trainers,
            op: Operation::SendGlobalModel {
                number_local_epochs: self.number_local_epochs,
            },
        }
    }

    fn send_kills(&self) -> RoleEvent {
        // TODO: Add unit test for that
        RoleEvent::ToBeSentPacket {
            filter: NodeFilter::Everyone,
            op: Operation::Kill,
        }
    }

    fn print_end_report(&self) {
        unimplemented!()
    }

    fn check_end_condition(&self) -> bool {
        // TODO: make it an argument for the aggregator
        // Support multiple end_conditions
        // TODO: Add unit test for that
        return self.total_number_local_epochs >= 1000;
    }
}

// Is this needed?
// pub trait AggregatorTrait: Role {
//
// }
