use clap::{Parser, Subcommand};

/// Beagle
///
/// This program is a helper to use and tweak the parameters of FaLaFElS simulations
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
pub struct Cli {
    #[arg(short, long)]
    pub simulation_name: String,

    #[arg(long)]
    pub output_dir: String,

    #[arg(long, default_value_t = String::from("./input/constants.xml"))]
    pub constants_path: String,

    #[arg(long, default_value_t = String::from("./input/profiles.xml"))]
    pub profiles_path: String,

    #[arg(long, default_value_t = String::from("./input/clusters.xml"))]
    pub clusters_path: String,

    #[arg(long)]
    pub platform_specs: Option<String>,

    #[arg(long, default_value_t = false)]
    pub show_plot: bool,

    #[command(subcommand)]
    pub command: Option<Commands>,
}

#[derive(Subcommand, Debug)]
pub enum Commands {
    /// Launches a study with varying machine numbers
    Varying {
        #[arg(long)]
        step: u16,
        #[arg(long)]
        total_number_gen: u32,
    },
    /// Launches a study with evolution algorithm
    Evolution {
        #[arg(long)]
        total_number_gen: u32,
        #[arg(long)]
        evolution_criteria: String,
        #[arg(long, default_value_t = 2)]
        /// The denominator used to delete a proportion of individuals each generation
        delete_denominator: usize,
    },
    LoadPreviousStudy {
        #[arg(long)]
        study_obj_path: String,
    },
}
