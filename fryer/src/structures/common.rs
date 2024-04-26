use serde::{Serialize, Deserialize};

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Constants {
    #[serde(rename = "constant")]
    list: Vec<Constant>,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub struct Constant {
    #[serde(rename = "@name")]
    name: String,

    #[serde(rename = "@value")]
    value: String
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Arg {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@value")]
    pub value: String,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct NetworkManager {
    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}

#[derive(Deserialize, Serialize, Debug, Clone)]
pub enum AggregatorType {
    #[serde(rename = "simple")]
    Simple,
    #[serde(rename = "asynchronous")]
    Asynchronous,
}

#[derive(Deserialize, Serialize, Debug, Clone)]
pub enum TrainerType {
    #[serde(rename = "simple")]
    Simple,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Prop {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@value")]
    pub value: String,
}

#[derive(Serialize, Deserialize, Debug, Clone)]
pub enum ClusterTopology {
    #[serde(rename = "star")]
    Star,
    #[serde(rename = "ring")]
    Ring,
    #[serde(rename = "fully-connected")]
    FullyConnected,
}
