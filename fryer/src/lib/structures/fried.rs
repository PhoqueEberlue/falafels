use std::borrow::BorrowMut;
use std::usize;

use super::common::{AggregatorType, Arg, ClusterTopology, Constants, NetworkManager, TrainerType};
use rand::rngs::StdRng;
use rand::{seq::SliceRandom, Rng};
use rand::{thread_rng, SeedableRng};
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
        self.clusters
            .iter()
            .map(|c| c.nodes.iter().map(|n| n.name.clone()).collect::<Vec<_>>())
            .flatten()
            .collect::<Vec<_>>()
    }

    pub fn get_number_nodes(&self) -> usize {
        // Count nodes in each cluster and sum
        self.clusters.iter().map(|c| c.nodes.iter().count()).sum()
    }

    pub fn shuffle_node_names(&mut self, rng: &mut StdRng) {
        let mut names = self.get_node_names();

        // Shuffle the names
        names.shuffle(rng);

        let mut count = 0;
        self.clusters.iter_mut().for_each(|c| {
            c.nodes.iter_mut().for_each(|n| {
                n.name = names[count].clone();
                count += 1;
            })
        });
    }

    pub fn permute_node_names(&mut self, rng: &mut StdRng, nb_permutations: u32) {
        let mut names = self.get_node_names();

        // Generate `nb_permutations`, a list of tupples containing two usize
        let permuts = (0..nb_permutations)
            .map(|_| (rng.gen_range(0..names.len()), rng.gen_range(0..names.len())))
            .collect::<Vec<_>>();

        // Perform the permutations
        permuts.into_iter().for_each(|(a, b)| {
            let tmp = names[a].clone();
            names[a] = names[b].clone();
            names[b] = tmp;
        });

        // Redistribute nodes names
        let mut count = 0;
        self.clusters.iter_mut().for_each(|c| {
            c.nodes.iter_mut().for_each(|n| {
                n.name = names[count].clone();
                count += 1;
            })
        });
    }

    pub fn mutate_nodes(&mut self, rng: &mut StdRng) {
        self.clusters.iter_mut().for_each(|c| {
            c.nodes.iter_mut().for_each(|n| match n.role.borrow_mut() {
                NodeRole::Aggregator(a) => {
                    FriedFalafels::mutate_aggregator(a, rng);
                }
                _ => {}
            })
        });
    }

    pub fn mutate_aggregator(aggregator: &mut Aggregator, rng: &mut StdRng) {
        // Randomly increase/decrease the number of local epochs the aggregator will ask the trainer to do
        FriedFalafels::set_arg_value_with(
            aggregator.args.borrow_mut(),
            "number_local_epochs",
            |value_opt| {
                // Generate random variation
                let incr: i64 = rng.gen_range(-1..1);
                match value_opt {
                    Some(value) => {
                        let parsed_value = value.parse::<i64>().unwrap();
                        match parsed_value {
                            // We can't go less than one
                            1 => (parsed_value + incr.abs()).to_string(),
                            // Otherwise either substract one, add one, or add 0
                            _ => (parsed_value + incr).to_string(),
                        }
                    }
                    // Case number_local_epochs wasn't initialized, stick to 3
                    None => "3".to_string(),
                }
            },
        );

        // Randomly increase/decrease the proportion_threshold if it is an asynchronous aggregator
        if let AggregatorType::Asynchronous = aggregator.aggregator_type {
            FriedFalafels::set_arg_value_with(
                aggregator.args.borrow_mut(),
                "proportion_threshold",
                |value_opt| {
                    // Generate random variation either -0.1, 0.0 or 0.1
                    let mut incr = rng.gen_range(-1..2) as f64;
                    incr = incr / 10.0;
                    match value_opt {
                        Some(value) => {
                            let parsed_value = value.parse::<f64>().unwrap();
                            if parsed_value < 0.2 {
                                return format!("{:.1}", parsed_value + incr.abs());
                            } else if parsed_value > 0.8 {
                                return format!("{:.1}", parsed_value - incr.abs());
                            } else {
                                return format!("{:.1}", parsed_value + incr);
                            }
                        }
                        // Case the proportion_threshold wasn't initialized, stick to 0.5
                        None => "0.5".to_string(),
                    }
                },
            );
        }
    }

    /// Set an argument value with a function `f`.
    /// If the argument (or the argument vector) didn't exist initialize it.
    /// In this case None is passed to `f` which lets the user
    /// handle this particular case.
    /// The function can capture variables such as rng contexts.
    pub fn set_arg_value_with<F>(args_opt: &mut Option<Vec<Arg>>, arg_name: &str, mut f: F)
    where
        F: FnMut(Option<&String>) -> String,
    {
        // Get mutable reference of the Arg vector and intialize it if needed
        let args = args_opt.get_or_insert_with(|| Vec::new());

        // Find the argument name in the vector
        match args.iter_mut().find(|a| a.name == arg_name) {
            // If it exists pass its previous value to `f` and update it with its result
            Some(arg) => arg.value = f(Some(&arg.value)),
            // Otherwise create the argument and pass None to `f`
            None => args.push(Arg {
                name: arg_name.to_string(),
                value: f(None),
            }),
        };
    }

    /// Set an argument value, erase previous value if it existed, otherwise create the argument.
    pub fn set_arg_value(args_opt: &mut Option<Vec<Arg>>, arg_name: &str, new_value: String) {
        // Get mutable reference of the Arg vector and intialize it if needed
        let args = args_opt.get_or_insert_with(|| Vec::new());

        // Find the argument name in the vector
        match args.iter_mut().find(|a| a.name == arg_name) {
            // If it exists pass its previous value to `f` and update it with its result
            Some(arg) => arg.value = new_value,
            // Otherwise create the argument and pass None to `f`
            None => args.push(Arg {
                name: arg_name.to_string(),
                value: new_value,
            }),
        };
    }

    /// Gets an argument reference.
    pub fn get_arg<'a>(args_opt: &'a Option<Vec<Arg>>, arg_name: &str) -> Option<&'a Arg> {
        match args_opt {
            // Find the argument name in the vector
            Some(args) => args.iter().find(|a| a.name == arg_name),
            // Case there are no arguments
            None => None,
        }
    }

    /// TODO: write tests
    /// Add the main aggregator name as a bootstrap-node argument for every node of each cluster.
    /// It overwrites the value if a previous one existed.
    pub fn add_booststrap_nodes(&mut self) {
        self.clusters.iter_mut().for_each(|c| {
            let aggregator_node = c
                .nodes
                .iter()
                // Find the first aggregator, it should be the main one so we don't check here
                .find(|n| matches!(n.role, NodeRole::Aggregator(_)));

            // Unpack node name
            if let Some(Node { name, .. }) = aggregator_node {
                // Clone to lose borrowing of `c`
                let main_aggregator_name: String = name.clone();

                c.nodes.iter_mut().for_each(|n| {
                    // Only add to nodes that aren't the main_aggregator itself
                    if n.name != main_aggregator_name {
                        // Set bootstrap-node arg
                        FriedFalafels::set_arg_value(
                            &mut n.network_manager.args,
                            "bootstrap-node",
                            main_aggregator_name.clone(),
                        )
                    }
                })
            }
        });
    }

    /// TODO: write tests
    /// Link hierarchical aggregators to the central aggregator (the one that connects every
    /// subclusters).
    /// Note that the central aggregator is found in a Hierarchical cluster.
    pub fn link_hierarchical_aggregators(&mut self) {
        let mut central_aggregator_name_opt = None;

        // Star by finding the central_aggregator_name
        for cluster in self.clusters.iter() {
            // When a cluster has a Hierarchical topology, we link it to every other hierarchical
            // aggregators
            if let ClusterTopology::Hierarchical = cluster.topology {
                // Get the first node (because it should have only one aggregator which is the
                // central one)
                let aggregator_some = cluster.nodes.get(0);
                central_aggregator_name_opt = Some(aggregator_some.unwrap().name.clone());
            }
        }

        if let Some(central_aggregator_name) = central_aggregator_name_opt {
            // Then add a central_aggregator_name to every hierarchical aggregators
            // Start by iter every fried cluster
            self.clusters.iter_mut().for_each(
                // Iter through each node
                |c| {
                    c.nodes.iter_mut().for_each(
                        // If node is aggregator
                        |n| {
                            if let NodeRole::Aggregator(a) = n.role.borrow_mut() {
                                // of type hierarchical
                                if let AggregatorType::Hierarchical = a.aggregator_type {
                                    // Add argument with value of the central_aggregator_name
                                    FriedFalafels::set_arg_value(
                                        &mut a.args,
                                        "central_aggregator_name",
                                        central_aggregator_name.clone(),
                                    );
                                }
                            }
                        },
                    )
                },
            )
        }
    }
}

