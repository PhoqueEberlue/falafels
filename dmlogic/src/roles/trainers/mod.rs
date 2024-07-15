use crate::{motherboard::{KindExec, TaskExec}, protocol::{filters::NodeFilter, operations::Operation}};

use super::RoleEvent;

mod simple_trainers;

// Struct representing the Trainer
struct TrainerBase {
    number_local_epochs: u64,
}

impl TrainerBase {
    // Constructor for Trainer
    pub fn new() -> Self {
        TrainerBase {
            number_local_epochs: 0,
        }
    }

    // Method to run and wait for the training activities in parallel
    pub fn create_training_task(&self) -> TaskExec {
        TaskExec { kind: KindExec::Training }
    }

    // Method to send the local model to aggregator(s)
    pub fn create_send_local_model_event(&self) -> RoleEvent {
        RoleEvent::ToBeSentPacket {
            filter: NodeFilter::Aggregators,
            op: Operation::SendLocalModel {
                number_local_epochs_done: self.number_local_epochs 
            }
        }
    }
}
