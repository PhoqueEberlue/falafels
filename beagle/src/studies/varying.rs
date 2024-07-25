use plotly::{
    common::{Anchor, Marker, Title},
    layout::{
        themes::{PLOTLY_DARK, PLOTLY_WHITE},
        Annotation, Axis, GridPattern, LayoutGrid,
    },
    ImageFormat, Layout, Plot, Scatter,
};
use serde::{Deserialize, Serialize};

use std::{collections::HashMap, thread};

use crate::{
    individual_factory::IndividualFactory,
    launcher::{self, Outcome},
};

use super::StudyBase;

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct VaryingStudy {
    pub base: StudyBase,
    pub outcomes_map: HashMap<String, Vec<Outcome>>,
    pub x_axis: Vec<u16>,
}

impl VaryingStudy {
    pub fn new(base: StudyBase) -> VaryingStudy {
        VaryingStudy {
            base,
            outcomes_map: HashMap::new(),
            x_axis: vec![],
        }
    }

    pub fn varying_machines_number_sim(&mut self, step: u16, total_number_gen: u32) {
        let base_rf = self.base.recompose_rf().unwrap();

        let mut factory = IndividualFactory::new(base_rf, &self.base.output_dir);

        // let main_cluster = factory.base_rf.clusters.get_mut(0).unwrap();
        // let args = main_cluster.aggregators.args.get_or_insert_with(|| Vec::new());
        // args.push(Arg { name: "number_local_epochs".to_string(), value: "1".to_string() });

        // let mut number_local_epochs = 1;

        // let start_number = 500;
        // factory.base_rf.clusters.list.get_mut(0).unwrap().trainers.as_mut().unwrap().number = start_number;

        for gen_nb in 0..total_number_gen {
            factory.generation_number = gen_nb as u32;

            let mut individuals = factory.init_individuals();

            let mut handles = vec![];

            for ind in individuals.iter_mut() {
                ind.gen_and_write_platform();

                let output_dir = self.base.output_dir.to_string();
                let mut individual = ind.clone();

                // Launch as much threads as there are individuals
                handles.push(thread::spawn(move || {
                    individual.content.as_mut().unwrap().gen_nb = gen_nb;
                    // Write individual fried file
                    individual.write_fried();

                    let outcome =
                        launcher::run_simulation(gen_nb as u32, output_dir, individual, false);
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
            let current_color = *super::COLOR_MAP.get(algo_topo.as_str()).unwrap();

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
            .title(Title::new(&self.base.name));

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

        plot.write_html(format!("{}/out.html", self.base.output_dir));
        plot.write_image(
            format!("{}/out_white.ext", self.base.output_dir),
            ImageFormat::PNG,
            1920,
            1080,
            1.0,
        );
    }
}
