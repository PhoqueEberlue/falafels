use core::panic;
use std::borrow::BorrowMut;

use crate::{
    motherboard::{MotherboardEvent, MotherboardTask, MotherbroadOrRoleEvent},
    protocol::{filters::NodeFilter, operations::Operation, packet::Packet, NodeInfo, RoleEnum},
    roles::RoleEvent,
};

use super::{MOMBase, MOMEvent, MOM};

// States for StarMom, do not expose
enum State {
    Initializing,
    WaitingRegistrationRequests,
    WaitingRegistrationConfirmation,
    Running,
    Killing,
}

struct StarMom {
    base: MOMBase,
    state: State,
    connected_nodes: Vec<NodeInfo>,
    /// Store NodeInfo of nodes that requested to register
    registration_requests: Vec<NodeInfo>,
    tasks: Vec<MotherboardTask>,
    timeout_task_sent: bool,
}

impl StarMom {
    fn new(my_node_info: NodeInfo, bootstrap_nodes: Vec<NodeInfo>) -> Self {
        Self {
            base: MOMBase::new(my_node_info, bootstrap_nodes),
            state: State::Initializing,
            connected_nodes: Vec::new(),
            registration_requests: Vec::new(),
            tasks: Vec::new(),
            timeout_task_sent: false,
        }
    }

    fn handle_registration_requests(&mut self) -> MOMEvent {
        let mut confirmations = self
            .registration_requests
            .iter()
            .map(|n| {
                // Add send task for RegistrationConfirmation
                MotherboardTask::Send(Packet::new(
                    n.name.clone(),
                    n.name.clone(),
                    Operation::RegistrationConfirmation {
                        node_list: vec![self.base.my_node_info.clone()],
                    },
                ))
            })
            .collect();

        self.tasks.append(&mut confirmations);

        // Replace empty connected nodes list with the registration requests list
        assert_eq!(self.connected_nodes.len(), 0,
            "The connected_nodes is supposed to be empty at this point and will be replaced by the registration_requests list, thus erasing the previous one."); // Just check in case for futures changes

        self.connected_nodes = self.registration_requests.clone();

        // After sending every confirmation, send ClusterConnected event with the number of
        // trainers
        MOMEvent::ClusterConnected(self.connected_nodes.len() as u64)
    }

    fn send_registration_request(&mut self) {
        let bootstrap_node = self.base.bootstrap_nodes.get(0).unwrap();

        self.tasks.push(MotherboardTask::Send(Packet::new(
            bootstrap_node.name.clone(),
            bootstrap_node.name.clone(),
            Operation::RegistrationRequest {
                node_to_register: self.base.my_node_info.clone(),
            },
        )));
    }

    /// Broadcast a packet to everyone we know.
    /// TODO: handle is_redirected
    fn broadcast(&mut self, packet: Packet, is_redirected: bool) {
        // Create a Send task for every connected node
        let mut tasks = self
            .connected_nodes
            .iter()
            .map(|n| {
                let mut p = packet.clone();
                p.src = self.base.my_node_info.name.clone();
                p.dst = n.name.clone();
                MotherboardTask::Send(p)
            })
            .collect();

        self.tasks.append(&mut tasks);
    }

    fn handle_received_packet(&mut self, packet: Packet) -> Option<MOMEvent> {
        println!(
            "{} <--{}({})--- {}",
            packet.dst, packet.op, packet.id, packet.src
        );

        // Case where we receive a kill
        if let Operation::Kill = packet.op {
            // TODO: I think that's not needed
            // self.broadcast(packet, true);

            self.state = State::Killing;

            // TODO: maybe cancel every sending tasks like we did in C++?
            // this->clear_async_puts();
        }

        self.base.if_target_get_event(packet)
    }

    fn handle_to_be_sent_packet(&mut self, filter: NodeFilter, op: Operation) -> Option<MOMEvent> {
        let packet = Packet::new_broadcast(filter, op);

        // Case where we send kill to someone else
        if let Operation::Kill = packet.op {
            self.state = State::Killing;

            // TODO: maybe cancel every sending tasks like we did in C++?
            // this->clear_async_puts();
        }

        self.broadcast(packet, false);

        None
    }

    // TODO: not sure I need to handle this
    fn handle_kill_phase(&self) {
        // implementation
    }

    // ---------------------------------- State related functions ---------------------------------
    fn initializing(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        match event {
            MotherbroadOrRoleEvent::Motherboard(MotherboardEvent::MotherboardInitialized) => {
                match self.base.my_node_info.role {
                    RoleEnum::Trainer => {
                        self.send_registration_request();
                        self.state = State::WaitingRegistrationConfirmation;
                    }
                    RoleEnum::MainAggregator => {
                        self.state = State::WaitingRegistrationRequests;
                    }
                    RoleEnum::Aggregator => {
                        panic!(
                            "The StarNetworkManager can't have a secondary Aggregator, only a Main one."
                        );
                    }
                };
            }
            _ => {}
        };

        None
    }

