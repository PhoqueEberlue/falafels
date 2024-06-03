use std::thread;

use environment::Environment;
use fryer::{fryer::Fryer, platformer::Platformer, structures::{common::{AggregatorType, ClusterTopology}, raw::RawFalafels}};
use individual::Individual;

mod environment;
mod individual;
mod launcher;

fn main() {
    let mut factory = IndividualFactory::new("../xml/raw-falafels.xml");

    
    for gen_nb in 0..10 { 
        factory.generation_number = gen_nb;

        let main_cluster = factory.base_rf.clusters.list.get_mut(0).unwrap();
        main_cluster.trainers.as_mut().unwrap().number += 10;

        let individuals = factory.init_individuals();

        // we generate a common platform with any fried file we want because they will all generate the
        // same platform
        let mut platformer = Platformer::new(&factory.base_rf, &individuals.get(0).unwrap().ff);

        let platform = platformer.create_star_topology();
        
        let platform_path = format!("../xml/platform/GEN-{gen_nb}-simgrid-platform.xml");

        platformer.write_platform(&platform_path, &platform);

        let environment = Environment { platform, platform_path };

        let mut handles = vec![];

        for ind in individuals {
            let platform_path = environment.platform_path.clone();

            handles.push(thread::spawn(move || {
                let outcome = launcher::run_simulation(gen_nb, &ind, platform_path);
                println!("{:?}", outcome);
                outcome
            }));
        }

        for handle in handles {
            let outcome = handle.join().unwrap();
        }
    }
}

pub struct IndividualFactory {
    pub fryer: Fryer,
    // Used as a base to generate mulitple ff files
    pub base_rf: RawFalafels,
    pub generation_number: u32,
}

impl IndividualFactory {
    pub fn new(rf_path: &str) -> IndividualFactory {
        let mut fryer = Fryer::new(None);
        let base_rf = fryer.load_raw_falafels(rf_path);

        IndividualFactory { fryer, base_rf, generation_number: 0 }
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

        individuals
    }

    fn init_individual(
        &mut self,
        topo: ClusterTopology, 
        agg_type: AggregatorType,
        name: String,
    ) -> Individual {

        let main_cluster = self.base_rf.clusters.list.get_mut(0).unwrap();

        main_cluster.topology = topo;
        main_cluster.aggregators.aggregator_type = agg_type;

        let ff = self.fryer.fry(&self.base_rf);
        let ff_path = format!("../xml/fried/GEN-{}-{}.xml", self.generation_number, name);

        self.fryer.write_fried_falafels(&ff_path, &ff);

        Individual { name, ff, ff_path }
    }
}
