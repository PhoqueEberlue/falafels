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

    match args.command {
        Some(c) => {
            match c {
                // Study with varying machines number
                Commands::Varying {
                    step,
                    total_number_gen,
                } => {

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

                    study.varying_machines_number_sim(step, total_number_gen);
                    study.export_to_json();
                    study.plot_results_varying();
                }
                // Study with evolution algorithm
                Commands::Evolution { total_number_gen } => {
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

                    study.evolution_algorithm_sim(total_number_gen);
                    study.export_to_json();
                }
                Commands::LoadPreviousStudy { study_obj_path } => {
                    let study = Study::load_from_json(&study_obj_path);

                    if !study.outcomes_vec.is_empty() {
                        study.plot_results_evolution();
                    } else if !study.outcomes_map.is_empty() {
                        study.plot_results_varying();
                    } else {
                        panic!("No result to plot for study: {}", study_obj_path);
                    }
                }
            }
        }
        None => println!("Nothing to do, exiting."),
    }

    // TODO: add command to load study objects
    // Load a previous study
}
