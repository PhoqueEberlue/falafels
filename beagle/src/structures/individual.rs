use fryer::{
    fryer::Fryer,
    platformer::{Platformer, RawAndFried, SpecsAndProfiles},
    structures::{
        common::{AggregatorType, ClusterTopology, TrainerType},
        fried::FriedFalafels,
        platform::Platform,
        raw::RawFalafels,
    },
};

use crate::launcher::Outcome;

#[derive(Debug, Clone)]
pub struct Individual {
    pub meta: Metadata,
    pub content: Option<Content>,
}

#[derive(Debug, Clone)]
pub struct Metadata {
    // Individual name that describes its specifications (Topo / Algo)
    pub name: String,
    /// The type of algo / topology used
    pub category: String,

    // Dir path where we should save the files
    pub output_dir_path: String,

    pub topo: Vec<ClusterTopology>,
    pub agg_type: AggregatorType,
    // RawFalafels structure which helped creating the Fried one
    pub base_rf: RawFalafels,
    pub is_hierarchical: bool,
}

#[derive(Debug, Clone)]
pub struct Content {
    // Generation of the individual
    pub gen_nb: u32,
    // FriedFalafels structure
    pub ff: FriedFalafels,
    //
    pub previous_outcome: Option<Outcome>,
    /// Sometimes we might assign a specific platform to an individual
    pub platform: Option<Platform>,
}

impl Individual {
    pub fn new(
        base_rf: RawFalafels,
        topo: Vec<ClusterTopology>,
        agg_type: AggregatorType,
        name: String,
        output_dir_path: String,
    ) -> Individual {
        let mut ind = Individual {
            meta: Metadata {
                topo: topo.clone(),
                agg_type,
                name: name.clone(),
                base_rf,
                output_dir_path,
                category: name.clone(),
                is_hierarchical: topo.len() <= 1,
            },
            content: None,
        };

        ind.refresh_content();
        ind
    }

    pub fn refresh_content(&mut self) {
        if self.meta.is_hierarchical {
            self.init_content_normal();
        } else {
            self.init_content_hierarchical();
        }
    }

    pub fn init_content_normal(&mut self) {
        let main_cluster = self.meta.base_rf.clusters.get_mut(0).unwrap();

        // Tweak the parameters
        main_cluster.topology = self.meta.topo.get(0).unwrap().clone();
        main_cluster.aggregators.aggregator_type = self.meta.agg_type.clone();

        let mut fryer = Fryer::new(None);
        // Generate the FriedFalafels
        let ff = fryer.fry(&self.meta.base_rf);
        
        self.content = Some(Content {
            gen_nb: 0,
            ff,
            previous_outcome: None,
            platform: None,
        });

        // Regenerate links
        self.content.as_mut().unwrap().ff.link_hierarchical_aggregators();
        self.content.as_mut().unwrap().ff.add_booststrap_nodes();
    }

