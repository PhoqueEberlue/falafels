mod environment;
mod individual;
mod launcher;
mod structures;
mod options;
mod study;
use clap::Parser;

use options::{Cli, Commands};
use study::{InputForVarying, Study};

fn main() {
    let args = Cli::parse();

    // Study with varying machines number
    let mut study;

    match args.command {
        Some(c) => {
            match c {
                Commands::Varying { clusters_path, step, total_number_gen } => {
                    // Create two Rawfalafels structures, one to be the base for "normal"
                    // topologies, one for hierarchical topologies
                    study = Study::new(
                        args.simulation_name.clone(),
                        format!("./records/{}", args.simulation_name.replace(" ", "_")),
                        InputForVarying { clusters_path, constants_path: args.constants_path, profiles_path: args.profiles_path }
                    );

                    study.varying_machines_number_sim(step, total_number_gen);
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
