/// This library defines the structures of our different XML files in falafels, respectively
/// described in their own module:
/// - raw: structure of raw-falafels files used to generate fried-falafels files
/// - fried: strucure of fried-falafels files used to describe the deployment file of the falafels
/// simulator
/// - common: structures in common between the XML definitions
/// - platform: structure of simgrid platform file, used to describe the network in simgrid

pub mod fried;
pub mod common;
pub mod platform;
pub mod raw;
