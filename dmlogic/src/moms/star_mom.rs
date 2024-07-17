use crate::{bridge::ffi::NodeInfo, motherboard::{MotherbroadOrRoleEvent, TaskSend}, protocol::{operations::Operation, packet::Packet}, roles::RoleEvent};

use super::{MOMEvent, MOM};

// States for StarMom, do not expose
enum State
{
    Initializing,
    WaitingRegistrationRequest,
    WaitingRegistrationConfirmation,
    Running,
    Killing,
}

struct StarMom {
    connected_nodes: Vec<NodeInfo>,
    /// Store operations of type RegistrationRequest
    registration_requests: Vec<Operation>,
    tasks: Vec<TaskSend>,
}


impl StarMom {
    fn new(node_info: NodeInfo) -> Self {
        Self {
            connected_nodes: Vec::new(),
            registration_requests: Vec::new(),
            tasks: Vec::new(),
        }
    }

    fn handle_registration_requests(&mut self) {

        for request in self.registration_requests {
            if let Operation::RegistrationRequest { node_to_register } = request {

                // Add this node to the connected nodes
                self.connected_nodes.push(node_to_register);

                // TODO
                // TaskSend { packet: Packet {} }
            }
        }
        // for (auto request : *this->registration_requests)
        // {
        //     this->connected_nodes->push_back(request.node_to_register); 

        //     auto node_list = vector<NodeInfo>();
        //     node_list.push_back(this->my_node_info);

        //     auto res_p = make_unique<Packet>(Packet(
        //         request.node_to_register.name, request.node_to_register.name,
        //         operations::RegistrationConfirmation(
        //             make_shared<vector<NodeInfo>>(node_list)
        //         )
        //     ));

        //     this->send_async(res_p);
        // }

        // this->mp->put_nm_event(
        //     new Mediator::Event {
        //         Mediator::ClusterConnected { .number_client_connected=(uint16_t)this->connected_nodes->size() }
        //     }
        // );
    }

    fn send_registration_request(&self) {
        // implementation
    }

    fn handle_registration_confirmation(&self, confirmation: &Operation) {
        // implementation
    }

    fn broadcast(&self, packet: Packet, is_redirected: bool) {
        // implementation
    }

    fn handle_kill_phase(&self) {
        // implementation
    }

    fn run_one_step(&mut self, event: MotherboardOrMOMEvent) -> Option<RoleEvent> {
        match self.state {
            State::Initializing => self.initializing(event),
            State::WaitingLocalModels => self.waiting_local_models(event),
            State::Aggregating => self.aggregating(event),
        }
    }
}

impl MOM for StarMom {
    fn put_event(&mut self, event: MotherbroadOrRoleEvent) -> Option<MOMEvent> {
        self.run_one_step(event)
    }

    fn pop_task(&mut self) -> Option<TaskSend> {
        self.tasks.pop()
    }

    fn add_task(&mut self, task: TaskSend) {
        self.tasks.push(task);
    }
}
