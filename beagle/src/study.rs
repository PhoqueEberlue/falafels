use std::{collections::hash_map::HashMap, error::Error, fs, path::Path, thread};

use plotly::{common::{Anchor, Marker, Title}, layout::{themes::{PLOTLY_DARK, PLOTLY_WHITE}, Annotation, Axis, GridPattern, LayoutGrid}, ImageFormat, Layout, Plot, Scatter};

use serde::{Serialize, Deserialize};
use crate::environment::Environment;
use fryer::{platformer::Platformer, structures::{common::{Arg, Constants}, raw::{Profiles, RawFalafels}}};
use crate::individual::{Individual, IndividualFactory};
use launcher::Outcome;
use crate::structures::base::Clusters;
use crate::launcher;
use quick_xml::de::from_str;


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

fn create_dir_if_not_exists<P: AsRef<Path>>(path: P) {
    match fs::create_dir(&path) {
        Ok(()) => {},
        // Ignore if directory already exists
        Err(e) if e.kind() == std::io::ErrorKind::AlreadyExists => {},
        Err(e) => panic!("{}", e),
    }
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
        create_dir_if_not_exists(&output_dir);
        create_dir_if_not_exists(format!("{}/logs", &output_dir));
        create_dir_if_not_exists(format!("{}/fried", &output_dir));
        create_dir_if_not_exists(format!("{}/platform", &output_dir));

        Study { name, output_dir, outcomes_map: HashMap::new(), x_axis: Vec::new() }
    } 

    pub fn load_from_json(study_path: &str) -> Study {
        let content = String::from_utf8(fs::read(study_path).unwrap()).unwrap();
        serde_json::from_str(&content).unwrap()
    }

    /// Creates a RawFalafels structure by loading and combininig 3 xml files:
    /// - base (the clusters)
    /// - constants
    /// - profiles
    pub fn recompose_rf(base_path: &str, constants_path: &str, profiles_path: &str) -> Result<RawFalafels, Box<dyn Error>> {
        let base_content = fs::read_to_string(base_path)?;
        let constants_content = fs::read_to_string(constants_path)?;
        let profiles_content = fs::read_to_string(profiles_path)?;

        let base: Clusters = from_str(&base_content)?;
        let constants: Constants = from_str(&constants_content)?;
        let profiles: Profiles = from_str(&profiles_content)?;

        Ok(RawFalafels { constants, profiles, clusters: base.list })
    }

    pub fn export_to_json(&self) {
        let content = serde_json::to_string_pretty(self).unwrap();
        fs::write(format!("{}/study_obj.json", self.output_dir), content).unwrap();
    }

    pub fn varying_machines_number_sim(&mut self, base_rf: RawFalafels, step: u16, total_number_gen: u16) {


        let mut factory = IndividualFactory::new(base_rf, &self.output_dir);

        // let main_cluster = factory.base_rf.clusters.get_mut(0).unwrap();
        // let args = main_cluster.aggregators.args.get_or_insert_with(|| Vec::new());
        // args.push(Arg { name: "number_local_epochs".to_string(), value: "1".to_string() });

        // let mut number_local_epochs = 1;

        // let start_number = 500;
        // factory.base_rf.clusters.list.get_mut(0).unwrap().trainers.as_mut().unwrap().number = start_number;

        for gen_nb in 0..total_number_gen {
            factory.generation_number = gen_nb as u32;

            let individuals = factory.init_individuals();

            // we generate one platform for hierarchical topologies and one for others
            let non_hierachical_ind = &individuals.get(0).unwrap();
            let hierarchical_ind = &individuals.last().unwrap();
            let mut platformer = Platformer::new(&non_hierachical_ind.rf, &non_hierachical_ind.ff);
            let mut platformer_h = Platformer::new(&hierarchical_ind.rf, &hierarchical_ind.ff);

            let platform = platformer.create_star_topology();
            let platform_h = platformer_h.create_star_topology();
            
            let platform_path = format!("{}/platform/GEN-{gen_nb}-simgrid-platform.xml", self.output_dir);
            let platform_path_h = format!("{}/platform/GEN-{gen_nb}-simgrid-platform-hierarchical.xml", self.output_dir);

            platformer.write_platform(&platform_path, &platform);
            platformer_h.write_platform(&platform_path_h, &platform_h);

            let environment = Environment { platform, platform_path };
            let environment_h = Environment { platform: platform_h, platform_path: platform_path_h };

            let mut handles = vec![];

            for ind in &individuals {
                let p_path;

                if ind.is_hierarchical {
                    p_path = environment_h.platform_path.clone();
                } else {
                    p_path = environment.platform_path.clone();
                }

                let output_dir = self.output_dir.to_string();
                let individual = ind.clone();

                // Launch as much threads as there are individuals
                handles.push(
                    thread::spawn(move || {
                        let outcome = launcher::run_simulation(gen_nb as u32, output_dir, individual, p_path);
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

            let main_cluster = factory.base_rf.clusters.get_mut(0).unwrap();
            // Add current number of machines to x_axis
            self.x_axis.push(main_cluster.trainers.number);
            
            // self.x_axis.push(number_local_epochs);
            // let args = main_cluster.aggregators.args.as_mut().unwrap();
            // number_local_epochs += 1;
            // args.get_mut(0).unwrap().value = number_local_epochs.to_string();

            // Increase the number of machines to be used for the next platform creation
            main_cluster.trainers.number += step as u16;
        }
    }

    pub fn plot_results(&self) {
        let mut plot = Plot::new();

        // Set colors
        let mut color_map = HashMap::new();
        color_map.insert("StarSimple", COLORS[0]);
        color_map.insert("StarAsynchronous", COLORS[1]);
        color_map.insert("RingUniSimple", COLORS[2]);
        color_map.insert("RingUniAsynchronous", COLORS[3]);
        color_map.insert("StarStarHierarchical", COLORS[4]);
        color_map.insert("RingUniRingUniHierarchical", COLORS[5]);
        color_map.insert("StarStarHierarchicalAsync", COLORS[6]);
        color_map.insert("RingUniRingUniHierarchicalAsync", COLORS[7]);

        for (algo_topo, outcomes_vec) in &self.outcomes_map {
            let current_color = *color_map.get(algo_topo.as_str()).unwrap();

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_consumption).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x1")
                .y_axis("y1")
                .marker(Marker::new().color(current_color))
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_host_consumption).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x2")
                .y_axis("y2")
                .marker(Marker::new().color(current_color))
                .show_legend(false)
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_link_consumption).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x3")
                .y_axis("y3")
                .marker(Marker::new().color(current_color))
                .show_legend(false)
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.simulation_time).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x4")
                .y_axis("y4")
                .marker(Marker::new().color(current_color))
                .show_legend(false)
            );

            plot.add_trace(Scatter::new(self.x_axis.clone(), outcomes_vec.iter().map(|o| o.total_consumption / o.simulation_time).collect())
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x5")
                .y_axis("y5")
                .marker(Marker::new().color(current_color))
                .show_legend(false)
            );
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
