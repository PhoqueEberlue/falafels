mod individual_factory;
mod launcher;
mod options;
mod structures;
mod study;
use clap::Parser;

use options::{Cli, Commands};
use study::{InputFiles, Study};

fn main() {
    let args = Cli::parse();

    let mut study = Study::new(
        args.simulation_name.clone(),
        format!("./records/{}", args.simulation_name.replace(" ", "_")),
        InputFiles {
            clusters_path: args.clusters_path,
            constants_path: args.constants_path,
            profiles_path: args.profiles_path,
            platform_specs: args.platform_specs,
        },
    );

    match args.command {
        Some(c) => {
            match c {
                // Study with varying machines number
                Commands::Varying {
                    step,
                    total_number_gen,
                } => {
                    study.varying_machines_number_sim(step, total_number_gen);
                }
                // Study with evolution algorithm
                Commands::Evolution { yes } => {
                    study.evolution_algorithm_sim();
                    unimplemented!();
                }
            }

            study.export_to_json();
            study.plot_results();
        }
        None => println!("Nothing to do, exiting."),
    }

    // Load a previous study
    // let study = Study::load_from_json("./records/varying_machines/study_obj.json");
    // study.plot_results();
}
