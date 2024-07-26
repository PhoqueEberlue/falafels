use fryer::fryer::Fryer;
use fryer::structures::{
    common::{AggregatorType, ClusterTopology},
    raw::RawFalafels,
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

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::Star],
            AggregatorType::Simple,
            "StarSimple".to_string(),
            self.output_dir.clone(),
        ));

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::Star],
            AggregatorType::Asynchronous,
            "StarAsynchronous".to_string(),
            self.output_dir.clone(),
        ));

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::RingUni],
            AggregatorType::Simple,
            "RingUniSimple".to_string(),
            self.output_dir.clone(),
        ));

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::RingUni],
            AggregatorType::Asynchronous,
            "RingUniAsynchronous".to_string(),
            self.output_dir.clone(),
        ));

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::Star, ClusterTopology::Star],
            AggregatorType::Simple,
            "StarStarHierarchical".to_string(),
            self.output_dir.clone(),
        ));
 
        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::Star, ClusterTopology::Star],
            AggregatorType::Asynchronous,
            "StarStarHierarchicalAsync".to_string(),
            self.output_dir.clone(),
        ));

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::RingUni, ClusterTopology::RingUni],
            AggregatorType::Simple,
            "RingUniRingUniHierarchical".to_string(),
            self.output_dir.clone(),
        ));

        individuals.push(Individual::new(
            self.base_rf.clone(),
            vec![ClusterTopology::RingUni, ClusterTopology::RingUni],
            AggregatorType::Asynchronous,
            "RingUniRingUniHierarchicalAsync".to_string(),
            self.output_dir.clone(),
        ));

        individuals
    }
}
