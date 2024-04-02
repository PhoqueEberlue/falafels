use serde::Deserialize;
use crate::structures::common::{AggregatorType, Arg, Constants, NetworkManager, TrainerType};

/// Represents <root>...</root>
#[derive(Deserialize, Debug)]
pub struct RawFalafels {
    pub constants: Constants,
    pub trainers: Trainers,
    pub aggregators: Aggregators,
}

#[derive(Deserialize, Debug, Clone)]
pub struct Trainers {
    #[serde(rename = "@number")]
    pub number: u8,

    #[serde(rename = "@type")]
    pub trainer_type: TrainerType,

    #[serde(rename = "network-manager")]
    pub network_manager: NetworkManager,

    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}

#[derive(Deserialize, Debug)]
pub struct Aggregators {
    #[serde(rename = "@number")]
    pub number: u8,

    #[serde(rename = "@type")]
    pub aggregator_type: AggregatorType,

    #[serde(rename = "network-manager")]
    pub network_manager: NetworkManager,

    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}
