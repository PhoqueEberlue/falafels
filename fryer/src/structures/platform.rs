use serde::{Serialize, Deserialize};

#[derive(Debug, Serialize, Deserialize)]
#[serde(rename = "platform")]
pub struct Platform {
    #[serde(rename = "@version")]
    pub version: String,
    pub zone: Zone,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Zone {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@routing")]
    pub routing: String,
    pub host: Vec<Host>,
    pub link: Vec<Link>,
    pub route: Vec<Route>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Host {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@speed")]
    pub speed: String,
    #[serde(rename = "@core")]
    pub core: Option<String>,
    pub prop: Vec<Prop>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Prop {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@value")]
    pub value: String,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Link {
    #[serde(rename = "@id")]
    pub id: String,
    #[serde(rename = "@bandwidth")]
    pub bandwidth: String,
    #[serde(rename = "@latency")]
    pub latency: String,
    #[serde(rename = "@sharing_policy")]
    pub sharing_policy: Option<String>,
    pub prop: Vec<Prop>,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct Route {
    #[serde(rename = "@src")]
    pub src: String,
    #[serde(rename = "@dst")]
    pub dst: String,
    #[serde(rename = "link_ctn")]
    pub link_ctn: LinkContainer,
}

#[derive(Debug, Serialize, Deserialize)]
pub struct LinkContainer {
    #[serde(rename = "@id")]
    pub id: String,
}
