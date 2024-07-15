use serde::Deserialize;
use super::raw::{LinkProfileRef, HostProfileRef};


#[derive(Deserialize, Debug, Clone)]
pub struct PlatformSpecs {
    #[serde(rename = "node-profile")]
    pub node_profiles: Vec<NodeProfile>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct NodeProfile {
    #[serde(rename = "@number")]
    pub number: u16,
    #[serde(rename = "host-profile-ref")]
    pub host_profile: HostProfileRef,
    #[serde(rename = "link-profile-ref")]
    pub link_profile: LinkProfileRef,
}
