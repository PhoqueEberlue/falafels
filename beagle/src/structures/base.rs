use serde::{Serialize, Deserialize};
use fryer::structures::raw::Cluster;

/// A Base contains the skeleton of a cluster that we want to evaluate by tweaking its base
/// paramaters.
#[derive(Deserialize, Debug, Clone)]
pub struct Clusters {
    #[serde(rename = "cluster")]
    pub list: Vec<Cluster>,
}
