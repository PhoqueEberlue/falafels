use crate::{moms::MOMEvent, motherboard::{MotherboardEvent, MotherboardOrMOMEvent, TaskExec}, protocol::operations::Operation, roles::{Role, RoleEvent}};

use super::TrainerBase;

// State enum for the SimpleTrainer, do not expose
enum State {
    WaitingGlobalModels,
    Training,
}

pub struct SimpleTrainer {
    // Contains common fields and methods for all trainers 
    base: TrainerBase,
    state: State,
    tasks: Vec<TaskExec>,
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
            MotherboardOrMOMEvent::MOM(
                MOMEvent::OperationReceived(
                    Operation::SendGlobalModel { number_local_epochs })) => {

                self.base.number_local_epochs = number_local_epochs;
                self.state = State::Training;
                // Add training task
                self.add_task(self.base.create_training_task());
            },
            _ => {}
        }

        None
    }

    fn training(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match event {
            MotherboardOrMOMEvent::Motherboard(
                MotherboardEvent::TaskExecDone) => {
                self.state = State::WaitingGlobalModels;
                return Some(self.base.create_send_local_model_event());
            },
            _ => { None },
        }
    }

    fn run_one_step(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match self.state {
            State::WaitingGlobalModels => { self.waiting_global_model(event) },
            State::Training => { self.training(event) },
        }
    }
}

impl Role for SimpleTrainer {
    fn put_event(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        self.run_one_step(event)
    }

    fn pop_task(&mut self) -> Option<TaskExec> {
        self.tasks.pop()
    }
    
    fn add_task(&mut self, task: TaskExec) {
        self.tasks.push(task);
    }
}

