use fryer::{
    fryer::Fryer,
    platformer::{self, Platformer},
};

fn main() {
    //let mut fryer = Fryer::new(Some("../name-champions-lol.txt"));
    let mut fryer = Fryer::new(None);

    let raw_falafels = fryer.load_raw_falafels("../xml/raw-falafels.xml");

    let fried_falafels = fryer.fry(&raw_falafels);

    Fryer::write_fried_falafels("../xml/fried-falafels.xml", &fried_falafels);

    // Case where we defined the Host and Link profiles directly in the Aggregator and Trainers
    // let raw_and_fried = platformer::RawAndFried { rf: &raw_falafels, ff: &fried_falafels };
    // let mut platformer = Platformer::new(raw_and_fried);

    // Case where we defined the Host and Link profiles in the platform-specs
    let specs_and_profiles = platformer::SpecsAndProfiles {
        specs: &raw_falafels.platform_specs.as_ref().unwrap(),
        profiles: &raw_falafels.profiles,
    };

    let mut platformer = Platformer::new(specs_and_profiles);

    let platform = platformer.create_star_topology();

    platformer.write_platform("../xml/simgrid-platform.xml", &platform);
}
