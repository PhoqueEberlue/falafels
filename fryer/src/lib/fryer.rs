use crate::structures::{
    common::{AggregatorType, Arg, ClusterTopology},
    fried::{Aggregator, FriedFalafels, Node, NodeRole, Trainer},
};

use super::structures::{common, fried, raw};
use core::panic;
use quick_xml::de::from_str;
use rand::{self, RngCore, SeedableRng};
use serde::Serialize;
use std::{borrow::BorrowMut, fs};

/// Tool to fry falafels.
/// Load a raw-falafels file to produce fried-falafels file.
pub struct Fryer {
    names_list: Option<NamesList>,
    count_node: u32,
}

impl Fryer {
    /// Create a new Fryer, Optionnaly pass a file path containing names to be used for node naming
    pub fn new(optional_list_path: Option<&str>) -> Fryer {
        let mut fryer = Fryer {
            names_list: None,
            count_node: 0,
        };

        // If a list path have been provided
        if let Some(list_path) = optional_list_path {
            // Create a NamesList structure
            fryer.names_list = Some(NamesList::new(list_path));
        }

        fryer
    }

    /// Load the content of a raw-falafels file by deserializing then returns a RawFalafels
    /// structure.
    pub fn load_raw_falafels(&mut self, file_path: &str) -> raw::RawFalafels {
        println!("Loading raw falafels at `{}`... 📰", file_path);

        let content = String::from_utf8(fs::read(file_path).unwrap()).unwrap();

        match from_str(&content) {
            Ok(raw_falafels) => raw_falafels,
            Err(e) => panic!("Error while parsing raw falafels file: {}", e),
        }
    }

    /// Generate a FriedFalafels structure from a RawFalafels one.
    pub fn fry(&mut self, rf: &raw::RawFalafels) -> fried::FriedFalafels {
        println!("Frying those falafels... 🧑🍳");

        if let Some(names_list) = &self.names_list {
            // Panic if the list is too short
            names_list.is_list_big_enough(Fryer::count_total_number_nodes(&rf.clusters));
        }

        let mut fried_clusters = Vec::<fried::Cluster>::new();

        for raw_cluster in &rf.clusters {
            // Create cluster nodes
            let fried_cluster = self.create_cluster(&raw_cluster);
            // Append them to the other nodes
            fried_clusters.push(fried_cluster);
        }

        // Create FriedFalafels
        let mut ff = fried::FriedFalafels {
            // Setting version, cloning constants (because no processing needed), creating list of node
            version: "0.1".to_string(),
            // Cloning constants because no treatment needed
            constants: rf.constants.clone(),
            // Assigning our nodes
            clusters: fried_clusters,
        };

        // Link hierarchical aggregators in case we are in a hierarchical scenario
        ff.link_hierarchical_aggregators();

        // Add bootstrap-node arguments
        ff.add_booststrap_nodes();

        println!("Falafels ready! 🧆");

        // Resets the counter cause fry() can be called multiple times
        self.count_node = 0;
        // TODO: also reset the name list!

        ff
    } 

    /// Count the total number of nodes in every clusters
    fn count_total_number_nodes(clusters: &Vec<raw::Cluster>) -> u16 {
        clusters
            .iter()
            .map(|c| c.trainers.number + c.aggregators.number)
            .sum::<u16>()
    }

    fn create_cluster(&mut self, cluster: &raw::Cluster) -> fried::Cluster {
        let mut fried_cluster = fried::Cluster {
            nodes: Vec::new(),
            topology: cluster.topology.clone(),
        };

        let mut trainer_nodes = self.create_trainer_nodes(cluster);

        let mut aggregator_nodes = self.create_aggregator_nodes(cluster);

        fried_cluster.nodes.append(&mut trainer_nodes);
        fried_cluster.nodes.append(&mut aggregator_nodes);

        fried_cluster
    }

    fn create_trainer_nodes(&mut self, cluster_param: &raw::Cluster) -> Vec<fried::Node> {
        let mut res = Vec::<fried::Node>::new();

        // Creating the trainers(s)
        for _ in 0..cluster_param.trainers.number {
            let role = fried::NodeRole::Trainer(fried::Trainer {
                trainer_type: cluster_param.trainers.trainer_type.clone(),
                args: cluster_param.trainers.args.clone(),
            });

            let nm = match &cluster_param.trainers.network_manager {
                Some(nm) => nm.clone(),
                None => common::NetworkManager { args: None },
            };

            let node = fried::Node {
                name: self.pick_node_name(),
                role,
                network_manager: nm,
            };

            res.push(node);
        }

        res
    }

