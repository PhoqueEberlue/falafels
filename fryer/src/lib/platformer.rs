use core::panic;
use std::fs;
use std::slice::Iter;

use crate::structures::common::Prop;
use crate::structures::fried;
use crate::structures::platform::{Host, Link, LinkContainer, Platform, Route, Router, Zone};
use crate::structures::raw;
use serde::Serialize;

pub struct Platformer<'a> {
    pub rf: &'a raw::RawFalafels,
    pub ff: &'a fried::FriedFalafels,
    link_count: u32,
}

impl<'a> Platformer<'a> {
    pub fn new(
        raw_falafels: &'a raw::RawFalafels,
        fried_falafels: &'a fried::FriedFalafels,
    ) -> Platformer<'a> {
        Platformer {
            rf: raw_falafels,
            ff: fried_falafels,
            link_count: 0,
        }
    }

    /// Create a simgrid platform with a star topology with our repective raw and fried
    /// configurations.
    ///
    /// /!\ Warning /!\
    /// For now, no matter how many clusters (nor their topology) impact the simgrid platform
    /// because we choosed that the "physical" network could be abstracted as single star network,
    /// and that the "logical" or "applicative" topology will be handled in the simulator as the
    /// implementation of the network algorithms.
    pub fn create_star_topology(&mut self) -> Platform {
        let number_nodes = self.ff.clusters.list.iter().map(|c| c.nodes.len()).sum(); // N
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

        for (raw_cluster, fried_cluster) in self.rf.clusters.list.iter().zip(&self.ff.clusters.list)
        {
            let (mut h, mut l, mut r) =
                self.create_cluster_components(raw_cluster, &fried_cluster, &router_name);
            hosts.append(&mut h);
            links.append(&mut l);
            routes.append(&mut r);
        }

        let zone = Zone {
            id: zone_name.to_string(),
            // Using shortest path algorithm
            routing: "Floyd".to_string(),
            hosts,
            links,
            routes,
            routers,
        };

        Platform {
            // Generating simgrid 4.1 platform file
            version: "4.1".to_string(),
            zone,
        }
    }

    fn create_cluster_components(
        &mut self,
        raw_cluster: &raw::Cluster,
        fried_cluster: &fried::Cluster,
        router_name: &String,
    ) -> (Vec<Host>, Vec<Link>, Vec<Route>) {
        let mut hosts = Vec::<Host>::new();
        let mut links = Vec::<Link>::new();
        let mut routes = Vec::<Route>::new();

        // Rework that later, this is absolutely ugly
        let empty_vector_h: Vec<raw::HostProfileRef> = vec![];
        let empty_vector_l: Vec<raw::LinkProfileRef> = vec![];

        // Trainers can be optional when defining a cluster with only "connected-to"
        let trainers_host_profiles = match &raw_cluster.trainers {
            Some(t) => &t.host_profiles,
            None => &empty_vector_h,
        };

        let trainers_link_profiles = match &raw_cluster.trainers {
            Some(t) => &t.link_profiles,
            None => &empty_vector_l,
        };

        let mut profile_handler_host = ProfilesHandler::new(
            &trainers_host_profiles,
            &raw_cluster.aggregators.host_profiles,
            &self.rf.profiles.host_profiles,
        );

        let mut profile_handler_link = ProfilesHandler::new(
            &trainers_link_profiles,
            &raw_cluster.aggregators.link_profiles,
            &self.rf.profiles.link_profiles,
        );

        for node in &fried_cluster.nodes {
            let (host_profile, link_profile) = (
                profile_handler_host.pick_profile(&node.role),
                profile_handler_link.pick_profile(&node.role),
            );

            // Creating host for the current node
            hosts.push(Platformer::create_host(node.name.clone(), host_profile));

            // Adding route that references the loopback link to the current node
            // routes.push(self.create_route(node.name.to_string(), node.name.to_string(), "loopback".to_string()));

            // Adding link and route to the router of the current zone
            links.push(self.create_link(link_profile));
            let link_id = self.link_count;
            routes.push(self.create_route(
                node.name.to_string(),
                router_name.clone(),
                link_id.to_string(),
            ));
        }

        (hosts, links, routes)
    }

    fn create_router(&self, name: String) -> Router {
        Router {
            id: name,
            coordinates: None,
        }
    }

    fn create_host(name: String, profile: &raw::HostProfile) -> Host {
        Host {
            id: name,
            speed: profile.speed.clone(),
            core: profile.core.clone(),
            pstate: profile.pstate.clone(),
            props: profile.props.clone(),
        }
    }

    fn create_loopback_link(&self) -> Link {
        // We do not need to increment link count because the loopback link is a special one with
        // its own name. In simgrid we can define the same loopback link for each host, which makes
        // the platform file easier.

        let prop_watt_range = Prop {
            id: "wattage_range".to_string(),
            value: "100.0:200.0".to_string(),
        };
        let prop_watt_off = Prop {
            id: "wattage_off".to_string(),
            value: "10".to_string(),
        };

        Link {
            id: "loopback".to_string(),
            bandwidth: "498MBps".to_string(),
            latency: "15us".to_string(),
            sharing_policy: Some("FATPIPE".to_string()),
            props: vec![prop_watt_range, prop_watt_off],
        }
    }

    fn create_link(&mut self, profile: &raw::LinkProfile) -> Link {
        // Increment link count
        self.link_count += 1;
        let link_id = self.link_count;

        Link {
            id: link_id.to_string(),
            bandwidth: profile.bandwidth.clone(),
            latency: profile.latency.clone(),
            sharing_policy: profile.sharing_policy.clone(),
            props: profile.props.clone(),
        }
    }

    fn create_route(&mut self, src: String, dst: String, link_id: String) -> Route {
        Route {
            src,
            dst,
            link_ctn: LinkContainer { id: link_id },
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

        let _ = platform
            .serialize(serializer)
            .map_err(|e| panic!("Error while serializing simgrid platform file: {e}"));

        let _ = fs::write(path, result_buffer)
            .map_err(|e| panic!("Couldn't write simgrid platform file: {e}"));
    }
}

