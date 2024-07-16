use super::common::{AggregatorType, Arg, ClusterTopology, Constants, NetworkManager, TrainerType};
use rand::seq::SliceRandom;
use rand::thread_rng;
use serde::{Deserialize, Serialize};

#[derive(Debug, Deserialize, Serialize, Clone)]
#[serde(rename = "fried")]
pub struct FriedFalafels {
    // Struct fields cannot contain @ so we rename it with this macro
    #[serde(rename = "@version")]
    pub version: String,
    pub constants: Constants,
    #[serde(rename = "cluster")]
    pub clusters: Vec<Cluster>,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Cluster {
    #[serde(rename = "@topology")]
    pub topology: ClusterTopology,
    #[serde(rename = "node")]
    pub nodes: Vec<Node>,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Node {
    #[serde(rename = "@name")]
    pub name: String,

    // Rename XML element as the name of the enum member
    #[serde(rename = "$value")]
    pub role: NodeRole,

    #[serde(rename = "network-manager")]
    pub network_manager: NetworkManager,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub enum NodeRole {
    // Renaming members without capital leter as in XML format
    #[serde(rename = "aggregator")]
    Aggregator(Aggregator),
    #[serde(rename = "trainer")]
    Trainer(Trainer),
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Aggregator {
    #[serde(rename = "@type")]
    pub aggregator_type: AggregatorType,

    #[serde(rename = "arg", skip_serializing_if = "Option::is_none")]
    pub args: Option<Vec<Arg>>,
}

#[derive(Debug, Deserialize, Serialize, Clone)]
pub struct Trainer {
    #[serde(rename = "@type")]
    pub trainer_type: TrainerType,

    #[serde(rename = "arg", skip_serializing_if = "Option::is_none")]
    pub args: Option<Vec<Arg>>,
}

impl FriedFalafels {
    /// Gets the name of every nodes among every clusters
    fn get_node_names(&self) -> Vec<String> {
        self
            .clusters
            .iter()
            .map(|c| c.nodes.iter().map(|n| n.name.clone()).collect::<Vec<_>>())
            .flatten()
            .collect::<Vec<_>>()
    }

    pub fn shuffle_node_names(&mut self) {
        let mut names = self.get_node_names(); 

        // Shuffle the names
        let mut rng = thread_rng();
        names.shuffle(&mut rng);

        self.clusters.iter_mut().for_each(|c| {
            c.nodes
                .iter_mut()
                .for_each(|n| n.name = names.pop().unwrap())
        });
    }
}

#[cfg(test)]
mod tests {
    use std::fs;

    // Note this useful idiom: importing names from outer (for mod tests) scope.
    use super::*;

    #[test]
    fn test_get_names() {
        let content = String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let names = fried.get_node_names();

        assert_eq!(names, ["Node 1", "Node 2", "Node 3", "Node 4", "Node 5", "Node 6", "Node 7", "Node 8", "Node 9", "Node 10", "Node 11"]);
    }

    #[test]
    fn test_shuffle() {
        let content = String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let names = fried.get_node_names();

        fried.shuffle_node_names();

        let names_shuffled = fried.get_node_names();

        // I guess there is a small probability that the result is the same?
        assert_ne!(names, names_shuffled);
    }
}
