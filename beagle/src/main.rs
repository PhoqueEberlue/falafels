mod individual_factory;
mod launcher;
mod options;
mod structures;
mod studies;

use clap::Parser;

use options::{Cli, Commands};
use studies::{
    evolution::{EvolutionCriteria, EvolutionStudy},
    varying::VaryingStudy,
    InputFiles, StudyBase, StudyKind,
};

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
                    let mut study = VaryingStudy::new(StudyBase::new(
                        args.simulation_name.clone(),
                        args.output_dir,
                        InputFiles {
                            clusters_path: args.clusters_path,
                            constants_path: args.constants_path,
                            profiles_path: args.profiles_path,
                            platform_specs: args.platform_specs,
                        },
                    ));

                    study.varying_machines_number_sim(step, total_number_gen);
                    study
                        .base
                        .export_to_json(studies::StudyKind::Varying(study.clone()));
                    study.plot_results_varying(args.show_plot);
                }
                // Study with evolution algorithm
                Commands::Evolution {
                    total_number_gen,
                    evolution_criteria,
                    delete_denominator,
                } => {
                    let mut study = EvolutionStudy::new(
                        StudyBase::new(
                            args.simulation_name.clone(),
                            args.output_dir,
                            InputFiles {
                                clusters_path: args.clusters_path,
                                constants_path: args.constants_path,
                                profiles_path: args.profiles_path,
                                platform_specs: args.platform_specs,
                            },
                        ),
                        match evolution_criteria.as_str() {
                            "total_consumption" => EvolutionCriteria::TotalConsumption,
                            "simulation_time" => EvolutionCriteria::SimulationTime,
                            "power_rate" => EvolutionCriteria::PowerRate,
                            _ => panic!("Evolution criteria incorrect. Please chose between 'total_consumption' and 'simulation_time'")
                        },
                        delete_denominator
                    );

                    study.evolution_algorithm_sim(total_number_gen);
                    study
                        .base
                        .export_to_json(studies::StudyKind::Evolution(study.clone()));
                    study.plot_results_evolution(args.show_plot);
                }
                Commands::LoadPreviousStudy { study_obj_path } => {
                    let study_kind = StudyBase::load_from_json(&study_obj_path);

                    match study_kind {
                        StudyKind::Varying(study) => {
                            study.plot_results_varying(args.show_plot);
                        }
                        StudyKind::Evolution(study) => {
                            study.plot_results_evolution(args.show_plot);
                        }
                    }
                }
            }
        }
        None => println!("Nothing to do, exiting."),
    }
}
