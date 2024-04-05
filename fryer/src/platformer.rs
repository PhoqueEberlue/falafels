use core::panic;
use std::fs;
use std::str::Split;

use serde::Serialize;
use crate::structures::fried::{FriedFalafels, RoleEnum};
use crate::structures::raw::{HostProfile, LinkProfile, RawFalafels};
use crate::structures::platform::{Host, Link, LinkContainer, Platform, Route, Router, Zone};
use crate::structures::common::Prop;

pub struct Platformer<'a> {
    pub rf: &'a RawFalafels,
    pub ff: &'a FriedFalafels,
    link_count: u32,

    // Defining cycle iterators for allocating evenly the profiles
    trainer_profile_iter: std::iter::Cycle<Split<'a, &'a str>>,
    aggregator_profile_iter: std::iter::Cycle<Split<'a, &'a str>>,
}

impl<'a> Platformer<'a> {
    pub fn new(raw_falafels: &'a RawFalafels, fried_falafels: &'a FriedFalafels) -> Platformer<'a> {

        // Create our profile iterators by spliting the "profiles" field by the comma symbol
        let tpi = raw_falafels.trainers.host_profiles.split(",").cycle();
        let api = raw_falafels.aggregators.host_profiles.split(",").cycle();

        Platformer { 
            rf: raw_falafels, ff: fried_falafels, link_count: 0, 
            trainer_profile_iter: tpi, aggregator_profile_iter: api,
        }
    }

    pub fn write_platform(&self, path: &str, platform: &Platform) {
        println!("Writing the simgrid platform file to `{}`...", path);
        let mut result_buffer = String::from(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<!DOCTYPE platform SYSTEM \"https://simgrid.org/simgrid.dtd\">\n"
        );

        let mut serializer = quick_xml::se::Serializer::new(&mut result_buffer);
        serializer.expand_empty_elements(false);
        serializer.indent(' ', 4);

        let _ = platform.serialize(serializer)
            .map_err(|e| panic!("Error while serializing simgrid platform file: {e}"));

        let _ = fs::write(path, result_buffer)
            .map_err(|e| panic!("Couldn't write simgrid platform file: {e}"));

    }

    pub fn create_star_topology(&mut self) -> Platform {
        let number_nodes = self.ff.nodes.list.len(); // N
        let zone_name = "zone1";
        let router_name = format!("{}-router", zone_name);

        // Each node will be given one host, so we should define N hosts
        let mut hosts = Vec::<Host>::with_capacity(number_nodes);

        // In a star topology with N nodes we have N-1 links.
        // Adding the loopback link we have N links in total.
        let mut links = Vec::<Link>::with_capacity(number_nodes);

        // One route for each Host because of the loopback link + one route from each 
        // node to the aggregator = N + N - 1
        let mut routes = Vec::<Route>::with_capacity(number_nodes * 2 - 1);

        // Only one router in a star architecture
        let mut routers = Vec::<Router>::with_capacity(1);

        routers.push(self.create_router(router_name.clone()));

        // Create one loopback link to be used for every host.
        // links.push(self.create_loopback_link());

        for node in &self.ff.nodes.list {

            let profile = self.pick_profile(&node.role);
            // Creating host for the current node
            hosts.push(
                Platformer::create_host(node.name.clone(), profile)
            ); 

            // Adding route that references the loopback link to the current node
            // routes.push(self.create_route(node.name.to_string(), node.name.to_string(), "loopback".to_string()));

            // Adding link and route to the router of the current zone
            links.push(self.create_link());
            let link_id = self.link_count;
            routes.push(
                self.create_route(node.name.to_string(), 
                router_name.clone(), 
                link_id.to_string())
            );
        }

        let zone = Zone {
            id: zone_name.to_string(),
            // Using shortest path algorithm
            routing: "Floyd".to_string(),
            hosts, links, routes, routers
        };

        Platform {
            // Generating simgrid 4.1 platform file
            version: "4.1".to_string(),
            zone,
        }
    }

    /// Get a profile by its name
    fn get_profile(&self, profile_name: &str) -> &HostProfile {
        for profile in &self.rf.profiles.host_profiles {
            if profile.name == profile_name {
                return &profile;
            }
        }
        panic!("HostProfile named `{}` not found in raw-falafels file", profile_name);
    }

    /// Pick a profile depending on the
    fn pick_profile(&mut self, role: &RoleEnum) -> &HostProfile { 
        match role {
            RoleEnum::Trainer(_) => {
                let profile_name = self.trainer_profile_iter.next().unwrap();
                self.get_profile(profile_name)
            }
            RoleEnum::Aggregator(_) => {
                let profile_name = self.aggregator_profile_iter.next().unwrap();
                self.get_profile(profile_name)
            }
        }
    }

    fn create_router(&self, name: String) -> Router {
        Router {
            id: name,
            coordinates: None
        }
    }

    fn create_host(name: String, profile: &HostProfile) -> Host {
        Host { 
            id: name, 
            speed: profile.speed.clone(), 
            core: profile.core.clone(), 
            pstate: profile.pstate.clone(),
            props: profile.props.clone()
        }
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
