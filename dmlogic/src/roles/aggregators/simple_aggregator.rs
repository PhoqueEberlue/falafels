use super::AggregatorBase;
use crate::moms::MOMEvent;
use crate::motherboard::{MotherboardEvent, MotherboardOrMOMEvent, MotherboardTask};
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
    tasks: Vec<MotherboardTask>,
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
            // Case where we receive ClusterConnected event
            MotherboardOrMOMEvent::MOM(MOMEvent::ClusterConnected(nb_clients_connected)) => {
                self.base.number_client_training = nb_clients_connected;
                self.state = State::WaitingLocalModels;
                return Some(self.base.create_send_global_model_event());
            }
            _ => {} // Ignore all other events
        }

        None
    }

    fn waiting_local_models(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match event {
            MotherboardOrMOMEvent::MOM(MOMEvent::OperationReceived(
                Operation::SendLocalModel {
                    number_local_epochs_done,
                },
            )) => {
                self.base.number_local_models += 1;
                self.base.total_number_local_epochs += number_local_epochs_done;

                if self.base.number_local_models >= self.base.number_client_training {
                    // Send the aggregation task and change state
                    self.tasks.push(self.base.create_aggregating_task());
                    self.state = State::Aggregating;
                }
            }
            _ => {} // Ignore all other events
        }

        None
    }

    fn aggregating(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        // TODO: Do we still need MainAggregator???? It's pretty annoying ngl
        // Only check end condition as MainAggregator
        // if (this->get_role_type() == NodeRole::MainAggregator
        //     && this->check_end_condition())
        // {
        //
        // Only check End condition or send global model when the aggregation task finished
        match event {
            MotherboardOrMOMEvent::Motherboard(MotherboardEvent::TaskExecDone) => {
                // Increment the number of aggregated models
                self.base.total_aggregated_models += self.base.number_local_models;
                // Compute the number of global epochs
                self.base.number_global_epochs =
                    self.base.total_aggregated_models / self.base.number_client_training;

                if self.base.check_end_condition() {
                    self.base.print_end_report();
                    // Return TBSP with Kill op
                    return Some(self.base.send_kills());
                } else {
                    self.base.number_local_models = 0;
                    self.state = State::WaitingLocalModels;
                    // Return TBSP with GlobalModel op
                    return Some(self.base.create_send_global_model_event());
                }
            }
            _ => {} // Ignore all other events
        }

        None
    }

    fn run_one_step(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match self.state {
            State::Initializing => self.initializing(event),
            State::WaitingLocalModels => self.waiting_local_models(event),
            State::Aggregating => self.aggregating(event),
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

    fn pop_task(&mut self) -> Option<MotherboardTask> {
        self.tasks.pop()
    }
}

#[cfg(test)]
mod tests {
    use crate::motherboard::KindExec;

    use super::*;

    #[test]
    fn test_typical_workflow() {
        let mut sa = SimpleAggregator::new();

        // It should be still in initializing mode
        assert!(matches!(sa.state, State::Initializing));

        // Pretend that 12 clients have connected
        let event = sa.put_event(MotherboardOrMOMEvent::MOM(MOMEvent::ClusterConnected(12)));

        // Should have 12 training clients and in WaitingLocalModels state
        assert!(sa.base.number_client_training == 12);
        assert!(matches!(sa.state, State::WaitingLocalModels));

        // The Aggregator sends a ToBeSentPacket event to the MOM
        assert!(matches!(event, Some(RoleEvent::ToBeSentPacket { .. })));

        for i in 0..12 {
            sa.put_event(MotherboardOrMOMEvent::MOM(MOMEvent::OperationReceived(
                Operation::SendLocalModel {
                    number_local_epochs_done: 3,
                },
            )));

            if i == 11 {
                // <- last local model
                // When all local models have been received, it should have changed its state to Aggregating
                assert!(matches!(sa.state, State::Aggregating));
                assert!(matches!(
                    sa.pop_task(),
                    Some(MotherboardTask::Exec(KindExec::Aggregating))
                ));
            } else {
                // Otherwise it should still be listening for new local models
                assert!(matches!(sa.state, State::WaitingLocalModels));
            }
        }

        sa.put_event(MotherboardOrMOMEvent::Motherboard(
            MotherboardEvent::TaskExecDone,
        ));
    }
}
