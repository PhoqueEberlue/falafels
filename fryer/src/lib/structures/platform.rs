use super::common::Prop;
/// Rewrite of the simgrid platform XML definition using serde.
/// See https://simgrid.org/doc/latest/XML_reference.html for full specification
/// Not everything is available yet.
use serde::{Deserialize, Serialize};

#[derive(Debug, Serialize, Deserialize, Clone)]
#[serde(rename = "platform")]
pub struct Platform {
    #[serde(rename = "@version")]
    pub version: String,
    pub zone: Zone,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Zone {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@routing")]
    pub routing: String,
    #[serde(rename = "router")]
    pub routers: Vec<Router>,
    #[serde(rename = "host")]
    pub hosts: Vec<Host>,
    #[serde(rename = "link")]
    pub links: Vec<Link>,
    #[serde(rename = "route")]
    pub routes: Vec<Route>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Router {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@coordinates", skip_serializing_if = "Option::is_none")]
    pub coordinates: Option<String>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Host {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@speed")]
    pub speed: String,
    #[serde(rename = "@core", skip_serializing_if = "Option::is_none")]
    pub core: Option<String>,
    #[serde(rename = "@pstate", skip_serializing_if = "Option::is_none")]
    pub pstate: Option<String>,
    #[serde(rename = "prop")]
    pub props: Vec<Prop>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Link {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@bandwidth")]
    pub bandwidth: String,
    #[serde(rename = "@latency")]
    pub latency: String,
    #[serde(rename = "@sharing_policy", skip_serializing_if = "Option::is_none")]
    pub sharing_policy: Option<String>,
    #[serde(rename = "prop")]
    pub props: Vec<Prop>,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct Route {
    #[serde(rename = "@src")]
    pub src: String,
    #[serde(rename = "@dst")]
    pub dst: String,
    #[serde(rename = "link_ctn")]
    pub link_ctn: LinkContainer,
}

#[derive(Debug, Serialize, Deserialize, Clone)]
pub struct LinkContainer {
    #[serde(rename = "@id")]
    pub id: String,
}