    fn waiting_registration_confirmation(
        &mut self,
        mut event: MotherbroadOrRoleEvent,
    ) -> Option<MOMEvent> {
        match event.borrow_mut() {
            MotherbroadOrRoleEvent::Motherboard(MotherboardEvent::PacketReceived(packet)) => {
                if let Operation::RegistrationConfirmation { node_list } = &mut packet.op {
                    // Add connected_nodes
                    self.connected_nodes.append(node_list);
                    self.state = State::Running;
                    // Return node connected event
                    Some(MOMEvent::NodeConnected)
                } else {
                    None
                }
            }
            _ => None,
        }
    }

    fn waiting_registration_requests(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        // Add a Timeout task to quit this State at a certain amount of time
        if !self.timeout_task_sent {
            // TODO: add the amount as a parameter
            self.tasks.push(MotherboardTask::Timeout(20.0));
        }

        match event {
            // Case we receive a packet
            MotherbroadOrRoleEvent::Motherboard(MotherboardEvent::PacketReceived(packet)) => {
                // If it's a RegistrationRequest we add the NodeInfo to the list
                if let Operation::RegistrationRequest { node_to_register } = packet.op {
                    self.registration_requests.push(node_to_register);
                }
            }
            // Case timeout reached
            MotherbroadOrRoleEvent::Motherboard(MotherboardEvent::TaskTimeoutDone) => {
                self.handle_registration_requests();
                self.state = State::Running;
            }
            _ => {}
        }

        None
    }

    fn running(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        match event {
            MotherbroadOrRoleEvent::Motherboard(MotherboardEvent::PacketReceived(packet)) => {
                // Route packet to the Role if we're a packet target
                self.handle_received_packet(packet)
            }
            MotherbroadOrRoleEvent::Role(RoleEvent::ToBeSentPacket { filter, op }) => {
                // Send the packet
                self.handle_to_be_sent_packet(filter, op)
            }
            _ => None,
        }
    }

    fn killing(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        unimplemented!()
    }

    // -------------------------------------------------------------------------------------------
    pub fn run_one_step(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        match self.state {
            State::Initializing => self.initializing(event),
            State::WaitingRegistrationConfirmation => self.waiting_registration_confirmation(event),
            State::WaitingRegistrationRequests => self.waiting_registration_requests(event),
            State::Running => self.running(event),
            // TODO: I think we have nothing to do here.
            // IDK how to handle kill yet, we'll see that later, don't forget to add tests
            State::Killing => self.killing(event),
        }
    }
}

impl MOM for StarMom {
    fn put_event(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        self.run_one_step(event)
    }

    fn pop_task(&mut self) -> Option<MotherboardTask> {
        self.tasks.pop()
    }
}

#[cfg(test)]
mod tests {

    use super::*;

    #[test]
    fn test_typical_workflow_for_trainer() {
        let aggregator_info = NodeInfo {
            name: "Node 2".to_string(),
            role: RoleEnum::MainAggregator,
        };

        let mut sm = StarMom::new(
            // The node of the MOM
            NodeInfo {
                name: "Node 1".to_string(),
                role: RoleEnum::Trainer,
            },
            // bootstrap node is a MainAggregator
            vec![aggregator_info.clone()],
        );

        sm.put_event(MotherbroadOrRoleEvent::Motherboard(
            MotherboardEvent::MotherboardInitialized,
        ));

        assert!(matches!(sm.state, State::WaitingRegistrationConfirmation));

        let task = sm.pop_task().unwrap();

        assert!(matches!(task, MotherboardTask::Send(_)));

        if let MotherboardTask::Send(packet) = task {
            assert_eq!(packet.dst, "Node 2");
            assert_eq!(packet.final_dst, "Node 2");
        }

        let event = sm.put_event(MotherbroadOrRoleEvent::Motherboard(
            MotherboardEvent::PacketReceived(Packet::new(
                "Node 1".to_string(),
                "Node 1".to_string(),
                Operation::RegistrationConfirmation {
                    node_list: vec![aggregator_info.clone()],
                },
            )),
        ));

        assert!(matches!(sm.state, State::Running));
        assert!(matches!(event, Some(MOMEvent::NodeConnected)));

        // Pretend that we send a global model
        let event = sm.put_event(MotherbroadOrRoleEvent::Motherboard(
            MotherboardEvent::PacketReceived(Packet::new_broadcast(
                // With this filter, it should reroute the packet to Role
                NodeFilter::Trainers,
                Operation::SendGlobalModel {
                    number_local_epochs: 3,
                },
            )),
        ));

        // Still running
        assert!(matches!(sm.state, State::Running));
        // Make sure the MOM routed the received operation to the Role
        assert!(matches!(
            event,
            Some(MOMEvent::OperationReceived(Operation::SendGlobalModel {
                number_local_epochs: 3,
            }))
        ));
    }

    // TODO: add more tests
}
