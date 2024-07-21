pub mod filters;
pub mod operations;
pub mod packet;

#[derive(Clone)]
pub enum RoleEnum {
    MainAggregator,
    Aggregator,
    Trainer,
}

#[derive(Clone)]
pub struct NodeInfo {
    pub name: String,
    pub role: RoleEnum,
}
