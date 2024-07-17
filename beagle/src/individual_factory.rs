use fryer::fryer::Fryer;
use fryer::structures::{
    common::{AggregatorType, ClusterTopology, TrainerType},
    fried::FriedFalafels,
    raw::{Cluster, RawFalafels},
};

use crate::structures::individual::Individual;

pub struct IndividualFactory {
    pub fryer: Fryer,
    // Used as a base to generate mulitple ff files
    pub base_rf: RawFalafels,
    pub generation_number: u32,
    pub output_dir: String,
}

impl IndividualFactory {
    pub fn new(base_rf: RawFalafels, output_dir: &str) -> IndividualFactory {
        // Create a fryer without any name list so we can name nodes with numbers
        let fryer = Fryer::new(None);

        IndividualFactory {
            fryer,
            base_rf,
            generation_number: 0,
            output_dir: output_dir.to_string(),
        }
    }

    pub fn init_individuals(&mut self) -> Vec<Individual> {
        let mut individuals: Vec<Individual> = vec![];

        individuals.push(self.init_individual(
            ClusterTopology::Star,
            AggregatorType::Simple,
            "StarSimple".to_string(),
        ));

        individuals.push(self.init_individual(
            ClusterTopology::Star,
            AggregatorType::Asynchronous,
            "StarAsynchronous".to_string(),
        ));

        individuals.push(self.init_individual(
            ClusterTopology::RingUni,
            AggregatorType::Simple,
            "RingUniSimple".to_string(),
        ));

        individuals.push(self.init_individual(
            ClusterTopology::RingUni,
            AggregatorType::Asynchronous,
            "RingUniAsynchronous".to_string(),
        ));

        individuals.push(self.init_hierarchical_individual(
            vec![ClusterTopology::Star, ClusterTopology::Star],
            AggregatorType::Simple,
            "StarStarHierarchical".to_string(),
        ));

        individuals.push(self.init_hierarchical_individual(
            vec![ClusterTopology::RingUni, ClusterTopology::RingUni],
            AggregatorType::Simple,
            "RingUniRingUniHierarchical".to_string(),
        ));

        individuals.push(self.init_hierarchical_individual(
            vec![ClusterTopology::Star, ClusterTopology::Star],
            AggregatorType::Asynchronous,
            "StarStarHierarchicalAsync".to_string(),
        ));

        individuals.push(self.init_hierarchical_individual(
            vec![ClusterTopology::RingUni, ClusterTopology::RingUni],
            AggregatorType::Asynchronous,
            "RingUniRingUniHierarchicalAsync".to_string(),
        ));

        individuals
    }

    fn init_individual(
        &mut self,
        topo: ClusterTopology,
        agg_type: AggregatorType,
        name: String,
    ) -> Individual {
        // Start by cloning the base RawFalafels
        let mut rf = self.base_rf.clone();

        let main_cluster = rf.clusters.get_mut(0).unwrap();

        // Tweak the parameters
        main_cluster.topology = topo;
        main_cluster.aggregators.aggregator_type = agg_type;

        // Generate the FriedFalafels
        let ff = self.fryer.fry(&rf);

        // Write it
        let ff_path = format!(
            "{}/fried/GEN-{}-{}.xml",
            self.output_dir, self.generation_number, name
        );

        Individual {
            name,
            rf,
            ff,
            ff_path,
            is_hierarchical: false,
        }
    }

    fn init_hierarchical_individual(
        &mut self,
        topologies: Vec<ClusterTopology>,
        agg_type: AggregatorType,
        name: String,
    ) -> Individual {
        // Start by cloning the base RawFalafels
        let mut rf = self.base_rf.clone();

        let main_cluster = rf.clusters.pop().unwrap();

        let mut nb_trainers = main_cluster.trainers.number;
        let nb_sub_clusters = topologies.len() as u16;

        // Minus as much aggregators as we have sub clusters
        nb_trainers -= nb_sub_clusters;

        let nb_trainers_per_sub_cluster = nb_trainers / nb_sub_clusters;

        // -------------------------- CREATING SUB CLUSTERS -------------------------- //
        // For each topology, instanciate its sub cluster
        for (_, topology) in topologies.iter().enumerate() {
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
        central_cluster.aggregators.aggregator_type = agg_type;

        // Push the central cluster
        rf.clusters.push(central_cluster);

        // Generate the FriedFalafels
        let ff = self.fryer.fry(&rf);

        // Write it
        let ff_path = format!(
            "{}/fried/GEN-{}-{}.xml",
            self.output_dir, self.generation_number, name
        );

        Individual {
            name,
            rf,
            ff,
            ff_path,
            is_hierarchical: true,
        }
    }
}
