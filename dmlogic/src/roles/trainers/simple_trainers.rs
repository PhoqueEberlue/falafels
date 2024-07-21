use crate::{
    moms::MOMEvent,
    motherboard::{MotherboardEvent, MotherboardOrMOMEvent, MotherboardTask},
    protocol::operations::Operation,
    roles::{Role, RoleEvent},
};

use super::TrainerBase;

// State enum for the SimpleTrainer, do not expose
#[derive(Debug)]
enum State {
    WaitingGlobalModels,
    Training,
}

pub struct SimpleTrainer {
    // Contains common fields and methods for all trainers
    base: TrainerBase,
    state: State,
    tasks: Vec<MotherboardTask>,
}

impl SimpleTrainer {
    pub fn new() -> SimpleTrainer {
        SimpleTrainer {
            base: TrainerBase::new(),
            state: State::WaitingGlobalModels,
            tasks: Vec::new(),
        }
    }

    fn waiting_global_model(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match event {
            MotherboardOrMOMEvent::MOM(MOMEvent::OperationReceived(
                Operation::SendGlobalModel {
                    number_local_epochs,
                },
            )) => {
                self.base.number_local_epochs = number_local_epochs;
                self.state = State::Training;
                // Add training task
                self.tasks.push(self.base.create_training_task());
            }
            _ => {}
        }

        None
    }

    fn training(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match event {
            MotherboardOrMOMEvent::Motherboard(MotherboardEvent::TaskExecDone) => {
                self.state = State::WaitingGlobalModels;
                return Some(self.base.create_send_local_model_event());
            }
            _ => None,
        }
    }

    fn run_one_step(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match self.state {
            State::WaitingGlobalModels => self.waiting_global_model(event),
            State::Training => self.training(event),
        }
    }
}

impl Role for SimpleTrainer {
    fn put_event(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        self.run_one_step(event)
    }

    fn pop_task(&mut self) -> Option<MotherboardTask> {
        self.tasks.pop()
    }
}

#[cfg(test)]
mod tests {
    use crate::{motherboard::KindExec, protocol::filters::NodeFilter};

    use super::*;

    #[test]
    fn test_typical_workflow() {
        let mut trainer = SimpleTrainer::new();

        // Sending a SendGlobalModel op with 3 locals epochs to perform
        trainer.run_one_step(MotherboardOrMOMEvent::MOM(MOMEvent::OperationReceived(
            Operation::SendGlobalModel {
                number_local_epochs: 3,
            },
        )));

        assert!(matches!(trainer.state, State::Training));

        let task = trainer.pop_task();

        assert!(matches!(
            task,
            Some(MotherboardTask::Exec(KindExec::Training))
        ));

        let event = trainer.run_one_step(MotherboardOrMOMEvent::Motherboard(
            MotherboardEvent::TaskExecDone,
        ));

        assert!(matches!(trainer.state, State::WaitingGlobalModels));

        // The last step should have generated a TBSP event with operation containing the
        // information that the trainer did 3 local epochs.
        assert!(matches!(
            event,
            Some(RoleEvent::ToBeSentPacket {
                filter: NodeFilter::Aggregators,
                op: Operation::SendLocalModel {
                    number_local_epochs_done: 3
                }
            })
        ));
    }
}
