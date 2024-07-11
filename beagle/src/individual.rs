use fryer::structures::{common::{AggregatorType, ClusterTopology, TrainerType}, fried::FriedFalafels, raw::{Cluster, RawFalafels}};
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
    // 
    pub is_hierarchical: bool,
}

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
                ClusterTopology::RingUni, AggregatorType::Simple, "RingUniSimple".to_string()
            )
        );

        individuals.push(
            self.init_individual(
                ClusterTopology::RingUni, AggregatorType::Asynchronous, "RingUniAsynchronous".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::Star, ClusterTopology::Star], 
                AggregatorType::Simple,
                "StarStarHierarchical".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::RingUni, ClusterTopology::RingUni], 
                AggregatorType::Simple,
                "RingUniRingUniHierarchical".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::Star, ClusterTopology::Star], 
                AggregatorType::Asynchronous,
                "StarStarHierarchicalAsync".to_string()
            )
        );

        individuals.push(
            self.init_hierarchical_individual(
                vec![ClusterTopology::RingUni, ClusterTopology::RingUni], 
                AggregatorType::Asynchronous,
                "RingUniRingUniHierarchicalAsync".to_string()
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

        let main_cluster = rf.clusters.get_mut(0).unwrap();

        // Tweak the parameters
        main_cluster.topology = topo;
        main_cluster.aggregators.aggregator_type = agg_type;

        // Generate the FriedFalafels
        let ff = self.fryer.fry(&rf);

        // Write it
        let ff_path = format!("{}/fried/GEN-{}-{}.xml", self.output_dir, self.generation_number, name);
        self.fryer.write_fried_falafels(&ff_path, &ff);

        Individual { name, rf, ff, ff_path, is_hierarchical: false }
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
        for (i, topology) in topologies.iter().enumerate() {
            // Create the sub cluster based on the main one defined in input RawFalafels
            let mut sub_cluster = main_cluster.clone();
            sub_cluster.aggregators.aggregator_type = AggregatorType::Hierarchical;
            sub_cluster.topology = topology.clone();
            sub_cluster.trainers.number = nb_trainers_per_sub_cluster;

            // Set hierarchical aggregators host and link profiles by copying the main cluster trainers profiles
            // and by keeping only the first profiles.
            let hp = vec![main_cluster.trainers.host_profiles.clone().first().unwrap().clone()];
            let lp = vec![main_cluster.trainers.link_profiles.clone().first().unwrap().clone()];
            sub_cluster.aggregators.host_profiles = hp;
            sub_cluster.aggregators.link_profiles = lp;

            // Same for the trainers
            sub_cluster.trainers.host_profiles = main_cluster.trainers.host_profiles.clone();
            sub_cluster.trainers.link_profiles = main_cluster.trainers.link_profiles.clone();

            // But here we reverse the profiles to take into account that the sub cluster's
            // aggregator already took the first profile. Then we have a correct distribution.
            sub_cluster.trainers.host_profiles.reverse();
            sub_cluster.trainers.link_profiles.reverse();

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
        let ff_path = format!("{}/fried/GEN-{}-{}.xml", self.output_dir, self.generation_number, name);
        self.fryer.write_fried_falafels(&ff_path, &ff);

        Individual { name, rf, ff, ff_path, is_hierarchical: true }
    }
}
