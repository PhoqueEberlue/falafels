use regex::Regex;

use crate::structures::individual::Individual;

use serde::{Deserialize, Serialize};
use std::fs::File;
use std::io::Write;
use std::process::Command;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Outcome {
    pub individual_name: String,
    pub command: String,
    pub total_host_consumption: f32,
    pub used_host_consumption: f32,
    pub idle_host_consumption: f32,
    pub total_link_consumption: f32,
    pub total_consumption: f32,
    pub simulation_time: f32,
}

pub fn run_simulation(
    gen_nb: u32,
    output_dir: String,
    ind: Individual,
    platform_path: String,
    write_logs: bool
) -> Outcome {
    let output = Command::new("../simulator/build/main")
        .args([&platform_path, &ind.ff_path])
        .output()
        .expect("failed to execute process");

    if write_logs {
        // Writing logs of the output
        let mut file =
            File::create(format!("{output_dir}/logs/GEN-{gen_nb}-{}.txt", ind.name)).unwrap();
        file.write_all(&output.stderr).unwrap();
    }

    let logs = String::from_utf8(output.stderr).unwrap();

    let [total_host_consumption, used_host_consumption, idle_host_consumption] =
        parse_host_energy_results(&logs);

    let total_link_consumption = parse_link_energy_results(&logs);

    let simulation_time = parse_simulation_time(&logs);

    Outcome {
        individual_name: ind.name.clone(),
        command: format!("../simulator/build/main {} {}", platform_path, &ind.ff_path),
        total_host_consumption,
        used_host_consumption,
        idle_host_consumption,
        total_link_consumption,
        total_consumption: total_host_consumption + total_link_consumption,
        simulation_time,
    }
}

///
///
/// Return: total_host_consumption, used_host_consumption, idle_host_consumption
fn parse_host_energy_results(logs: &String) -> [f32; 3] {
    // Look for the line containing host energy informations
    let hosts_consumption = logs
        .lines()
        .rev() // Reverse the iterator because we know the result is at the end
        .find(|l| l.contains("Total energy consumption"))
        .expect(&format!("{}\n Couldn't find 'Total energy consumption' in the logs.", logs));

    // Match every floating numbers with varying digit number
    let re = Regex::new(r"([0-9]+\.[0-9]+)").unwrap();

    // Captures each floating number in the line containing energy consumption information and
    // creates a vector of owned string
    let v = re
        .captures_iter(hosts_consumption)
        .map(
            // Parse each value into f32
            |n| {
                n[0].parse::<f32>().expect(&format!(
                    "Couldn't parse value `{}` into f32",
                    n[0].to_owned()
                ))
            },
        )
        .skip(1) // Skip simulation time
        .take(3)
        .collect::<Vec<f32>>();

    // Verify we obtained exactly 3 values and return a fixed size array.
    <[f32; 3]>::try_from(v).ok().expect(&format!(
        "Failed to capture 3 FP values on line: {hosts_consumption}"
    ))
}

///
///
/// Return: total_link_consumption
fn parse_link_energy_results(logs: &String) -> f32 {
    // Look for the line containing link energy informations
    let links_consumption = logs
        .lines()
        .rev() // Reverse the iterator because we know the result is at the end
        .find(|l| l.contains("Total energy over all links"))
        .expect(&format!("{}\n Couldn't find 'Total energy over all links' in the logs.", logs));

    // Match every floating numbers with varying digit number
    let re = Regex::new(r"([0-9]+\.[0-9]+)").unwrap();

    let link_energy = re
        .captures_iter(links_consumption)
        .map(|n| {
            n[0].parse::<f32>().expect(&format!(
                "Couldn't parse value `{}` into f32",
                n[0].to_owned()
            ))
        })
        .skip(1) // Skip simulation time
        .take(1) // Take energy
        .next();

    // Verify we obtained a value and return it
    link_energy.expect(&format!(
        "Failed to capture 1 FP values on line: {links_consumption}"
    ))
}

///
///
/// Return: simulation_time
fn parse_simulation_time(logs: &String) -> f32 {
    // Look for the line containing host energy informations
    let simulation_over = logs
        .lines()
        .rev() // Reverse the iterator because we know the result is at the end
        .find(|l| l.contains("Total energy over all links"))
        .expect(&format!("{}\n Couldn't find 'Total energy over all links' in the logs.", logs));

    // Match every floating numbers with varying digit number
    let re = Regex::new(r"([0-9]+\.[0-9]+)").unwrap();

    let simulation_time = re
        .captures_iter(simulation_over)
        .map(|n| {
            n[0].parse::<f32>().expect(&format!(
                "Couldn't parse value `{}` into f32",
                n[0].to_owned()
            ))
        })
        .take(1) // Take simulation time
        .next();

    // Verify we obtained a value and return it
    simulation_time.expect(&format!(
        "Failed to capture 1 FP values on line: {simulation_over}"
    ))
}
