use fryer::Fryer;

mod structures;
mod fryer;
mod platformer;

fn main() {
    let mut fryer = Fryer::new(Some("../name-champions-lol.txt"));

    let raw_falafels = fryer.load_raw_falafels("../xml/raw-falafels.xml");

    let fried_falafels = fryer.fry(&raw_falafels);

    fryer.write_fried_falafels("../xml/fried-falafels.xml", &fried_falafels);
}
