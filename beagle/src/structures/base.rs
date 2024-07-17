use fryer::structures::raw::Cluster;
use serde::{Deserialize, Serialize};

/// Contains the clusters only (compared to RawFalafels). Used to tweak the parameters of the
/// Clusters in a Study
#[derive(Deserialize, Debug, Clone)]
pub struct Clusters {
    #[serde(rename = "cluster")]
    pub list: Vec<Cluster>,
}
