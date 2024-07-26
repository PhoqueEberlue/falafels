use std::{collections::HashMap, error::Error, fs};

use evolution::EvolutionStudy;
use fryer::structures::{
    common::Constants,
    fried::FriedFalafels,
    platform::Platform,
    raw::{Profiles, RawFalafels},
};
use plotly::common::{DashType, Line, Marker, MarkerSymbol};
use serde::{Deserialize, Serialize};
use varying::VaryingStudy;

use crate::structures::base::Clusters;

pub mod evolution;
pub mod varying;

#[derive(Debug, Serialize, Deserialize)]
pub enum StudyKind {
    Varying(VaryingStudy),
    Evolution(EvolutionStudy),
}

/// Struct containing the necessary inputs for a study
#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct InputFiles {
    pub clusters_path: String,
    pub constants_path: String,
    pub profiles_path: String,
    pub platform_specs: Option<String>,
}

#[derive(Clone, Debug, Serialize, Deserialize)]
pub struct StudyBase {
    pub input_files: InputFiles,
    pub name: String,
    pub output_dir: String,
    pub color_map: HashMap<String, String>
}

impl StudyBase {
    pub fn new(name: String, output_dir: String, input_files: InputFiles) -> StudyBase {
        StudyBase::create_dir_if_not_exists(&output_dir);
        StudyBase::create_dir_if_not_exists(format!("{}/logs", &output_dir));
        StudyBase::create_dir_if_not_exists(format!("{}/fried", &output_dir));
        StudyBase::create_dir_if_not_exists(format!("{}/platform", &output_dir));

        StudyBase {
            name,
            output_dir,
            input_files,
            color_map: StudyBase::init_color_map()
        }
    }

    pub fn init_color_map() -> HashMap<String, String> {
        // Colors for dark mode
        // let colors = vec![
        //     "#636efa", "#EF553B", "#00cc96", "#ab63fa", "#FFA15A", "#19d3f3", "#FF6692", "#B6E880",
        //     "#FF97FF", "#FECB52",
        // ];

        // Colors for white mode
        // let colors = vec![
        //     "#636efa", "#EF553B", "#00cc96", "#ab63fa", "#FFA15A", "#19d3f3", "#FF6692", "#B6E880",
        //     "#FF97FF", "#FECB52",
        // ];

        // Colorblind-friendly colors
        let colors = vec![
            "#a6cee3",
            "#1f78b4",
            "#b2df8a",
            "#33a02c",
        ];

        let mut color_map = HashMap::new();

        color_map.insert("StarSimple".to_string(), colors[0].to_string());
        color_map.insert("StarAsynchronous".to_string(), colors[0].to_string());
        color_map.insert("RingUniSimple".to_string(), colors[1].to_string());
        color_map.insert("RingUniAsynchronous".to_string(), colors[1].to_string());
        color_map.insert("StarStarHierarchical".to_string(), colors[2].to_string());
        color_map.insert("StarStarHierarchicalAsync".to_string(), colors[2].to_string());
        color_map.insert("RingUniRingUniHierarchical".to_string(), colors[3].to_string());
        color_map.insert("RingUniRingUniHierarchicalAsync".to_string(), colors[3].to_string());
        color_map
    }

    fn create_dir_if_not_exists<P: AsRef<std::path::Path>>(path: P) {
        match fs::create_dir(&path) {
            Ok(()) => {}
            // Ignore if directory already exists
            Err(e) if e.kind() == std::io::ErrorKind::AlreadyExists => {}
            Err(e) => panic!("{}", e),
        }
    }

    pub fn export_to_json(&self, study: StudyKind) {
        let content = serde_json::to_string_pretty(&study).unwrap();
        fs::write(format!("{}/study_obj.json", self.output_dir), content).unwrap();
    }

    pub fn load_from_json(study_path: &str) -> StudyKind {
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

    pub fn retrieve_ff_by_individual_name(
        &self,
        name: &String,
        gen_nb: u32,
    ) -> Result<FriedFalafels, Box<dyn Error>> {
        let file_path = format!("{}/fried/GEN-{}-{}.xml", self.output_dir, gen_nb, name);
        let ff_content =
            fs::read_to_string(&file_path).expect(&format!("File not found at: {file_path}"));

        let ff: FriedFalafels = quick_xml::de::from_str(&ff_content)?;

        Ok(ff)
    }

    pub fn retrieve_platform_by_individual_name(
        &self,
        name: &String,
        gen_nb: u32,
    ) -> Result<Platform, Box<dyn Error>> {
        let file_path = format!("{}/platform/GEN-{}-{}.xml", self.output_dir, gen_nb, name);
        let platform_content =
            fs::read_to_string(&file_path).expect(&format!("File not found at: {file_path}"));

        let platform: Platform = quick_xml::de::from_str(&platform_content)?;

        Ok(platform)
    }

    /// Get plotly marker with style infered by the category name
    pub fn get_marker(&self, category_name: &String) -> Marker {
        let color = self.color_map.get(category_name.as_str()).unwrap();
        
        Marker::new().color(color.clone()).symbol(MarkerSymbol::Circle)
    }

    /// Get plotly line with style infered by the category name
    pub fn get_line(&self, category_name: &String) -> Line {
        match category_name.contains("Async") {
            true => {
                Line::new().dash(DashType::DashDot)
            },
            false => {
                Line::new().dash(DashType::Solid)
            }
        }
    }
}
