use std::{collections::hash_map::HashMap, fs, thread};

use plotly::{common::{Anchor, Marker, Title}, layout::{themes::{PLOTLY_DARK, PLOTLY_WHITE}, Annotation, Axis, GridPattern, LayoutGrid}, ImageFormat, Layout, Plot, Scatter};

use serde::{Serialize, Deserialize};
use environment::Environment;
use fryer::platformer::Platformer;
use individual::{Individual, IndividualFactory};
use launcher::Outcome;

mod environment;
mod individual;
mod launcher;

// Colors for dark mode
// const COLORS: &'static [&'static str; 10] = &[
//     "#636efa", "#EF553B", "#00cc96", "#ab63fa", "#FFA15A", "#19d3f3", "#FF6692", "#B6E880",
//     "#FF97FF", "#FECB52",
// ];

// Colors for white mode
const COLORS: &'static [&'static str; 10] = &[
            "#636efa", "#EF553B", "#00cc96", "#ab63fa", "#FFA15A", "#19d3f3", "#FF6692", "#B6E880",
            "#FF97FF", "#FECB52",
];

fn main() {
    // Study with varying machines number
    let mut study = Study::new(
        "Comparision of Algorithms/Topology combination with varying machines number".to_string(), 
        "./records/varying_machines".to_string()
    );

    study.varying_machines_number_sim();
    study.export_to_json();
    study.plot_results();

    // Load a previous study
    // let study = Study::load_from_json("./records/varying_machines/study_obj.json");
    // study.plot_results();
}


#[derive(Debug, Serialize, Deserialize)]
pub struct Study {
    pub name: String,
    pub output_dir: String,
    pub outcomes_map: HashMap<String, Vec<Outcome>>,
    pub x_axis: Vec<u16>,
}

impl Study {
    pub fn new(name: String, output_dir: String) -> Study {
        fs::create_dir(&output_dir).unwrap();
        fs::create_dir(format!("{}/logs", &output_dir)).unwrap();
        fs::create_dir(format!("{}/fried", &output_dir)).unwrap();
        fs::create_dir(format!("{}/platform", &output_dir)).unwrap();

        Study { name, output_dir, outcomes_map: HashMap::new(), x_axis: Vec::new() }
    }

    pub fn load_from_json(study_path: &str) -> Study {
        let content = String::from_utf8(fs::read(study_path).unwrap()).unwrap();
        serde_json::from_str(&content).unwrap()
    }

    pub fn export_to_json(&self) {
        let content = serde_json::to_string_pretty(self).unwrap();
        fs::write(format!("{}/study_obj.json", self.output_dir), content).unwrap();
    }

    pub fn varying_machines_number_sim(&mut self) {
        let mut factory = IndividualFactory::new("../xml/raw-falafels.xml", &self.output_dir);

        let total_number_gen = 5;
        let step_trainer = 50;

        // let start_number = 500;
        // factory.base_rf.clusters.list.get_mut(0).unwrap().trainers.as_mut().unwrap().number = start_number;

        for gen_nb in 0..total_number_gen {
            factory.generation_number = gen_nb as u32;

            let individuals = factory.init_individuals();

            // we generate a common platform with any fried file we want because they will all generate the
            // same platform anyways
            let mut platformer = Platformer::new(&factory.base_rf, &individuals.get(0).unwrap().ff);

            let platform = platformer.create_star_topology();
            
            let platform_path = format!("{}/platform/GEN-{gen_nb}-simgrid-platform.xml", self.output_dir);

            platformer.write_platform(&platform_path, &platform);

            let environment = Environment { platform, platform_path };

            let mut handles = vec![];

            for ind in &individuals {
                let platform_path = environment.platform_path.clone();

                let output_dir = self.output_dir.to_string();
                let individual = ind.clone();

                // Launch as much threads as there are individuals
                handles.push(
                    thread::spawn(move || {
                        let outcome = launcher::run_simulation(gen_nb as u32, output_dir, individual, platform_path);
                        println!("{:?}", outcome);
                        outcome
                    })
                );
            }

            // Wait for results and store in the outcome HashMap
            for handle in handles {
                let outcome = handle.join().unwrap();

                if self.outcomes_map.contains_key(&outcome.individual_name) {
                    self.outcomes_map.get_mut(&outcome.individual_name).unwrap().push(outcome);
                }
                else {
                    self.outcomes_map.insert(outcome.individual_name.clone(), vec![outcome]);
                }
            }

            let main_cluster = factory.base_rf.clusters.list.get_mut(0).unwrap();
            // Add current number of machines to x_axis
            self.x_axis.push(main_cluster.trainers.as_mut().unwrap().number);

            // Increase the number of machines to be used for the next platform creation
            main_cluster.trainers.as_mut().unwrap().number += step_trainer as u16;
        }
    }

