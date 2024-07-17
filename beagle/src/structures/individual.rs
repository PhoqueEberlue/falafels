use fryer::{fryer::Fryer, structures::{fried::FriedFalafels, raw::RawFalafels}};

#[derive(Debug, Clone)]
pub struct Individual {
    // Individual name that will be displayed in plots
    pub name: String,
    // RawFalafels structure which helped creating the Fried one
    pub rf: RawFalafels,
    // FriedFalafels structure
    pub ff: FriedFalafels,
    // FriedFalafels file path used to run the simulation of the Individual
    pub ff_path: String,
    //
    pub is_hierarchical: bool,
}

impl Individual {
    pub fn write_fried(&self) {
        Fryer::write_fried_falafels(&self.ff_path, &self.ff);
    }
}