/// Trait acting as an interface so we can return Profiles of different types and query their name.
trait ProfileTrait {
    fn get_pname(&self) -> &String;
}

impl ProfileTrait for raw::HostProfile {
    fn get_pname(&self) -> &String {
        &self.name
    }
}

impl ProfileTrait for raw::LinkProfile {
    fn get_pname(&self) -> &String {
        &self.name
    }
}

/// Trait acting as an interface so we can get names of different References
trait ProfileRefTrait {
    fn get_rname(&self) -> &String;
}

impl ProfileRefTrait for raw::HostProfileRef {
    fn get_rname(&self) -> &String {
        &self.name
    }
}

impl ProfileRefTrait for raw::LinkProfileRef {
    fn get_rname(&self) -> &String {
        &self.name
    }
}

/// A ProfilesHandler lets us nicely spread Profiles using cycle iterators.
/// Profiles must implement ProfileTrait so we can get them by their name.
/// ProfilesReference must implement ProfileRefTrait so we can get the name of the profile
struct ProfilesHandler<'a, P: ProfileTrait, R: ProfileRefTrait> {
    // Defining cycle iterators to evenly allocate the profiles
    profiles_iter_trainers: std::iter::Cycle<Iter<'a, R>>,
    profiles_iter_aggregators: std::iter::Cycle<Iter<'a, R>>,
    profiles_list: &'a Vec<P>,
}

impl<'a, P: ProfileTrait, R: ProfileRefTrait> ProfilesHandler<'a, P, R> {
    /// Creates a new ProfilesHandler given `profiles_trainers` and `profiles_aggregators`
    /// which corresponds to the profiles we want to give to our trainers and aggregators.
    /// Lastly `profiles_list` is a vector of profiles implementing ProfileTrait.
    fn new(
        profiles_trainers: &'a Vec<R>,
        profiles_aggregators: &'a Vec<R>,
        profiles_list: &'a Vec<P>,
    ) -> ProfilesHandler<'a, P, R> {
        // Create a cycle operator from the profiles vector.
        let profiles_iter_trainers = profiles_trainers.iter().cycle();
        let profiles_iter_aggregators = profiles_aggregators.iter().cycle();

        let res: ProfilesHandler<P, R> = ProfilesHandler::<P, R> {
            profiles_iter_trainers,
            profiles_iter_aggregators,
            profiles_list,
        };
        res
    }

    /// Get a profile by its `profile_name`
    fn get_profile(&self, profile_name: &str) -> &P {
        for profile in self.profiles_list {
            if profile_name == profile.get_pname() {
                return profile;
            }
        }
        panic!(
            "HostProfile named `{}` not found in raw-falafels file",
            profile_name
        );
    }

    /// Pick a profile depending on Node's Role
    fn pick_profile(&mut self, node_role: &fried::RoleEnum) -> &P {
        match node_role {
            fried::RoleEnum::Trainer(_) => {
                // Theoritically the cycle iterator never ends so its safe to unwrap.
                let profile_name = self.profiles_iter_trainers.next().unwrap();
                self.get_profile(profile_name.get_rname())
            }
            fried::RoleEnum::Aggregator(_) => {
                let profile_name = self.profiles_iter_aggregators.next().unwrap();
                self.get_profile(profile_name.get_rname())
            }
        }
    }
}
