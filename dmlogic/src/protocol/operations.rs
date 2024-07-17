use crate::bridge::ffi::NodeInfo;
use std::fmt;
use std::fmt::Formatter;
use std::sync::Arc;

#[derive(Clone)]
pub enum Operation {
    RegistrationConfirmation {
        node_list: Vec<NodeInfo>, // list of nodes attributed by the aggregator.
    },
    SendGlobalModel {
        number_local_epochs: u64, // number of local epochs the trainer should perform.
    },
    Kill,
    RegistrationRequest {
        node_to_register: NodeInfo, // the node that the aggregator should register.
    },
    SendLocalModel {
        number_local_epochs_done: u64, // the number of local epochs that the trainer actually did.
    },
}

impl fmt::Display for Operation {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        match self {
            Operation::RegistrationConfirmation { .. } => write!(f, "RegistrationConfirmation"),
            Operation::SendGlobalModel { .. } => write!(f, "SendGlobalModel"),
            Operation::Kill => write!(f, "Kill"),
            Operation::RegistrationRequest { .. } => write!(f, "RegistrationRequest"),
            Operation::SendLocalModel { .. } => write!(f, "SendLocalModel"),
        }
    }
}
