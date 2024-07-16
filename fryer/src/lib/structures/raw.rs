use super::common::{
    AggregatorType, Arg, ClusterTopology, Constants, NetworkManager, Prop, TrainerType,
};
use serde::Deserialize;

/// Represents <root>...</root>
#[derive(Deserialize, Debug, Clone)]
pub struct RawFalafels {
    pub constants: Constants,
    pub profiles: Profiles,
    #[serde(rename = "cluster")]
    pub clusters: Vec<Cluster>,
    #[serde(rename = "platform-specs", skip_serializing_if = "Option::is_none")]
    pub platform_specs: Option<PlatformSpecs>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct Profiles {
    #[serde(rename = "host-profile")]
    pub host_profiles: Vec<HostProfile>,
    #[serde(rename = "link-profile")]
    pub link_profiles: Vec<LinkProfile>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct HostProfile {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@speed")]
    pub speed: String,
    #[serde(rename = "@core", skip_serializing_if = "Option::is_none")]
    pub core: Option<String>,
    #[serde(rename = "@pstate", skip_serializing_if = "Option::is_none")]
    pub pstate: Option<String>,
    #[serde(rename = "prop")]
    pub props: Vec<Prop>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct HostProfileRef {
    #[serde(rename = "@name")]
    pub name: String,
}

#[derive(Deserialize, Debug, Clone)]
pub struct LinkProfile {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@bandwidth")]
    pub bandwidth: String,
    #[serde(rename = "@latency")]
    pub latency: String,
    #[serde(rename = "@sharing_policy", skip_serializing_if = "Option::is_none")]
    pub sharing_policy: Option<String>,
    #[serde(rename = "prop")]
    pub props: Vec<Prop>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct LinkProfileRef {
    #[serde(rename = "@name")]
    pub name: String,
}

#[derive(Deserialize, Debug, Clone)]
pub struct Cluster {
    #[serde(rename = "@topology")]
    pub topology: ClusterTopology,
    pub trainers: Trainers,
    pub aggregators: Aggregators,
}

#[derive(Deserialize, Debug, Clone)]
pub struct Trainers {
    #[serde(rename = "@number")]
    pub number: u16,
    #[serde(rename = "@type")]
    pub trainer_type: TrainerType,
    #[serde(rename = "host-profile-ref", skip_serializing_if = "Option::is_none")]
    pub host_profiles: Option<Vec<HostProfileRef>>,
    #[serde(rename = "link-profile-ref", skip_serializing_if = "Option::is_none")]
    pub link_profiles: Option<Vec<LinkProfileRef>>,
    #[serde(rename = "network-manager", skip_serializing_if = "Option::is_none")]
    pub network_manager: Option<NetworkManager>,
    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}

#[derive(Deserialize, Debug, Clone)]
pub struct Aggregators {
    #[serde(rename = "@number")]
    pub number: u16,
    #[serde(rename = "@type")]
    pub aggregator_type: AggregatorType,
    #[serde(rename = "host-profile-ref", skip_serializing_if = "Option::is_none")]
    pub host_profiles: Option<Vec<HostProfileRef>>,
    #[serde(rename = "link-profile-ref", skip_serializing_if = "Option::is_none")]
    pub link_profiles: Option<Vec<LinkProfileRef>>,
    #[serde(rename = "network-manager", skip_serializing_if = "Option::is_none")]
    pub network_manager: Option<NetworkManager>,
    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}

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

/// Trait acting as an interface so we can return Profiles of different types and query their name.
pub trait ProfileTrait: Clone {
    fn get_pname(&self) -> &String;
}

impl ProfileTrait for HostProfile {
    fn get_pname(&self) -> &String {
        &self.name
    }
}

impl ProfileTrait for LinkProfile {
    fn get_pname(&self) -> &String {
        &self.name
    }
}

/// Trait acting as an interface so we can get names of different References
pub trait ProfileRefTrait {
    fn get_rname(&self) -> &String;
}

impl ProfileRefTrait for HostProfileRef {
    fn get_rname(&self) -> &String {
        &self.name
    }
}

impl ProfileRefTrait for LinkProfileRef {
    fn get_rname(&self) -> &String {
        &self.name
    }
}
