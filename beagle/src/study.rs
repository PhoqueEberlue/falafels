use std::{
    collections::hash_map::HashMap,
    error::Error,
    fs,
    path::Path,
    thread::{self, JoinHandle},
};

use itertools::Itertools;
use lazy_static::lazy_static;
use plotly::{
    common::{Anchor, Marker, Title},
    layout::{
        themes::{PLOTLY_DARK, PLOTLY_WHITE},
        Annotation, Axis, BarMode, GridPattern, LayoutGrid,
    },
    Bar, ImageFormat, Layout, Plot, Scatter,
};
use rand::{rngs::StdRng, seq::SliceRandom, SeedableRng};

use crate::structures::base::Clusters;
use crate::{individual_factory::IndividualFactory, structures::environment::Environment};
use crate::{launcher, structures::individual::Individual};
use fryer::{
    platformer::{Platformer, RawAndFried, SpecsAndProfiles},
    structures::{
        common::{Arg, Constants},
        fried::FriedFalafels,
        raw::{PlatformSpecs, Profiles, RawFalafels},
    },
};
use launcher::Outcome;
use serde::{Deserialize, Serialize};

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

lazy_static! {
    static ref COLOR_MAP: HashMap<&'static str, &'static str> = {
        let mut color_map = HashMap::new();
        color_map.insert("StarSimple", COLORS[0]);
        color_map.insert("StarAsynchronous", COLORS[1]);
        color_map.insert("RingUniSimple", COLORS[2]);
        color_map.insert("RingUniAsynchronous", COLORS[3]);
        color_map.insert("StarStarHierarchical", COLORS[4]);
        color_map.insert("RingUniRingUniHierarchical", COLORS[5]);
        color_map.insert("StarStarHierarchicalAsync", COLORS[6]);
        color_map.insert("RingUniRingUniHierarchicalAsync", COLORS[7]);
        color_map
    };
}

fn create_dir_if_not_exists<P: AsRef<Path>>(path: P) {
    match fs::create_dir(&path) {
        Ok(()) => {}
        // Ignore if directory already exists
        Err(e) if e.kind() == std::io::ErrorKind::AlreadyExists => {}
        Err(e) => panic!("{}", e),
    }
}

/// Struct containing the necessary inputs for a study
#[derive(Debug, Serialize, Deserialize)]
pub struct InputFiles {
    pub clusters_path: String,
    pub constants_path: String,
    pub profiles_path: String,
    pub platform_specs: Option<String>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Study {
    pub input_files: InputFiles,
    pub name: String,
    pub output_dir: String,
    pub outcomes_map: HashMap<String, Vec<Outcome>>,
    pub outcomes_vec: Vec<Vec<Outcome>>,
    pub x_axis: Vec<u16>,
}

impl Study {
    pub fn new(name: String, output_dir: String, input_files: InputFiles) -> Study {
        create_dir_if_not_exists(&output_dir);
        create_dir_if_not_exists(format!("{}/logs", &output_dir));
        create_dir_if_not_exists(format!("{}/fried", &output_dir));
        create_dir_if_not_exists(format!("{}/platform", &output_dir));

        Study {
            name,
            output_dir,
            outcomes_map: HashMap::new(),
            outcomes_vec: Vec::new(),
            x_axis: Vec::new(),
            input_files,
        }
    }

    pub fn export_to_json(&self) {
        let content = serde_json::to_string_pretty(self).unwrap();
        fs::write(format!("{}/study_obj.json", self.output_dir), content).unwrap();
    }

    pub fn load_from_json(study_path: &str) -> Study {
        let content = String::from_utf8(fs::read(study_path).unwrap()).unwrap();
        serde_json::from_str(&content).unwrap()
    }

    /// Creates a RawFalafels structure by loading and combininig 3 xml files:
    /// - base (the clusters)
    /// - constants
    /// - profiles
    fn recompose_rf(&self) -> Result<RawFalafels, Box<dyn Error>> {
        let base_content = fs::read_to_string(&self.input_files.clusters_path)?;
        let constants_content = fs::read_to_string(&self.input_files.constants_path)?;
        let profiles_content = fs::read_to_string(&self.input_files.profiles_path)?;

        let base: Clusters = quick_xml::de::from_str(&base_content)?;
        let constants: Constants = quick_xml::de::from_str(&constants_content)?;
        let profiles: Profiles = quick_xml::de::from_str(&profiles_content)?;

        // Handling optional case when using a PlatformSpecs
        let platform_specs = match &self.input_files.platform_specs {
            Some(path) => {
                let content = fs::read_to_string(path)?;
                quick_xml::de::from_str(&content)?
            }
            None => None,
        };

        Ok(RawFalafels {
            constants,
            profiles,
            clusters: base.list,
            platform_specs,
        })
    }

