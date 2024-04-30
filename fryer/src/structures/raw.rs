use serde::Deserialize;
use super::common::{AggregatorType, Arg, Constants, NetworkManager, TrainerType, Prop, ClusterTopology};

/// Represents <root>...</root>
#[derive(Deserialize, Debug)]
pub struct RawFalafels {
    pub constants: Constants,
    pub profiles: Profiles,
    pub clusters: Clusters,
}

#[derive(Deserialize, Debug)]
pub struct Profiles {
    #[serde(rename = "host-profile")]
    pub host_profiles: Vec<HostProfile>,
    #[serde(rename = "link-profile")]
    pub link_profiles: Vec<LinkProfile>,
}

#[derive(Deserialize, Debug)]
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

#[derive(Deserialize, Debug)]
pub struct LinkProfile {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@bandwidth")]
    pub bandwidth: String,
    #[serde(rename = "@latency")]
    pub latency: String,
    #[serde(rename = "@psharing_policy", skip_serializing_if = "Option::is_none")]
    pub sharing_policy: Option<String>,
    #[serde(rename = "prop")]
    pub props: Vec<Prop>,

}

#[derive(Deserialize, Debug)]
pub struct Clusters {
    #[serde(rename = "cluster")]
    pub list: Vec<Cluster>,
}

#[derive(Deserialize, Debug)]
pub struct Cluster {
    #[serde(rename = "@name")]
    pub name: String,
    #[serde(rename = "@topology")]
    pub topology: ClusterTopology,
    pub trainers: Option<Trainers>,
    pub aggregators: Aggregators,
    #[serde(rename = "connected-to")]
    pub connections: Option<Vec<ConnectedTo>>,
}

#[derive(Deserialize, Debug)]
pub struct ConnectedTo {
    #[serde(rename = "@cluster-name")]
    pub cluster_name: String,
}

#[derive(Deserialize, Debug, Clone)]
pub struct Trainers {
    #[serde(rename = "@number")]
    pub number: u16,
    #[serde(rename = "@type")]
    pub trainer_type: TrainerType,
    #[serde(rename = "@host-profiles")]
    pub host_profiles: String,
    #[serde(rename = "@link-profiles")]
    pub link_profiles: String,
    #[serde(rename = "network-manager", skip_serializing_if = "Option::is_none")]
    pub network_manager: Option<NetworkManager>,
    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}

#[derive(Deserialize, Debug)]
pub struct Aggregators {
    #[serde(rename = "@number")]
    pub number: u16,
    #[serde(rename = "@type")]
    pub aggregator_type: AggregatorType,
    #[serde(rename = "@host-profiles")]
    pub host_profiles: String,
    #[serde(rename = "@link-profiles")]
    pub link_profiles: String,
    #[serde(rename = "network-manager", skip_serializing_if = "Option::is_none")]
    pub network_manager: Option<NetworkManager>,
    #[serde(rename = "arg")]
    pub args: Option<Vec<Arg>>,
}
