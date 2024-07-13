use super::NodeInfo;
use std::sync::Arc;
use std::fmt;
use std::fmt::Formatter;


#[derive(Clone)]
pub struct RegistrationConfirmation {
    pub node_list: Arc<Vec<NodeInfo>>, // list of nodes attributed by the aggregator.
}

#[derive(Clone)]
pub struct SendGlobalModel {
    pub number_local_epochs: u8, // number of local epochs the trainer should perform.
}

#[derive(Clone)]
pub struct Kill;

#[derive(Clone)]
pub struct RegistrationRequest {
    pub node_to_register: NodeInfo, // the node that the aggregator should register.
}

#[derive(Clone)]
pub struct SendLocalModel {
    pub number_local_epochs_done: u8, // the number of local epochs that the trainer actually did.
}

#[derive(Clone)]
pub enum Operation {
    RegistrationConfirmation(RegistrationConfirmation),
    SendGlobalModel(SendGlobalModel),
    Kill(Kill),
    RegistrationRequest(RegistrationRequest),
    SendLocalModel(SendLocalModel),
}

impl fmt::Display for Operation {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Operation::RegistrationConfirmation(_) => write!(f, "RegistrationConfirmation"),
            Operation::SendGlobalModel(_) => write!(f, "SendGlobalModel"),
            Operation::Kill(_) => write!(f, "Kill"),
            Operation::RegistrationRequest(_) => write!(f, "RegistrationRequest"),
            Operation::SendLocalModel(_) => write!(f, "SendLocalModel"),
        }
    }
}