    pub fn varying_machines_number_sim(&mut self, step: u16, total_number_gen: u32) {
        let base_rf = self.recompose_rf().unwrap();

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

            let mut platformer = Platformer::new(RawAndFried {
                rf: &non_hierachical_ind.rf,
                ff: &non_hierachical_ind.ff,
            });

            let mut platformer_h = Platformer::new(RawAndFried {
                rf: &hierarchical_ind.rf,
                ff: &hierarchical_ind.ff,
            });

            let platform = platformer.create_star_topology();
            let platform_h = platformer_h.create_star_topology();

            let platform_path = format!(
                "{}/platform/GEN-{gen_nb}-simgrid-platform.xml",
                self.output_dir
            );
            let platform_path_h = format!(
                "{}/platform/GEN-{gen_nb}-simgrid-platform-hierarchical.xml",
                self.output_dir
            );

            platformer.write_platform(&platform_path, &platform);
            platformer_h.write_platform(&platform_path_h, &platform_h);

            let environment = Environment {
                platform,
                platform_path,
            };
            let environment_h = Environment {
                platform: platform_h,
                platform_path: platform_path_h,
            };

            let mut handles = vec![];

            for ind in &individuals {
                let p_path;

                if ind.is_hierarchical {
                    p_path = environment_h.platform_path.clone();
                } else {
                    p_path = environment.platform_path.clone();
                }

                let output_dir = self.output_dir.to_string();
                let mut individual = ind.clone();

                // Launch as much threads as there are individuals
                handles.push(thread::spawn(move || {
                    individual.gen_nb = gen_nb;
                    // Write individial fried file
                    individual.write_fried();

                    let outcome = launcher::run_simulation(
                        gen_nb as u32,
                        output_dir,
                        individual,
                        p_path,
                        false,
                    );
                    println!("{:?}", outcome);
                    outcome
                }));
            }

            // Wait for results and store in the outcome HashMap
            for handle in handles {
                let outcome = handle.join().unwrap();

                if self.outcomes_map.contains_key(&outcome.individual_name) {
                    self.outcomes_map
                        .get_mut(&outcome.individual_name)
                        .unwrap()
                        .push(outcome);
                } else {
                    self.outcomes_map
                        .insert(outcome.individual_name.clone(), vec![outcome]);
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

    pub fn plot_results_varying(&self) {
        let mut plot = Plot::new();

        // Set colors

        for (algo_topo, outcomes_vec) in &self.outcomes_map {
            let current_color = *COLOR_MAP.get(algo_topo.as_str()).unwrap();

            plot.add_trace(
                Scatter::new(
                    self.x_axis.clone(),
                    outcomes_vec.iter().map(|o| o.total_consumption).collect(),
                )
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x1")
                .y_axis("y1")
                .marker(Marker::new().color(current_color)),
            );

            plot.add_trace(
                Scatter::new(
                    self.x_axis.clone(),
                    outcomes_vec
                        .iter()
                        .map(|o| o.total_host_consumption)
                        .collect(),
                )
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x2")
                .y_axis("y2")
                .marker(Marker::new().color(current_color))
                .show_legend(false),
            );

            plot.add_trace(
                Scatter::new(
                    self.x_axis.clone(),
                    outcomes_vec
                        .iter()
                        .map(|o| o.total_link_consumption)
                        .collect(),
                )
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x3")
                .y_axis("y3")
                .marker(Marker::new().color(current_color))
                .show_legend(false),
            );

            plot.add_trace(
                Scatter::new(
                    self.x_axis.clone(),
                    outcomes_vec.iter().map(|o| o.simulation_time).collect(),
                )
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x4")
                .y_axis("y4")
                .marker(Marker::new().color(current_color))
                .show_legend(false),
            );

            plot.add_trace(
                Scatter::new(
                    self.x_axis.clone(),
                    outcomes_vec
                        .iter()
                        .map(|o| o.total_consumption / o.simulation_time)
                        .collect(),
                )
                .name(format!("{}", algo_topo.clone()))
                .x_axis("x5")
                .y_axis("y5")
                .marker(Marker::new().color(current_color))
                .show_legend(false),
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
                    .pattern(GridPattern::Independent),
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
        plot.write_image(
            format!("{}/out_white.ext", self.output_dir),
            ImageFormat::PNG,
            1920,
            1080,
            1.0,
        );
    }

    pub fn evolution_algorithm_sim(&mut self, total_number_gen: u32) {
        let nb_individual_per_category = 20;

        // Set seed
        let mut rng = StdRng::seed_from_u64(42);

        let base_rf = self.recompose_rf().unwrap();

        // Create one common platform for every individuals
        let mut platformer = Platformer::new(SpecsAndProfiles {
            specs: &base_rf.platform_specs.as_ref().unwrap(),
            profiles: &base_rf.profiles,
        });

        let platform = platformer.create_star_topology();

        let platform_path = format!("{}/platform/simgrid-platform.xml", self.output_dir);

        platformer.write_platform(&platform_path, &platform);

        let environment = Environment {
            platform,
            platform_path,
        };

        let mut factory = IndividualFactory::new(base_rf, &self.output_dir);

        let mut number_ind = 0;
        // Init each category of individual multiple times
        let individuals = (0..nb_individual_per_category)
            .map(|_| {
                let mut inds = factory.init_individuals();
                inds.iter_mut().for_each(|ind| {
                    // Update name of the individual so it has a unique name
                    ind.name = format!("{}_{}", ind.name, number_ind);
                    // Shuffle the names of the nodes so they have random NodeProfiles
                    ind.ff.shuffle_node_names(&mut rng);
                    // After shuffling we need to recompute the links
                    ind.ff.add_booststrap_nodes();
                    ind.ff.link_hierarchical_aggregators();
                    number_ind += 1;
                });
                inds
            })
            .collect::<Vec<_>>();

        let mut individuals_by_category = (0..individuals.get(0).unwrap().len())
            .map(|i| individuals.iter().map(|vec| vec[i].clone()).collect())
            .collect::<Vec<_>>();

        for gen_nb in 0..total_number_gen {
            let mut outcomes_gen = vec![];

            for individuals in individuals_by_category.iter_mut() {
                // Append outcomes
                outcomes_gen.append(
                    // Pass the list of individuals and the function will apply modifications on it
                    &mut self.evolution_iteration(individuals, &environment, &mut rng, gen_nb)
                );
            }

            self.outcomes_vec.push(outcomes_gen);
        }
    }

    /// Step one evolution iteration and modifies the individuals vector for the next iteration 
    fn evolution_iteration(
        &mut self,
        individuals: &mut Vec<Individual>,
        environment: &Environment,
        rng: &mut StdRng,
        gen_nb: u32,
    ) -> Vec<Outcome> {
        let mut handles = vec![];

        individuals.iter().for_each(|ind| {
            // Clone the arguments we need to pass to the thread
            let output_dir = self.output_dir.to_string();
            let mut individual = ind.clone();
            let p_path = environment.platform_path.clone();

            // Run in a thread
            handles.push(thread::spawn(move || {
                individual.gen_nb = gen_nb;
                // Write the FriedFalafels file
                individual.write_fried();

                match individual.previous_outcome {
                    // Case previous outcomes exists, it means nothing changed in the config so we
                    // don't have to compute again
                    Some(previous) => previous,
                    // Else, launch simulation
                    None => {
                        let outcome =
                            launcher::run_simulation(gen_nb, output_dir, individual, p_path, false);
                        println!("{:?}", outcome);
                        outcome
                    }
                }
            }));
        });

        // Wait for results and store in the outcomes_vec
        let outcomes = handles
            .into_iter()
            .map(|h| h.join().unwrap())
            .collect::<Vec<_>>();

        // Zip outcomes and individuals together in order to sort individuals by their outcome
        let mut tmp = outcomes.iter().zip(individuals.iter()).collect::<Vec<_>>();
        tmp.sort_by(|a, b| {
            // Here we compare simulation time for the sort
            a.0.simulation_time
                .partial_cmp(&b.0.simulation_time)
                .unwrap()
        });

        // Gets a vector of owned individuals
        let mut sorted_individuals = tmp
            .iter_mut()
            .map(|oi| {
                let mut i = oi.1.clone();
                // Save previous outcome to prevent recomputing
                i.previous_outcome = Some(oi.0.clone());
                i
            })
            .collect::<Vec<_>>();

        // Gets a vector of owned outcomes
        let sorted_outcomes = tmp.iter_mut().map(|oi| oi.0.clone()).collect::<Vec<_>>();

        // Delete the worst individuals
        let nb_individual_to_delete = sorted_individuals.len() / 2;
        sorted_individuals.truncate(sorted_individuals.len() - nb_individual_to_delete);

        // Clone a number of `nb_individual_to_delete` by randomly selecting them and mutate them.
        let mut mutated_individuals = sorted_individuals
            .choose_multiple(rng, nb_individual_to_delete)
            .map(|i| {
                let mut new_i = i.clone();
                // Erase previous outcome to force recomputing because these individuals will be
                // mutated
                new_i.previous_outcome = None;
                new_i
            })
            .collect::<Vec<_>>();

        mutated_individuals.iter_mut().for_each(|i| {
            // The name of the mutated individuals become <previous name>_mut_<gen_nb>
            i.name = format!("{}_mut_{}", i.name, gen_nb);
            i.ff.mutate_nodes(rng);
            // Change a small proportion of the roles
            i.ff.permute_node_names(rng, i.ff.get_number_nodes() as u32 / 5);
            // Recompute links because of previous permutation
            i.ff.link_hierarchical_aggregators();
            i.ff.add_booststrap_nodes();
        });

        // // Add the mutated individuals to the final list
        sorted_individuals.append(&mut mutated_individuals);

        individuals.clear();
        individuals.append(&mut sorted_individuals);

        sorted_outcomes
    }

    pub fn plot_results_evolution(&self) {
        // get the different categories
        let categories = self
            .outcomes_vec
            .get(0)
            .unwrap()
            .into_iter()
            .unique_by(|o| &o.category)
            .map(|o| o.category.clone())
            .collect::<Vec<_>>();

        let mut plot = Plot::new();

        // Stacked bar plot to show the evolution of categories
        // for category_name in &categories {
        //     let current_color = *COLOR_MAP.get(category_name.as_str()).unwrap();

        //     let trace = Bar::new(
        //         (0..self.outcomes_vec.len()).collect(),
        //         // For each category, get the number of individuals
        //         self.outcomes_vec
        //             .iter()
        //             // Count the number of individuals in the current category
        //             .map(|outcomes| {
        //                 outcomes
        //                     .iter()
        //                     .filter(|o| &o.category == category_name)
        //                     .count()
        //             })
        //             .collect(),
        //     )
        //     .marker(Marker::new().color(current_color))
        //     .x_axis("x1")
        //     .y_axis("y1")
        //     .name(category_name);

        //     plot.add_trace(trace);
        // }

        // Plotting the simulation time of the best individals of each category
        for category_name in &categories {
            let current_color = *COLOR_MAP.get(category_name.as_str()).unwrap();

            let outcomes_cat = self
                .outcomes_vec
                .iter()
                // Count the number of individuals in the current category
                .map(|outcomes| {
                    outcomes
                        .iter()
                        .filter(|o| &o.category == category_name)
                        .collect::<Vec<_>>()
                })
                .collect::<Vec<_>>();

            let trace = Scatter::new(
                (0..self.outcomes_vec.len()).collect(),
                // For each category, get the number of individuals
                outcomes_cat
                    .iter()
                    .map(|outcomes| outcomes.iter().map(|o| o.simulation_time as u64).min())
                    .collect(),
            )
            .marker(Marker::new().color(current_color))
            .x_axis("x1")
            .y_axis("y1")
            .name(category_name);

            plot.add_trace(trace);
        }

        let layout = Layout::new()
            .template(&*PLOTLY_WHITE)
            .y_axis(Axis::new().title(Title::new("Total number of individuals")))
            .x_axis(Axis::new().title(Title::new("Generation number")))
            .bar_mode(BarMode::Stack)
            .height(1000)
            .grid(
                LayoutGrid::new()
                    .rows(2)
                    .columns(1)
                    .pattern(GridPattern::Independent),
            );

        plot.set_layout(layout);
        plot.show();

        plot.write_html(format!("{}/out.html", self.output_dir));
        plot.write_image(
            format!("{}/out_white.ext", self.output_dir),
            ImageFormat::PNG,
            1920,
            1080,
            1.0,
        );
    }
}
