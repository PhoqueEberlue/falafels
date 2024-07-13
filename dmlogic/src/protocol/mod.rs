use crate::roles::NodeRole;

mod packet;
mod operations;
mod filters;

#[derive(Clone)]
pub struct NodeInfo {
    pub name: String,
    pub role: NodeRole,
}


