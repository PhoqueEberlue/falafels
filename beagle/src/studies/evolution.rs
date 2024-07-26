use plotly::{
    common::{Mode, Title},
    layout::{
        themes::PLOTLY_WHITE,
        Axis, GridPattern, LayoutGrid
    },
    ImageFormat, Layout, Plot, Scatter,
};

use rand::{rngs::StdRng, seq::SliceRandom, SeedableRng};

use itertools::Itertools;
use std::fmt;
use std::{cmp::Ordering, thread};

use serde::{Deserialize, Serialize};

use crate::{
    individual_factory::IndividualFactory,
    launcher::{self, Outcome},
    structures::individual::Individual,
};

use super::StudyBase;

#[derive(Clone, Serialize, Deserialize, Debug)]
pub enum EvolutionCriteria {
    TotalConsumption,
    SimulationTime,
}

impl fmt::Display for EvolutionCriteria {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::SimulationTime => {
                write!(f, "simulation_time")
            }
            Self::TotalConsumption => {
                write!(f, "total_consumption")
            }
        }
    }
}

impl EvolutionCriteria {
    pub fn compare(&self, a: &Outcome, b: &Outcome) -> Ordering {
        match self {
            EvolutionCriteria::SimulationTime => {
                a.simulation_time.partial_cmp(&b.simulation_time).unwrap()
            }
            EvolutionCriteria::TotalConsumption => a
                .total_consumption
                .partial_cmp(&b.total_consumption)
                .unwrap(),
        }
    }
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct EvolutionStudy {
    pub base: StudyBase,
    pub outcomes_vec: Vec<Vec<Outcome>>,
    pub evolution_criteria: EvolutionCriteria,
    // The denominator used to delete a proportion of individuals each generation
    pub delete_denominator: usize,
}

impl EvolutionStudy {
    pub fn new(
        base: StudyBase,
        evolution_criteria: EvolutionCriteria,
        delete_denominator: usize,
    ) -> EvolutionStudy {
        EvolutionStudy {
            base,
            outcomes_vec: vec![],
            evolution_criteria,
            delete_denominator,
        }
    }

    pub fn evolution_algorithm_sim(&mut self, total_number_gen: u32) {
        let nb_individual_per_category = 10;

        // Set seed
        let mut rng = StdRng::seed_from_u64(42);

        // Base RawFalafels are config files use as a base to construct the individuals
        let base_rf = self.base.recompose_rf().unwrap();

        let mut factory = IndividualFactory::new(base_rf, &self.base.output_dir);

        let mut number_ind = 0;
        // Init each category of individual multiple times
        let individuals = (0..nb_individual_per_category)
            .map(|_| {
                // Create a factory with the current base
                let mut inds = factory.init_individuals();
                inds.iter_mut().for_each(|ind| {
                    // Update name of the individual so it has a unique name
                    ind.meta.name = format!("{}_{}", ind.meta.name, number_ind);
                    // Shuffle the names of the nodes so they have random NodeProfiles
                    // ind.content.as_mut().unwrap().ff.shuffle_node_names(&mut rng);
                    // ind.refresh_content();
                    number_ind += 1;
                });
                inds
            })
            .collect::<Vec<_>>();

        let mut individuals_by_category = (0..individuals.get(0).unwrap().len())
            .map(|i| {
                individuals
                    .iter()
                    .map(|vec| vec[i].clone())
                    .collect::<Vec<_>>()
            })
            .collect::<Vec<_>>();

        for gen_nb in 0..total_number_gen {
            let mut outcomes_gen = vec![];

            for individuals in individuals_by_category.iter_mut() {
                // Append outcomes
                outcomes_gen.append(
                    // Pass the list of individuals and the function will apply modifications on it
                    &mut self.evolution_iteration(individuals, &mut rng, gen_nb),
                );
            }

            self.outcomes_vec.push(outcomes_gen);
        }
    }

