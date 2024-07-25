pub mod filters;
pub mod operations;
pub mod packet;

#[derive(Clone, Debug)]
pub enum RoleEnum {
    MainAggregator,
    Aggregator,
    Trainer,
}

#[derive(Clone, Debug)]
pub struct NodeInfo {
    pub name: String,
    pub role: RoleEnum,
}
