use core::panic;
use std::fs;

use serde::Serialize;
use crate::structures::fried::{self, FriedFalafels};
use crate::structures::platform::{Host, Link, LinkContainer, Platform, Prop, Route, Zone };


pub struct Platformer {
    // pub config: Config
    link_count: u32,
}

impl Platformer {
    pub fn new() -> Platformer {
        Platformer { link_count: 0 }
    }

    pub fn write_platform(&self, path: &str, platform: &Platform) {
        println!("Writing the simgrid platform file to `{}`...", path);
        let mut result_buffer = String::from(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n"
        );

        let mut serializer = quick_xml::se::Serializer::new(&mut result_buffer);
        serializer.expand_empty_elements(false);
        serializer.indent(' ', 4);

        platform.serialize(serializer).unwrap();

        fs::write(path, result_buffer).unwrap();

    }

    pub fn create_star_topology(&mut self, fried_falafels: &FriedFalafels) -> Platform {
        let number_nodes = fried_falafels.nodes.list.len(); // N
        let mut aggregator_name: Option<&str> = None; 

        // Each node will be given one host, so we should define N hosts
        let mut hosts = Vec::<Host>::with_capacity(number_nodes);

        // In a star topology with N nodes we have N-1 links.
        // Adding the loopback link we have N links in total.
        let mut links = Vec::<Link>::with_capacity(number_nodes);

        // One route for each Host because of the loopback link + one route from each 
        // node to the aggregator = N + N - 1
        let mut routes = Vec::<Route>::with_capacity(number_nodes * 2 - 1);

        // Create one loopback link to be used for every host.
        links.push(self.create_loopback_link());

        for node in &fried_falafels.nodes.list {
            // Creating host for the current node
            hosts.push(
                self.create_host(node.name.to_string(), None, None)
            );

            // Adding route referencing loopback link to the current node
            routes.push(self.create_route(node.name.to_string(), node.name.to_string(), "loopback".to_string()));

            match &node.role {
                fried::RoleEnum::Aggregator(_) => { aggregator_name = Some(&node.name) }
                _ => {}
            }
        }

        let aggregator_name = match aggregator_name {
            Some(n) => n,
            None => panic!("No aggregator node have been found in the fried-falafels file.")
        };

        for node in &fried_falafels.nodes.list {
            // Only add route to the aggregator if its a Trainer
            if let fried::RoleEnum::Trainer(_) = node.role {
                links.push(self.create_link());
                let link_id = self.link_count;
                routes.push(self.create_route(node.name.to_string(), aggregator_name.to_string(), link_id.to_string()));
            }
        }

        let zone = Zone {
            id: "zone1".to_string(),
            routing: "Full".to_string(),
            hosts, links, routes
        };

        Platform {
            // Generating simgrid 4.1 platform file
            version: "4.1".to_string(),
            zone,
        }
    }


    fn create_host(&self, name: String, speed: Option<String>, core: Option<String>) -> Host {
        let speed = speed.unwrap_or(String::from("98.095Mf"));

        let prop_watt_state = Prop { id: "wattage_per_state".to_string(), value: "100.0:120.0:200.0".to_string() };
        let prop_watt_off = Prop { id: "wattage_off".to_string(), value: "10".to_string() };

        Host { id: name, speed, core, props: vec![prop_watt_state, prop_watt_off] }
    }

    fn create_loopback_link(&self) -> Link {
        // We do not need to increment link count because the loopback link is a special one with
        // its own name. In simgrid we can define the same loopback link for each host, which makes
        // the platform file easier.
        
        let prop_watt_range = Prop { id: "wattage_range".to_string(), value: "100.0:200.0".to_string() };
        let prop_watt_off = Prop { id: "wattage_off".to_string(), value: "10".to_string() };

        Link { 
            id: "loopback".to_string(), 
            bandwidth: "498MBps".to_string(), 
            latency: "15us".to_string(), 
            sharing_policy: Some("FATPIPE".to_string()),
            props: vec![prop_watt_range, prop_watt_off] 
        }
    }

    fn create_link(&mut self) -> Link {
        // Increment link count
        self.link_count += 1;
        let link_id = self.link_count;

        let prop_watt_range = Prop { id: "wattage_range".to_string(), value: "100.0:200.0".to_string() };
        let prop_watt_off = Prop { id: "wattage_off".to_string(), value: "10".to_string() };

        Link { 
            id: link_id.to_string(), 
            bandwidth: "11.618875MBps".to_string(), 
            latency: "189.98us".to_string(), 
            sharing_policy: None,
            props: vec![prop_watt_range, prop_watt_off] 
        }
    }

    fn create_route(&mut self, src: String, dst: String, link_id: String) -> Route {
        Route {
            src, dst,
            link_ctn: LinkContainer { id: link_id }

        }
    }
}