    /// Step one evolution iteration and modifies the individuals vector for the next iteration
    fn evolution_iteration(
        &mut self,
        individuals: &mut Vec<Individual>,
        rng: &mut StdRng,
        gen_nb: u32,
    ) -> Vec<Outcome> {
        let mut handles = vec![];

        individuals.iter().for_each(|ind| {
            // Clone the arguments we need to pass to the thread
            let output_dir = self.base.output_dir.to_string();
            let mut individual = ind.clone();

            // Run in a thread
            handles.push(thread::spawn(move || {
                individual.content.as_mut().unwrap().gen_nb = gen_nb;
                // Write the FriedFalafels file
                individual.write_fried();
                individual.gen_and_write_platform();

                match &individual.content.as_ref().unwrap().previous_outcome {
                    // Case previous outcomes exists, it means nothing changed in the config so we
                    // don't have to compute again
                    Some(previous) => previous.clone(),
                    // Else, launch simulation
                    None => {
                        let outcome =
                            launcher::run_simulation(gen_nb, output_dir, individual, false);
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
            // Compare depending on the criteria used
            self.evolution_criteria.compare(a.0, b.0)
        });

        // Gets a vector of owned individuals
        let mut sorted_individuals = tmp
            .iter_mut()
            .map(|oi| {
                let mut i = oi.1.clone();
                // Save previous outcome to prevent recomputing
                i.content.as_mut().unwrap().previous_outcome = Some(oi.0.clone());
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
                new_i.content.as_mut().unwrap().previous_outcome = None;
                new_i
            })
            .collect::<Vec<_>>();

        mutated_individuals.iter_mut().for_each(|i| {
            // The name of the mutated individuals become <previous name>_mut_<gen_nb>
            i.meta.name = format!("{}_{}", i.meta.name, gen_nb + 1);

            // Increment the number of machines in the platform
            let incr_nb = i
                .meta
                .base_rf
                .platform_specs
                .as_mut()
                .unwrap()
                .vary_random_profiles(rng);

            // Increase the number of trainers
            if incr_nb < 0 {
                i.meta.base_rf.clusters.get_mut(0).unwrap().trainers.number -= incr_nb.abs() as u16;
            } else {
                i.meta.base_rf.clusters.get_mut(0).unwrap().trainers.number += incr_nb as u16;
            }

            // Refresh content after incrementing the number of machines
            i.refresh_content();

            i.content.as_mut().unwrap().ff.mutate_nodes(rng);
            // Change a small proportion of the roles
            let nb_permut = i.content.as_ref().unwrap().ff.get_number_nodes() as u32 / 5;
            i.content
                .as_mut()
                .unwrap()
                .ff
                .permute_node_names(rng, nb_permut);

            i.content
                .as_mut()
                .unwrap()
                .ff
                .link_hierarchical_aggregators();
            i.content.as_mut().unwrap().ff.add_booststrap_nodes();
        });

        // Add the mutated individuals to the final list
        sorted_individuals.append(&mut mutated_individuals);

        individuals.clear();
        individuals.append(&mut sorted_individuals);

        sorted_outcomes
    }

    pub fn plot_results_evolution(&self) {
        self.plot_best_individuals();
        self.plot_top_individuals();
    }

    fn plot_best_individuals(&self) {
        // Separate the outcomes into different categories
        let categories = self
            .outcomes_vec
            .get(0)
            .unwrap()
            .into_iter()
            .unique_by(|o| &o.category)
            .map(|o| o.category.clone())
            .collect::<Vec<_>>();

        let mut plot = Plot::new();

        // Get the best individuals for each generation for each category
        let best_inds = categories
            .iter()
            // Loop through category
            .map(|category_name| {
                // Return category name and best individuals for each generation of this category
                (
                    category_name,
                    self.outcomes_vec
                        .iter()
                        .map(|outcomes| {
                            outcomes
                                .iter()
                                // Only select individuals from the current category
                                .filter(|o| &o.category == category_name)
                                // Get the best one according to the defined criteria
                                .min_by(|a, b| self.evolution_criteria.compare(a, b))
                                .unwrap()
                        })
                        .collect::<Vec<_>>(),
                )
            })
            .collect::<Vec<_>>();

        // Plot the best individual simulation time (for each category)
        self.evolution_plot_best_ind_with(&mut plot, &best_inds, "x1", "y1", true, |o, _| {
            o.simulation_time as f64
        });

        // Plot the best individual energy consumption (for each category)
        self.evolution_plot_best_ind_with(&mut plot, &best_inds, "x2", "y2", false, |o, _| {
            o.total_consumption as f64
        });

        // Plot the best individual number of machines (for each category)
        self.evolution_plot_best_ind_with(&mut plot, &best_inds, "x3", "y3", false, |o, gen_nb| {
            self.base
                .retrieve_ff_by_individual_name(&o.individual_name, gen_nb as u32)
                .unwrap()
                .count_total_number_nodes() as f64
        });

        // Plot the best individual total GFLOPS (for each category)
        self.evolution_plot_best_ind_with(&mut plot, &best_inds, "x4", "y4", false, |o, gen_nb| {
            let platform = self
                .base
                .retrieve_platform_by_individual_name(&o.individual_name, gen_nb as u32)
                .unwrap();
            // For each hosts in the current platform
            platform
                .zone
                .hosts
                .iter()
                .map(|h| {
                    // Multiply machine flops by its core number
                    h.speed.replace("Gf", "").parse::<f64>().unwrap()
                        * h.core.as_ref().unwrap().parse::<f64>().unwrap()
                })
                .sum::<f64>() // then sum every flops together
        });

        let height = 1500;

        let layout = Layout::new()
            .template(&*PLOTLY_WHITE)
            .x_axis(Axis::new().title(Title::new("Generation number")))
            .y_axis(Axis::new().title(Title::new("Simulation time in seconds")))
            .x_axis2(Axis::new().title(Title::new("Generation number")))
            .y_axis2(Axis::new().title(Title::new("Energy consumption in Joules")))
            .x_axis3(Axis::new().title(Title::new("Generation number")))
            .y_axis3(Axis::new().title(Title::new("Number of machines")))
            .x_axis4(Axis::new().title(Title::new("Generation number")))
            .y_axis4(Axis::new().title(Title::new("Cumulated Gflops in the platform")))
            .height(height)
            .title(Title::new(
                &format!("Evolution simulation, results of the best individual for each category with {} evolution criteria", 
                self.evolution_criteria),
            ))
            .grid(
                LayoutGrid::new()
                    .rows(4)
                    .columns(2)
                    .pattern(GridPattern::Independent),
            );

        plot.set_layout(layout);
        plot.show();

        plot.write_html(format!("{}/out_best_inds.html", self.base.output_dir));
        plot.write_image(
            format!("{}/out_best_inds.ext", self.base.output_dir),
            ImageFormat::SVG,
            1920,
            height,
            1.0,
        );
    }

    pub fn plot_top_individuals(&self) {
        // Separate the outcomes into different categories
        let categories = self
            .outcomes_vec
            .get(0)
            .unwrap()
            .into_iter()
            .unique_by(|o| &o.category)
            .map(|o| o.category.clone())
            .collect::<Vec<_>>();

        let mut plot = Plot::new();

        // Get the top X percent individuals for each generation for each category
        // X is determined by the delete_denominator, which means that we do not take into account
        // mutated individuals that performed worse.
        let top_inds = categories
            .iter()
            // Loop through category
            .map(|category_name| {
                // Return category name and best individuals for each generation of this category
                (
                    category_name,
                    self.outcomes_vec
                        .iter()
                        .map(|outcomes| {
                            let mut tmp = outcomes
                                .iter()
                                // Only select individuals from the current category
                                .filter(|o| &o.category == category_name)
                                // Get the best one according to the defined criteria
                                .collect::<Vec<_>>();

                            // Sort individuals
                            tmp.sort_by(|a, b| self.evolution_criteria.compare(a, b));
                            // NOTE: In fact the plots are better WITHOUT deleting the worse
                            // individuals, because otherwise it is way to close to the plots of
                            // the best individual.
                            //
                            // Delete the last part of the vector that contains the worse
                            // individuals
                            // tmp.truncate(tmp.len() / self.delete_denominator);
                            tmp
                        })
                        .collect::<Vec<_>>(),
                )
            })
            .collect::<Vec<_>>();

        // Plot the mean simulation time of the top individuals (for each category)
        self.evolution_plot_top_ind_with(&mut plot, &top_inds, "x1", "y1", true, |outcomes, _| {
            // Get simulation times of each outcome
            let simulation_times = outcomes
                .iter()
                .map(|o| o.simulation_time)
                .collect::<Vec<_>>();

            // Make the mean
            (simulation_times.iter().sum::<f32>() / simulation_times.len() as f32) as f64
        });

        // Plot the mean total consumption of the top individuals (for each category)
        self.evolution_plot_top_ind_with(&mut plot, &top_inds, "x2", "y2", false, |outcomes, _| {
            // Get total consumptions of each outcome
            let simulation_times = outcomes
                .iter()
                .map(|o| o.total_consumption)
                .collect::<Vec<_>>();

            // Make the mean
            (simulation_times.iter().sum::<f32>() / simulation_times.len() as f32) as f64
        });

        // Plot the mean number of machines in each platforms of the top individuals (for each category)
        self.evolution_plot_top_ind_with(
            &mut plot,
            &top_inds,
            "x3",
            "y3",
            false,
            |outcomes, gen_nb| {
                let nb_nodes = outcomes
                    .iter()
                    .map(|o| {
                        self.base
                            .retrieve_ff_by_individual_name(&o.individual_name, gen_nb as u32)
                            .unwrap()
                            .count_total_number_nodes()
                    })
                    .collect::<Vec<_>>();

                (nb_nodes.iter().sum::<u16>() / nb_nodes.len() as u16) as f64
            },
        );

        // Plot the mean GFLOPS (cumulated) of the top individuals (for each category)
        self.evolution_plot_top_ind_with(
            &mut plot,
            &top_inds,
            "x4",
            "y4",
            false,
            |outcomes, gen_nb| {
                // Vector containing the summed flops of each platforms
                let total_flops_platforms = outcomes
                    .iter()
                    .map(|o| {
                        let platform = self
                            .base
                            .retrieve_platform_by_individual_name(&o.individual_name, gen_nb as u32)
                            .unwrap();
                        // For each hosts in the current platform
                        platform
                            .zone
                            .hosts
                            .iter()
                            .map(|h| {
                                // Multiply machine flops by its core number
                                h.speed.replace("Gf", "").parse::<f64>().unwrap()
                                    * h.core.as_ref().unwrap().parse::<f64>().unwrap()
                            })
                            .sum::<f64>() // then sum every flops together
                    })
                    .collect::<Vec<_>>();

                // Compute the mean number of flops of the platforms of this category
                total_flops_platforms.iter().sum::<f64>() / total_flops_platforms.len() as f64
            },
        );

        let height = 1500;

        let layout = Layout::new()
            .template(&*PLOTLY_WHITE)
            .x_axis(Axis::new().title(Title::new("Generation number")))
            .y_axis(Axis::new().title(Title::new("Simulation time in seconds")))
            .x_axis2(Axis::new().title(Title::new("Generation number")))
            .y_axis2(Axis::new().title(Title::new("Energy consumption in Joules")))
            .x_axis3(Axis::new().title(Title::new("Generation number")))
            .y_axis3(Axis::new().title(Title::new("Number of machines")))
            .x_axis4(Axis::new().title(Title::new("Generation number")))
            .y_axis4(Axis::new().title(Title::new("Cumulated Gflops in the platform")))
            .height(height)
            .title(Title::new(&format!(
                "Evolution simulation, Mean results with {} evolution criteria",
                self.evolution_criteria
            )))
            .grid(
                LayoutGrid::new()
                    .rows(4)
                    .columns(2)
                    .pattern(GridPattern::Independent),
            );

        plot.set_layout(layout);
        plot.show();

        plot.write_html(format!("{}/out_top_inds.html", self.base.output_dir));
        plot.write_image(
            format!("{}/out_top_inds.ext", self.base.output_dir),
            ImageFormat::PNG,
            1920,
            height,
            1.0,
        );
    }

    fn evolution_plot_best_ind_with<F>(
        &self,
        plot: &mut Plot,
        best_ind_per_gen_per_cat: &Vec<(&String, Vec<&Outcome>)>,
        x_axis: &str,
        y_axis: &str,
        show_legend: bool,
        mut f: F,
    ) where
        F: FnMut(&Outcome, usize) -> f64,
    {
        for (category_name, gen_outcomes) in best_ind_per_gen_per_cat {
            let trace = Scatter::new(
                (0..self.outcomes_vec.len()).collect(),
                gen_outcomes
                    .iter()
                    .enumerate()
                    .map(|(gen_nb, o)| {
                        // Apply `f` for each gen best individual
                        f(o, gen_nb)
                    })
                    .collect(),
            )
            .mode(Mode::LinesMarkers)
            .line(self.base.get_line(category_name))
            .marker(self.base.get_marker(category_name))
            .x_axis(x_axis.to_string())
            .y_axis(y_axis.to_string())
            .name(category_name)
            .show_legend(show_legend);

            plot.add_trace(trace);
        }
    }

    fn evolution_plot_top_ind_with<F>(
        &self,
        plot: &mut Plot,
        top_ind_per_gen_per_cat: &Vec<(&String, Vec<Vec<&Outcome>>)>,
        x_axis: &str,
        y_axis: &str,
        show_legend: bool,
        mut f: F,
    ) where
        F: FnMut(&Vec<&Outcome>, usize) -> f64,
    {
        for (category_name, gen_vec_outcomes) in top_ind_per_gen_per_cat {
            let trace = Scatter::new(
                (0..self.outcomes_vec.len()).collect(),
                gen_vec_outcomes
                    .iter()
                    .enumerate()
                    .map(|(gen_nb, outcomes)| f(outcomes, gen_nb))
                    .collect(),
            )
            .mode(Mode::LinesMarkers)
            .line(self.base.get_line(category_name))
            .marker(self.base.get_marker(category_name))
            .x_axis(x_axis.to_string())
            .y_axis(y_axis.to_string())
            .name(category_name)
            .show_legend(show_legend);

            plot.add_trace(trace);
        }
    }

    fn evolution_plot_every_ind_with<F>(
        &self,
        plot: &mut Plot,
        categories: &Vec<String>,
        x_axis: &str,
        y_axis: &str,
        show_legend: bool,
        mut f: F,
    ) where
        F: FnMut(&Outcome) -> f64,
    {
        // Plotting the simulation time of every individuals
        for category_name in categories {
            // Get the outcomes of each generation for the current category only
            let outcomes_cat = self
                .outcomes_vec
                .iter()
                .map(|outcomes| {
                    outcomes
                        .iter()
                        .filter(|o| &o.category == category_name)
                        .collect::<Vec<_>>()
                })
                .collect::<Vec<_>>();

            // Get all the unique names in the outcomes
            let unique_names = outcomes_cat
                .iter()
                .map(|outcomes| {
                    outcomes
                        .iter()
                        .map(|o| o.individual_name.clone())
                        .collect::<Vec<_>>()
                })
                .flatten()
                .unique()
                .collect::<Vec<_>>();

            for ind_name in unique_names {
                // add the outcomes indexes gen_nb
                let mut gen_indexes = vec![];
                let outcomes_ind = outcomes_cat
                    .iter()
                    .enumerate()
                    .map(|(i, outcomes)| {
                        let res = outcomes
                            .iter()
                            .filter(|o| o.individual_name == ind_name)
                            // Call the custom user function to select a field
                            .map(|o| f(o))
                            .collect::<Vec<_>>();

                        if !res.is_empty() {
                            gen_indexes.push(i);
                        }
                        res
                    })
                    .flatten()
                    .collect::<Vec<_>>();

                let trace = Scatter::new(
                    gen_indexes,
                    // For each category, get the number of individuals
                    outcomes_ind,
                )
                .mode(Mode::LinesMarkers)
                .line(self.base.get_line(category_name))
                .marker(self.base.get_marker(category_name))
                .x_axis(x_axis.to_string())
                .y_axis(y_axis.to_string())
                .name(ind_name.split_once("_").unwrap().1)
                .show_legend(show_legend);

                plot.add_trace(trace);
            }
        }
    }
}
