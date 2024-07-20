use fryer::{
    fryer::Fryer,
    structures::{fried::FriedFalafels, raw::RawFalafels},
};

#[derive(Debug, Clone)]
pub struct Individual {
    // Individual name that describes its specifications (Topo / Algo)
    pub name: String,
    /// The type of algo / topology used
    pub category: String,
    // Generation of the individual
    pub gen_nb: u32,
    // RawFalafels structure which helped creating the Fried one
    pub rf: RawFalafels,
    // FriedFalafels structure
    pub ff: FriedFalafels,
    // Dir path where we should save the FriedFalafels file
    pub ff_dir_path: String,
    //
    pub is_hierarchical: bool,
}

impl Individual {
    /// Writes the FriedFalafels `ff` at `ff_file_path`
    pub fn write_fried(&self) {
        Fryer::write_fried_falafels(&self.get_ff_path(), &self.ff);
    }

    pub fn get_ff_path(&self) -> String {
        format!(
            "{}/GEN-{}-{}.xml",
            self.ff_dir_path, self.gen_nb, self.name
        )
    }
}
