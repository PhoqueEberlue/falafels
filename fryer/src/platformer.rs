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
}

impl<'a> Platformer<'a> {
    pub fn new(raw_falafels: &'a RawFalafels, fried_falafels: &'a FriedFalafels) -> Platformer<'a> {
        Platformer { 
            rf: raw_falafels, ff: fried_falafels, link_count: 0, 
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
        
        let mut profile_handler_host = ProfilesHandler::new(
            &self.rf.trainers.host_profiles, 
            &self.rf.aggregators.host_profiles, 
            &self.rf.profiles.host_profiles
        );

        let mut profile_handler_link = ProfilesHandler::new(
            &self.rf.trainers.link_profiles, 
            &self.rf.aggregators.link_profiles, 
            &self.rf.profiles.link_profiles
        );

        for node in &self.ff.nodes.list {

            let (host_profile, link_profile) = (
                profile_handler_host.pick_profile(&node.role),
                profile_handler_link.pick_profile(&node.role)
            );
                
            // Creating host for the current node
            hosts.push(
                Platformer::create_host(node.name.clone(), host_profile)
            ); 

            // Adding route that references the loopback link to the current node
            // routes.push(self.create_route(node.name.to_string(), node.name.to_string(), "loopback".to_string()));

            // Adding link and route to the router of the current zone
            links.push(self.create_link(link_profile));
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

    fn create_link(&mut self, profile: &LinkProfile) -> Link {
        // Increment link count
        self.link_count += 1;
        let link_id = self.link_count;

        Link { 
            id: link_id.to_string(), 
            bandwidth: profile.bandwidth.clone(),
            latency: profile.latency.clone(), 
            sharing_policy: profile.sharing_policy.clone(),
            props: profile.props.clone()
        }
    }

    fn create_route(&mut self, src: String, dst: String, link_id: String) -> Route {
        Route {
            src, dst,
            link_ctn: LinkContainer { id: link_id }

        }
    }
}

pub trait HasProfileName {
    fn get_name(&self) -> &String;
}

impl HasProfileName for HostProfile {
    fn get_name(&self) -> &String {
        &self.name
    }
}

impl HasProfileName for LinkProfile {
    fn get_name(&self) -> &String {
        &self.name
    }
}

struct ProfilesHandler<'a, T: HasProfileName> {
    // Defining cycle iterators to evenly allocate the profiles
    profiles_iter_trainers: std::iter::Cycle<Split<'a, &'a str>>,
    profiles_iter_aggregators: std::iter::Cycle<Split<'a, &'a str>>,
    profiles_list: &'a Vec<T>,
}

impl<'a, T: HasProfileName> ProfilesHandler<'a, T> {
    fn new(profile_str_trainers: &'a str, profile_str_aggregators: &'a str, profiles_list: &'a Vec<T>) -> ProfilesHandler<'a, T> {
        // Create our profile iterators by spliting the "profiles" field by the comma symbol
        let profiles_iter_trainers = profile_str_trainers.split(",").cycle();
        let profiles_iter_aggregators = profile_str_aggregators.split(",").cycle();

        let res: ProfilesHandler<T> = ProfilesHandler { 
            profiles_iter_trainers, profiles_iter_aggregators, profiles_list
        };
        res
    }

    /// Get a profile by its name
    fn get_profile(&self, profile_name: &str) -> &T {
        for profile in self.profiles_list {
            if profile_name == profile.get_name() {
                return profile;
            }
        }
        panic!("HostProfile named `{}` not found in raw-falafels file", profile_name);
    }

    /// Pick a profile depending on Node's Role
    fn pick_profile(&mut self, node_role: &RoleEnum) -> &T { 
        match node_role {
            RoleEnum::Trainer(_) => {
                let profile_name = self.profiles_iter_trainers.next().unwrap();
                self.get_profile(profile_name)
            }
            RoleEnum::Aggregator(_) => {
                let profile_name = self.profiles_iter_aggregators.next().unwrap();
                self.get_profile(profile_name)
            }
        }
    }
}
