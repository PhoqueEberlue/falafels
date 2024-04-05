use crate::structures::{common, fried, raw};
use core::panic;
use quick_xml::de::from_str;
use serde::Serialize;
use std::fs;
use rand::{self, Rng};

pub struct Fryer {
    names_list: Option<NamesList>,
}

impl Fryer {
    /// Create a new Fryer, Optionnaly pass a file path containing names to be used for node naming
    pub fn new(optional_list_path: Option<&str>) -> Fryer {
        let mut fryer = Fryer {
            names_list: None
        }; 

        // If a list path have been provided
        if let Some(list_path) = optional_list_path {
            // Create a NamesList structure
            fryer.names_list = Some(NamesList::new(list_path));
        }

        fryer
    }

    pub fn load_raw_falafels(&mut self, file_path: &str) -> raw::RawFalafels {
        println!("Loading raw falafels at `{}`... 📰", file_path);

        let content = String::from_utf8(fs::read(file_path).unwrap()).unwrap();

        match from_str(&content) {
            Ok(raw_falafels) => raw_falafels,
            Err(e) => panic!("Error while parsing raw falafels file: {}", e),
        }
    } 

    pub fn fry(&mut self, rf: &raw::RawFalafels) -> fried::FriedFalafels {
        println!("Frying those falafels... 🧑🍳");

        if let Some(names_list) = &self.names_list {
            // Total number of nodes
            let total_nb_nodes = rf.trainers.number + rf.aggregators.number;

            // Panic if the list is too short
            names_list.is_list_big_enough(total_nb_nodes);
        }

        // Create FriedFalafels
        let mut ff = fried::FriedFalafels {
            // Setting version, cloning constants (because no processing needed), creating list of node
            version: "0.1".to_string(),
            constants: rf.constants.clone(),
            nodes: fried::Nodes { list: vec![] },
        }; 

        // Creating the trainers(s)
        for i in 0..rf.trainers.number {
            let role = fried::RoleEnum::Trainer(fried::Trainer {
                trainer_type: rf.trainers.trainer_type.clone(),
                args: rf.trainers.args.clone(),
            });

            let node = fried::Node {
                name: if let Some(l) = &mut self.names_list { l.pick_name() } else { format!("Node {}", i) },
                role,
                network_manager: rf.trainers.network_manager.clone(),
            };

            ff.nodes.list.push(node);
        }

        // Creating the aggregator(s)
        for i in 0..rf.aggregators.number {
            let role = fried::RoleEnum::Aggregator(fried::Aggregator {
                aggregator_type: rf.aggregators.aggregator_type.clone(),
                args: rf.aggregators.args.clone(),
            });

            let mut network_manager = rf.aggregators.network_manager.clone();

            // Inserts a new array into the Option if its value is None, then returns a mutable
            // reference to the contained value.
            let args = network_manager.args.get_or_insert_with(|| Vec::<common::Arg>::new());
                        
            // For each trainer node, append its name to the current aggregator
            for node in &ff.nodes.list {
                args.push(common::Arg { name: "bootstrap-node".to_string(), value: node.name.clone() });
            }

            let aggregator_node = fried::Node {
                name: if let Some(l) = &mut self.names_list { l.pick_name() } else { format!("Node {}", i) },
                role,
                network_manager,
            };

            ff.nodes.list.push(aggregator_node);
        }

        println!("Falafels ready! 🧆");

        ff
    }

    pub fn write_fried_falafels(&self, path: &str, ff: &fried::FriedFalafels) {
        println!("Writing the fried falafels to `{}`...", path);
        let mut result_buffer = String::from("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");

        let mut serializer = quick_xml::se::Serializer::new(&mut result_buffer);
        serializer.expand_empty_elements(false);
        serializer.indent(' ', 4);

        ff.serialize(serializer).unwrap();

        fs::write(path, result_buffer).unwrap();
    }
}

struct NamesList {
    list_of_names: Vec<String>, 
}

impl NamesList {
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

    fn is_list_big_enough(&self, total_nb_nodes: u8) {
        // Verifying we have enough names in the file to name each node
        if self.list_of_names.len() < total_nb_nodes.into() {
            panic!("It seems that the provided list of names does not contain enough names ({}) for the total number of nodes ({})", self.list_of_names.len(), total_nb_nodes);
        }
    }

    /// Randomly pick a name for a node and remove it from the list to prevent it from being picked
    /// again.
    fn pick_name(&mut self) -> String {
        let mut generator = rand::thread_rng();

        let remove_index = generator.gen_range(0..self.list_of_names.len() - 1);

        // Remove the name at remove_index from the list and return it
        self.list_of_names.remove(remove_index)
    }
}
