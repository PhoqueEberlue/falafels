mod environment;
mod individual;
mod launcher;
mod structures;
mod options;
mod study;
use clap::Parser;

use options::{Cli, Commands};
use study::Study;

fn main() {
    let args = Cli::parse();

    // Study with varying machines number
    let mut study = Study::new(
        args.simulation_name.clone(),
        format!("./records/{}", args.simulation_name.replace(" ", "_"))
    );

    match args.command {
        Some(c) => {
            match c {
                Commands::Varying { clusters_path, step, total_number_gen } => {
                    // Create two Rawfalafels structures, one to be the base for "normal"
                    // topologies, one for hierarchical topologies
                    let base_rf = Study::recompose_rf(
                        &clusters_path, &args.constants_path, &args.profiles_path).unwrap();

                    study.varying_machines_number_sim(base_rf, step, total_number_gen);
                },
                Commands::Evolution { evolution_config_path } => {
                    unimplemented!();
                }
            }

            study.export_to_json();
            study.plot_results();
        },
        None => println!("Nothing to do, exiting.")
    }

    // Load a previous study
    // let study = Study::load_from_json("./records/varying_machines/study_obj.json");
    // study.plot_results();
}
