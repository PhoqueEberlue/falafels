use fryer::structures::{common::{AggregatorType, ClusterTopology}, fried::FriedFalafels, raw::{Cluster, ConnectedTo, RawFalafels}};
use fryer::fryer::Fryer;

#[derive(Debug, Clone)]
pub struct Individual {
    // Individual name that will be displayed in plots
    pub name: String,
    // RawFalafels structure which helped creating the Fried one
    pub rf: RawFalafels,
    // FriedFalafels structure
    pub ff: FriedFalafels,
    // FriedFalafels file path used to run the simulation of the Individual
    pub ff_path: String,
}

impl Individual {
    pub fn increment_trainers_number(&mut self, number: u16) {
        let nb_clusters = self.rf.clusters.list.len();
        // Case not hierarchical
        if nb_clusters == 1 {
            self.rf.clusters.list.get_mut(0).unwrap().trainers.as_mut().unwrap().number += number;
        }
        else {
            let nb_sub_clusters = (nb_clusters - 1) as u16;

            for cluster in self.rf.clusters.list.iter_mut() {
                // Equally distribute new machines to the sub clusters
                if let Some(trainers) = &mut cluster.trainers {
                    trainers.number = number / nb_sub_clusters;
                }
            }
        }
    }
}

pub struct IndividualFactory {
    pub fryer: Fryer,
    // Used as a base to generate mulitple ff files
    pub base_rf: RawFalafels,
    pub generation_number: u32,
    pub output_dir: String, 
}

impl IndividualFactory {
    pub fn new(rf_path: &str, output_dir: &str) -> IndividualFactory {
        let mut fryer = Fryer::new(None);
        let base_rf = fryer.load_raw_falafels(rf_path);

        IndividualFactory { fryer, base_rf, generation_number: 0, output_dir: output_dir.to_string() }
    }

    pub fn init_individuals(&mut self) -> Vec<Individual> {
        let mut individuals: Vec<Individual> = vec![];

        individuals.push(
            self.init_individual(
                ClusterTopology::Star, AggregatorType::Simple, "StarSimple".to_string()
            )
        );

        individuals.push(
            self.init_individual(
                ClusterTopology::Star, AggregatorType::Asynchronous, "StarAsynchronous".to_string()
            )
        );

        individuals.push(
            self.init_individual(
                ClusterTopology::Ring, AggregatorType::Simple, "RingSimple".to_string()
            )
        );

        individuals.push(
            self.init_individual(
                ClusterTopology::Ring, AggregatorType::Asynchronous, "RingAsynchronous".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::Star, ClusterTopology::Star], 
                "StarStarHierarchical".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::Ring, ClusterTopology::Star], 
                "RingStarHierarchical".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::Ring, ClusterTopology::Ring], 
                "RingRingHierarchical".to_string()
            )
        );

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

        let main_cluster = rf.clusters.list.get_mut(0).unwrap();

        // Tweak the parameters
        main_cluster.topology = topo;
        main_cluster.aggregators.aggregator_type = agg_type;

        // Generate the FriedFalafels
        let ff = self.fryer.fry(&rf);

        // Write it
        let ff_path = format!("{}/fried/GEN-{}-{}.xml", self.output_dir, self.generation_number, name);
        self.fryer.write_fried_falafels(&ff_path, &ff);

        Individual { name, rf, ff, ff_path }
    }

    fn init_hierarchical_individual(
        &mut self,
        topologies: Vec<ClusterTopology>,
        name: String,
    ) -> Individual {
        // Start by cloning the base RawFalafels
        let mut rf = self.base_rf.clone();

        let main_cluster = rf.clusters.list.pop().unwrap();

        let mut nb_trainers = main_cluster.trainers.as_ref().unwrap().number;
        let nb_sub_clusters = topologies.len() as u16;

        // Minus as much aggregators as we have sub clusters
        nb_trainers -= nb_sub_clusters;

        let nb_trainers_per_sub_cluster = nb_trainers / nb_sub_clusters;

        let mut sub_custers_names = vec![];

        // For each topology, instanciate its sub cluster
        for (i, topology) in topologies.iter().enumerate() {
            // Create the sub cluster based on the main one defined in input RawFalafels
            let mut sub_cluster = main_cluster.clone();
            sub_cluster.aggregators.aggregator_type = AggregatorType::Hierarchical;
            sub_cluster.topology = topology.clone();
            sub_cluster.trainers.as_mut().unwrap().number = nb_trainers_per_sub_cluster;

            // Set aggregators host and link profiles by copying the main cluster trainers profiles
            sub_cluster.aggregators.host_profiles = main_cluster.trainers.as_ref().unwrap().host_profiles.clone();
            sub_cluster.aggregators.link_profiles = main_cluster.trainers.as_ref().unwrap().link_profiles.clone();

            // Same for the trainers
            sub_cluster.trainers.as_mut().unwrap().host_profiles = main_cluster.trainers.as_ref().unwrap().host_profiles.clone();
            sub_cluster.trainers.as_mut().unwrap().link_profiles = main_cluster.trainers.as_ref().unwrap().link_profiles.clone();

            // But here we reverse the profiles to take into account that the sub cluster's
            // aggregator already took the first profile. Then we have a correct distribution.
            sub_cluster.trainers.as_mut().unwrap().host_profiles.reverse();
            sub_cluster.trainers.as_mut().unwrap().link_profiles.reverse();

            // Rename the cluster to be able to identify it in the ConnectedTo element
            sub_cluster.name = format!("sub-cluster-{i}");

            sub_custers_names.push(sub_cluster.name.clone());

            rf.clusters.list.push(sub_cluster);
        }

        // Use the main cluster as a base to build the central one
        let mut central_cluster = main_cluster;

        // Delete trainers
        central_cluster.trainers = None;

        // Force star topology
        central_cluster.topology = ClusterTopology::Star;

        // And create a connection to each sub_cluster to add it to the central cluster
        central_cluster.connections = Some(
            sub_custers_names.iter().map(
                    |name| ConnectedTo { cluster_name: name.to_string() }
            ).collect()
        );

        // Push the central cluster
        rf.clusters.list.push(central_cluster);

        // Generate the FriedFalafels
        let ff = self.fryer.fry(&rf);

        // Write it
        let ff_path = format!("{}/fried/GEN-{}-{}.xml", self.output_dir, self.generation_number, name);
        self.fryer.write_fried_falafels(&ff_path, &ff);

        Individual { name, rf, ff, ff_path }
    }
}
