use clap::{Parser, Subcommand};

/// Beagle
///
/// This program is a helper to use and tweak the parameters of FaLaFElS simulations
#[derive(Parser, Debug)]
#[command(version, about, long_about = None)]
pub struct Cli {
    #[arg(short, long)]
    pub simulation_name: String,

    #[arg(short, long, default_value_t = String::from("./input/constants.xml"))]
    pub constants_path: String,

    #[arg(short, long, default_value_t = String::from("./input/profiles.xml"))]
    pub profiles_path: String,

    #[arg(short, long, default_value_t = String::from("./input/clusters.xml"))]
    pub clusters_path: String,

    #[arg(short, long)]
    pub platform_specs: Option<String>,

    #[command(subcommand)]
    pub command: Option<Commands>,
}

#[derive(Subcommand, Debug)]
pub enum Commands {
    /// Launches a study with varying machine numbers
    Varying {
        #[arg(short, long)]
        step: u16,
        #[arg(short, long)]
        total_number_gen: u16,
    },
    /// Launches a study with evolution algorithm
    Evolution {
        /// Tmp
        yes: bool,
    },
}
