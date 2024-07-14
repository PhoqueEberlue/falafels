use super::AggregatorBase;
use crate::bridge::ffi::{EventEndExec, TaskExec};
use crate::moms::MOMEvent;
use crate::motherboard::{MotherboardEvent, MotherboardOrMOMEvent};
use crate::protocol::operations::Operation;
use crate::roles::{Role, RoleEvent};

// State enum for the SimpleAggregator, do not expose
enum State {
    Initializing,
    WaitingLocalModels,
    Aggregating,
}

pub struct SimpleAggregator {
    // Contains common fields and methods for all aggregators
    base: AggregatorBase,
    state: State,
    tasks: Vec<TaskExec>,
}

impl SimpleAggregator {
    pub fn new() -> SimpleAggregator {
        SimpleAggregator { 
            base: AggregatorBase::new(), 
            state: State::Initializing,
            tasks: Vec::new(),
        }
    }

    fn initializing(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match event {
            // At initialization, send the first global model
            MotherboardOrMOMEvent::Motherboard(
                MotherboardEvent::MotherboardInitialized()) => {
                return Some(self.base.create_send_global_model_event());
            },
            // Case where we receive ClusterConnected event 
            MotherboardOrMOMEvent::MOM(
                MOMEvent::ClusterConnected(nb_clients_connected)) => {
                self.base.number_client_training = nb_clients_connected;
                self.state = State::WaitingLocalModels;
            },
            _ => {}, // Ignore all other events
        }

        None
    }

    fn waiting_local_models(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match event {
            MotherboardOrMOMEvent::MOM(
                MOMEvent::OperationReceived(
                    Operation::SendLocalModel(op_send_global))) => {

                self.base.number_local_models += 1;
                self.base.total_number_local_epochs += op_send_global.number_local_epochs_done;

                if self.base.number_local_models >= self.base.number_client_training {
                    self.state = State::Aggregating;
                }
            },
            _ => {}, // Ignore all other events
        }

        None
    }

    fn aggregating(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        // If not already sent, send execution task
        if !self.base.aggregating_task_sent {
            self.add_task(
                self.base.create_aggregation_task()
            );
            return None;
        }

        // TODO: Do we still need MainAggregator???? It's pretty annoying ngl
        // Only check end condition as MainAggregator
        // if (this->get_role_type() == NodeRole::MainAggregator 
        //     && this->check_end_condition())
        // {
        //
        // Only check End condition or send global model when the aggregation task finished
        match event {
            MotherboardOrMOMEvent::Motherboard(
                MotherboardEvent::EndExec(event)) => {
                if self.base.check_end_condition() {
                    self.base.print_end_report();
                    self.base.send_kills();
                } else {
                    self.base.number_local_models = 0;
                    self.state = State::WaitingLocalModels;
                    return Some(self.base.create_send_global_model_event());
                }
            },
            _ => {}, // Ignore all other events
        }
        
        None
    }

    fn run_one_step(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match self.state {
            State::Initializing => { self.initializing(event) },
            State::WaitingLocalModels => { self.waiting_local_models(event) },
            State::Aggregating => { self.aggregating(event) },
        }
    }
}


impl Role for SimpleAggregator {
    // fn get_role_type(&self) -> RoleEnum {
    //     self.base.get_role_type()
    // }

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

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_initializing() {
        let mut sa = SimpleAggregator::new();
        let res = sa.put_event(MotherboardOrMOMEvent::Motherboard(MotherboardEvent::MotherboardInitialized()));

        // Verify that after MotherboardInitialized the aggregator sends a ToBeSentPacket event
        assert!(matches!(res.unwrap(), RoleEvent::ToBeSentPacket(_)));

        // It should be still in initializing mode
        assert!(matches!(sa.state, State::Initializing));

        sa.put_event(MotherboardOrMOMEvent::MOM(MOMEvent::ClusterConnected(12)));

        // Should have 12 training clients and in WaitingLocalModels state
        assert!(sa.base.number_client_training == 12);
        assert!(matches!(sa.state, State::WaitingLocalModels));
    }
}
