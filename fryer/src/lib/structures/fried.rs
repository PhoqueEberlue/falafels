use serde::{Deserialize, Serialize};
use super::common::{Constants, Arg, NetworkManager, AggregatorType, TrainerType, ClusterTopology};

#[derive(Debug, Deserialize, Serialize)]
#[serde(rename = "fried")]
pub struct FriedFalafels {
    // Struct fields cannot contain @ so we rename it with this macro
    #[serde(rename = "@version")]
    pub version: String,
    pub constants: Constants,
    pub clusters: Clusters,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Clusters {
    #[serde(rename = "cluster")]
    pub list: Vec<Cluster>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Cluster {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@topology")]
    pub topology: ClusterTopology,
    #[serde(rename = "node")]
    pub nodes: Vec<Node>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Node {
    #[serde(rename = "@name")]
    pub name: String,

    // Rename XML element as the name of the enum member
    #[serde(rename = "$value")]
    pub role: RoleEnum,

    #[serde(rename = "network-manager")]
    pub network_manager: NetworkManager,
}

#[derive(Debug, Deserialize, Serialize)]
pub enum RoleEnum {
    // Renaming members without capital leter as in XML format 
    #[serde(rename = "aggregator")]
    Aggregator(Aggregator),

    #[serde(rename = "trainer")]
    Trainer(Trainer)
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Aggregator {
    #[serde(rename = "@type")]
    pub aggregator_type: AggregatorType,

    #[serde(rename = "arg", skip_serializing_if = "Option::is_none")]
    pub args: Option<Vec<Arg>>,
}

#[derive(Debug, Deserialize, Serialize)]
pub struct Trainer {
    #[serde(rename = "@type")]
    pub trainer_type: TrainerType,
    
    #[serde(rename = "arg", skip_serializing_if = "Option::is_none")]
    pub args: Option<Vec<Arg>>,
}