    pub fn plot_results(&self) {
        let mut plot = Plot::new();

        let mut i = 0;

        for (algo_topo, outcomes_vec) in &self.outcomes_map {
            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_consumption).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x1")
                .y_axis("y1")
                .marker(Marker::new().color(COLORS[i]))
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_host_consumption).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x2")
                .y_axis("y2")
                .marker(Marker::new().color(COLORS[i]))
                .show_legend(false)
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_link_consumption).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x3")
                .y_axis("y3")
                .marker(Marker::new().color(COLORS[i]))
                .show_legend(false)
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.simulation_time).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x4")
                .y_axis("y4")
                .marker(Marker::new().color(COLORS[i]))
                .show_legend(false)
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_consumption / o.simulation_time).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x5")
                .y_axis("y5")
                .marker(Marker::new().color(COLORS[i]))
                .show_legend(false)
            );

            i+=1;
        }

        let mut layout = Layout::new()
            .height(1000)
            // Set dark theme
            .template(&*PLOTLY_WHITE)
            .y_axis(Axis::new().title(Title::new("Energy in joules")))
            .x_axis(Axis::new().title(Title::new("Number of trainers")))
            .y_axis4(Axis::new().title(Title::new("Time in seconds")))
            .y_axis5(Axis::new().title(Title::new("Energy in Watts")))
            // Set grid layout for sub_plots
            .grid(
                LayoutGrid::new()
                    .rows(2)
                    .columns(3)
                    .pattern(GridPattern::Independent)
            )
            .title(Title::new(&self.name));

        // Add sub titles by hand because this feature isn't available in ploty.rs :(
        layout.add_annotation(
            Annotation::new()
                .y_ref(format!("y{} domain", 1))
                .y_anchor(Anchor::Bottom)
                .y(0)
                .text("Total consumption")
                .x_ref(format!("x{} domain", 1))
                .x_anchor(Anchor::Center)
                .x(0.5)
                .show_arrow(false),
        );

        layout.add_annotation(
            Annotation::new()
                .y_ref(format!("y{} domain", 2))
                .y_anchor(Anchor::Bottom)
                .y(0)
                .text("Host consumption")
                .x_ref(format!("x{} domain", 2))
                .x_anchor(Anchor::Center)
                .x(0.5)
                .show_arrow(false),
        );

        layout.add_annotation(
            Annotation::new()
                .y_ref(format!("y{} domain", 3))
                .y_anchor(Anchor::Bottom)
                .y(0)
                .text("Link consumption")
                .x_ref(format!("x{} domain", 3))
                .x_anchor(Anchor::Center)
                .x(0.5)
                .show_arrow(false),
        );

        layout.add_annotation(
            Annotation::new()
                .y_ref(format!("y{} domain", 4))
                .y_anchor(Anchor::Bottom)
                .y(0)
                .text("Simulation time")
                .x_ref(format!("x{} domain", 4))
                .x_anchor(Anchor::Center)
                .x(0.5)
                .show_arrow(false),
        );

        layout.add_annotation(
            Annotation::new()
                .y_ref(format!("y{} domain", 5))
                .y_anchor(Anchor::Bottom)
                .y(0)
                .text("Total consumption / simulation time")
                .x_ref(format!("x{} domain", 5))
                .x_anchor(Anchor::Center)
                .x(0.5)
                .show_arrow(false),
        );

        plot.set_layout(layout);
        // plot.set_configuration(Configuration::new().responsive(true));
        plot.show();

        plot.write_html(format!("{}/out.html", self.output_dir));
        plot.write_image(format!("{}/out_white.ext", self.output_dir), ImageFormat::PNG, 1920, 1080, 1.0);
    }
}