#[cfg(test)]
mod tests {
    use core::panic;
    use std::fs;

    use super::*;

    #[test]
    fn test_get_names() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let names = fried.get_node_names();

        assert_eq!(
            names,
            [
                "Node 1", "Node 2", "Node 3", "Node 4", "Node 5", "Node 6", "Node 7", "Node 8",
                "Node 9", "Node 10", "Node 11"
            ]
        );
    }

    #[test]
    fn test_get_number_nodes() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let nb = fried.get_number_nodes();

        assert_eq!(nb, 11);
    }

    #[test]
    fn test_get_arg() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let node = fried.clusters.get(0).unwrap().nodes.get(0).unwrap();

        let arg = FriedFalafels::get_arg(&node.network_manager.args, "bootstrap-node");

        assert_eq!(arg.unwrap().value, "Node 5");

        // Arg that doesn't exists (Zhīshì huǒguō: Fondue au fromage => this is clearly not an
        // argument supported by falafels)
        let arg = FriedFalafels::get_arg(&node.network_manager.args, "芝士火锅");

        assert!(matches!(arg, None));
    }

    #[test]
    fn test_set_arg_value() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        // Get a trainer
        let node = fried.clusters.get_mut(0).unwrap().nodes.get_mut(0).unwrap();

        // Verify value before modification
        assert_eq!(
            FriedFalafels::get_arg(&mut node.network_manager.args, "bootstrap-node")
                .unwrap()
                .value,
            "Node 5"
        );

        FriedFalafels::set_arg_value(
            &mut node.network_manager.args,
            "bootstrap-node",
            "Node 6".to_string(),
        );

        // Verify value after modification
        assert_eq!(
            FriedFalafels::get_arg(&mut node.network_manager.args, "bootstrap-node")
                .unwrap()
                .value,
            "Node 6"
        );
    }

    #[test]
    fn test_set_arg_value_with() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        // Get a hierarchical aggregator
        let node = fried.clusters.get_mut(1).unwrap().nodes.get_mut(4).unwrap();

        if let NodeRole::Aggregator(aggregator) = node.role.borrow_mut() {
            FriedFalafels::set_arg_value_with(
                &mut aggregator.args,
                "central_aggregator_name",
                |value_opt| match value_opt {
                    Some(value) => format!("{value} modified"),
                    None => "Default value".to_string(),
                },
            );

            // Here in this test the value should already exists so it has been modified
            assert_eq!(
                FriedFalafels::get_arg(&aggregator.args, "central_aggregator_name")
                    .unwrap()
                    .value,
                "Node 11 modified"
            );

            // Test where we set an argument that didn't existed
            FriedFalafels::set_arg_value_with(
                &mut aggregator.args,
                "new_argument_name",
                |value_opt| match value_opt {
                    Some(value) => format!("{value} modified"),
                    None => "Default value".to_string(),
                },
            );

            assert_eq!(
                FriedFalafels::get_arg(&aggregator.args, "new_argument_name")
                    .unwrap()
                    .value,
                "Default value"
            );
        } else {
            panic!("This test is supposed to grab an aggregator")
        }
    }

    #[test]
    fn test_shuffle_node_names() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let mut rng = StdRng::seed_from_u64(42);

        fried.shuffle_node_names(&mut rng);

        let names_shuffled = fried.get_node_names();

        assert_eq!(
            names_shuffled,
            [
                "Node 8", "Node 4", "Node 11", "Node 10", "Node 1", "Node 9", "Node 7", "Node 5",
                "Node 3", "Node 6", "Node 2"
            ]
        );
    }

    #[test]
    fn test_permute_node_names() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let mut rng = StdRng::seed_from_u64(42);

        fried.permute_node_names(&mut rng, 3);

        let names_shuffled = fried.get_node_names();

        assert_eq!(
            names_shuffled,
            [
                "Node 8", "Node 2", "Node 3", "Node 4", "Node 1", "Node 6", "Node 7", "Node 5",
                "Node 10", "Node 9", "Node 11"
            ]
        );
    }

    #[test]
    fn test_mutate_aggregator() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let mut rng = StdRng::seed_from_u64(22);

        // Get a hierarchical aggregator
        let node = fried.clusters.get_mut(1).unwrap().nodes.get_mut(4).unwrap();

        if let NodeRole::Aggregator(aggregator) = node.role.borrow_mut() {
            FriedFalafels::mutate_aggregator(aggregator, &mut rng);

            let arg = FriedFalafels::get_arg(&aggregator.args, "number_local_epochs");
            // The arg get's decreased to 2
            assert_eq!(arg.unwrap().value, "2");

            FriedFalafels::mutate_aggregator(aggregator, &mut rng);

            let arg = FriedFalafels::get_arg(&aggregator.args, "number_local_epochs");
            // The arg get's decreased to 1
            assert_eq!(arg.unwrap().value, "1");

            FriedFalafels::mutate_aggregator(aggregator, &mut rng);

            let arg = FriedFalafels::get_arg(&aggregator.args, "number_local_epochs");
            // The arg stays at 1
            assert_eq!(arg.unwrap().value, "1");

            FriedFalafels::mutate_aggregator(aggregator, &mut rng);

            let arg = FriedFalafels::get_arg(&aggregator.args, "number_local_epochs");
            // Increase at 2
            assert_eq!(arg.unwrap().value, "2");
        } else {
            panic!(
                "This test is supposed to get an aggregator from ./tests-files/fried-falafels.xml"
            );
        }
    }

    #[test]
    fn test_mutate_async_aggregator() {
        let content =
            String::from_utf8(fs::read("./tests-files/fried-falafels.xml").unwrap()).unwrap();
        let mut fried: FriedFalafels = quick_xml::de::from_str(&content).unwrap();

        let mut rng = StdRng::seed_from_u64(22);

        // Get an async aggregator
        let node = fried.clusters.get_mut(2).unwrap().nodes.get_mut(0).unwrap();

        if let NodeRole::Aggregator(aggregator) = node.role.borrow_mut() {
            let results = vec![
                "0.5", "0.5", "0.6", "0.5", "0.5", "0.4", "0.4", "0.5", "0.6", "0.7", "0.6"
            ];
            for res in results {
                FriedFalafels::mutate_aggregator(aggregator, &mut rng);

                let arg = FriedFalafels::get_arg(&aggregator.args, "proportion_threshold");
                assert_eq!(arg.unwrap().value, res);
            }
        } else {
            panic!(
                "This test is supposed to get an aggregator from ./tests-files/fried-falafels.xml"
            );
        }
    }
}