    fn create_aggregator_nodes(&mut self, cluster_param: &raw::Cluster) -> Vec<fried::Node> {
        if cluster_param.topology != ClusterTopology::RingUni
            && cluster_param.aggregators.number > 1
        {
            panic!("Only UniRing can support multiple aggregators");
        }

        let mut res = Vec::<fried::Node>::new();
        let mut main_aggregator = false;

        // Creating the aggregator(s)
        for _ in 0..cluster_param.aggregators.number {
            let mut aggregator = fried::Aggregator {
                aggregator_type: cluster_param.aggregators.aggregator_type.clone(),
                args: cluster_param.aggregators.args.clone(),
            };

            let args = aggregator.args.get_or_insert_with(|| Vec::new());

            // Assign one main aggregator (per cluster)
            if !main_aggregator {
                // push argument to enable is_main_aggregator flag
                args.push(Arg {
                    name: "is_main_aggregator".to_string(),
                    value: "1".to_string(),
                });
                main_aggregator = true;
            }

            // Add default value of 3 number of local epochs
            args.push(Arg {
                name: "number_local_epochs".to_string(),
                value: "3".to_string(),
            });

            if let AggregatorType::Asynchronous = aggregator.aggregator_type {
                // Add default value for proportion_threshold
                args.push(Arg {
                    name: "proportion_threshold".to_string(),
                    value: "0.5".to_string(),
                });
            }

            let role = fried::NodeRole::Aggregator(aggregator);

            let nm = match &cluster_param.aggregators.network_manager {
                Some(nm) => nm.clone(),
                None => common::NetworkManager { args: None },
            };

            let aggregator_node = fried::Node {
                name: self.pick_node_name(),
                role,
                network_manager: nm,
            };

            res.push(aggregator_node);
        }

        res
    }

    /// Wraper that either call NamesList.pick_name() if it hase been initialized, or attribute a
    /// number to the node and increment the counter
    fn pick_node_name(&mut self) -> String {
        if let Some(l) = &mut self.names_list {
            l.pick_name()
        } else {
            self.count_node += 1;
            format!("Node {}", self.count_node)
        }
    }

    /// Serialize and write to a file a FriedFalafels structure.
    pub fn write_fried_falafels(path: &str, ff: &fried::FriedFalafels) {
        println!("Writing the fried falafels to `{}`...", path);
        let mut result_buffer = String::from("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

        let mut serializer = quick_xml::se::Serializer::new(&mut result_buffer);
        serializer.expand_empty_elements(false);
        serializer.indent(' ', 4);

        ff.serialize(serializer).unwrap();

        fs::write(path, result_buffer).unwrap();
    }
}

/// Handle naming nodes with custom names
struct NamesList {
    list_of_names: Vec<String>,
}

impl NamesList {
    /// Creates a new NamesList from a text file. The names should be separated with new lines.
    fn new(list_path: &str) -> NamesList {
        let byte_content = fs::read(list_path).expect("Names list path not found");

        let content = String::from_utf8(byte_content)
            .expect("Error while parsing names list file content: Expecting utf8 encoding");

        let list_of_names = content
            .lines() // Splitting by lines
            .map(|s| String::from(s)) // Creating owned values for each line
            .collect::<Vec<String>>(); // Collect the result

        NamesList { list_of_names }
    }

    /// Verifying that the list is big enough for the total number of picks we need
    fn is_list_big_enough(&self, total_nb: u16) {
        if self.list_of_names.len() < total_nb.into() {
            panic!("It seems that the provided list of names does not contain enough names ({}) for the total number we want to pick ({})", self.list_of_names.len(), total_nb);
        }
    }

    /// Randomly pick a name and remove it from the list to prevent it from being picked again.
    fn pick_name(&mut self) -> String {
        let mut generator = rand::rngs::StdRng::seed_from_u64(42);

        // Using next_u64() + modulo operator instead of gen_range() because this last produces
        // weirdly distributed outputs
        let remove_index = generator
            .next_u64()
            .rem_euclid(self.list_of_names.len() as u64);

        // Remove the name at remove_index from the list and return it
        self.list_of_names.remove(remove_index as usize)
    }
}