    pub fn init_content_hierarchical(&mut self) {
        // Start by cloning the base RawFalafels
        let mut rf = self.meta.base_rf.clone();

        let main_cluster = rf.clusters.remove(0);

        let mut nb_trainers = main_cluster.trainers.number;
        let nb_sub_clusters = self.meta.topo.len() as u16;

        // Minus as much aggregators as we have sub clusters
        nb_trainers -= nb_sub_clusters;

        let nb_trainers_per_sub_cluster = nb_trainers / nb_sub_clusters;

        // -------------------------- CREATING SUB CLUSTERS -------------------------- //
        // For each topology, instanciate its sub cluster
        for (_, topology) in self.meta.topo.iter().enumerate() {
            // Create the sub cluster based on the main one defined in input RawFalafels
            let mut sub_cluster = main_cluster.clone();
            sub_cluster.aggregators.aggregator_type = AggregatorType::Hierarchical;
            sub_cluster.topology = topology.clone();
            sub_cluster.trainers.number = nb_trainers_per_sub_cluster;

            // Set hierarchical aggregators host profile by copying the main cluster trainers profiles
            // and by keeping only the first profile.
            sub_cluster.aggregators.host_profiles = match &main_cluster.trainers.host_profiles {
                Some(profiles) => {
                    // take the first profile and clone it
                    Some(profiles.clone()[0..1].to_owned())
                }
                None => None,
            };

            // Same for the link profiles
            sub_cluster.aggregators.link_profiles = match &main_cluster.trainers.link_profiles {
                Some(profiles) => {
                    // take the first profile and clone it
                    Some(profiles.clone()[0..1].to_owned())
                }
                None => None,
            };

            // Same for the trainers but here we reverse the profiles to take into account that the sub cluster's
            // aggregator already took the first profile. Then we have a correct distribution.
            // TODO: this statement is false because if we have an even amount of subclusters the
            // distribution isn't respected anymore.
            // E.g: let we have two profiles A and B, The hierarchical aggregator takes A, then
            // the 2 (or any even number) first trainers from subclusters takes B.
            // In the end A have been picked one time, and B tow times. However in a star version,
            // A and B would have been correctly distributed. The problem is indeed reversed when
            // the number of cluster is odd. So this solution is not correct but it is ok...
            sub_cluster.trainers.host_profiles = match &main_cluster.trainers.host_profiles {
                Some(profiles) => {
                    // take the first profile and clone it
                    let mut res = profiles.clone();
                    res.reverse();
                    Some(res)
                }
                None => None,
            };

            sub_cluster.trainers.link_profiles = match &main_cluster.trainers.link_profiles {
                Some(profiles) => {
                    // take the first profile and clone it
                    let mut res = profiles.clone();
                    res.reverse();
                    Some(res)
                }
                None => None,
            };

            rf.clusters.push(sub_cluster);
        }

        // -------------------------- CREATING CENTRAL CLUSTER -------------------------- //
        // Use the main cluster as a base to build the central one
        let mut central_cluster = main_cluster;

        // Set number of trainers to 0
        central_cluster.trainers.number = 0;
        central_cluster.trainers.trainer_type = TrainerType::None;

        // Force Hierarchical topology for the central cluster
        central_cluster.topology = ClusterTopology::Hierarchical;

        // Set central aggregator type
        central_cluster.aggregators.aggregator_type = self.meta.agg_type.clone();

        // Push the central cluster
        rf.clusters.push(central_cluster);

        // Generate the FriedFalafels
        let mut fryer = Fryer::new(None);
        let ff = fryer.fry(&rf);

        self.content = Some(Content {
            gen_nb: 0,
            ff,
            previous_outcome: None,
            platform: None,
        });

        // Regenerate links
        self.content.as_mut().unwrap().ff.link_hierarchical_aggregators();
        self.content.as_mut().unwrap().ff.add_booststrap_nodes();
    }

    /// Writes the FriedFalafels `ff` at `ff_file_path`
    pub fn write_fried(&self) {
        Fryer::write_fried_falafels(&self.get_ff_path(), &self.content.as_ref().unwrap().ff);
    }

    pub fn gen_and_write_platform(&mut self) {
        match &self.meta.base_rf.platform_specs {
            // If specs defined, use them
            Some(specs) => {
                let mut platformer = Platformer::new(SpecsAndProfiles {
                    specs: &specs,
                    profiles: &self.meta.base_rf.profiles,
                });

                let platform = platformer.create_star_topology();

                platformer.write_platform(&self.get_platform_path(), &platform);

                self.content.as_mut().unwrap().platform = Some(platform);
            }
            None => {
                let mut platformer = Platformer::new(RawAndFried {
                    rf: &self.meta.base_rf,
                    ff: &self.content.as_ref().unwrap().ff,
                });

                let platform = platformer.create_star_topology();
                platformer.write_platform(&self.get_platform_path(), &platform);

                self.content.as_mut().unwrap().platform = Some(platform);
            }
        }
    }

    pub fn get_ff_path(&self) -> String {
        format!(
            "{}/fried/GEN-{}-{}.xml",
            self.meta.output_dir_path,
            self.content.as_ref().unwrap().gen_nb,
            self.meta.name
        )
    }

    pub fn get_platform_path(&self) -> String {
        format!(
            "{}/platform/GEN-{}-{}.xml",
            self.meta.output_dir_path,
            self.content.as_ref().unwrap().gen_nb,
            self.meta.name
        )
    }
}
