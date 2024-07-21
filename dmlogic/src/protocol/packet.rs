use uuid::Uuid;

use super::filters::NodeFilter;
use super::operations::Operation;

#[derive(Clone)]
pub struct Packet {
    /// Packet's operation, storing potential values corresponding to the variant member
    pub op: Operation,

    /// Const src and dst
    pub original_src: String,
    pub final_dst: String,

    /// Writable src and dst, useful in a peer-to-peer scenario
    pub src: String,
    pub dst: String,

    /// Flag indicating if the packet should be broadcasted
    pub broadcast: bool,

    /// NodeFilter used to check if a Node should route this packet to their Role.
    /// In other words, it tells if a Node is the target for receiving this packet.
    pub target_filter: Option<NodeFilter>,

    /// Unique packet identifier using uuid
    pub id: Uuid,

    /// Used in p2p scenario when you want to count the hops from a redirected packet
    pub nb_hops: u32,
}

impl Packet {
    //TODO: rework the constructors because we might want to use filter and src/dst at the same
    //time. We may also hide the src and dst under an interface.
    
    /// Packet constructor both final and intermediate destination, used for peer-to-peer communications with several hops
    pub fn new(dst: String, final_dst: String, op: Operation) -> Packet {
        Packet {
            op,
            original_src: String::new(),
            final_dst: final_dst.clone(),
            src: dst.clone(),
            dst,
            broadcast: false,
            target_filter: None,
            id: Uuid::new_v4(),
            nb_hops: 0,
        }
    }

    /// Broadcast packet constructor taking NodeFilter instead of concrete destinations
    pub fn new_broadcast(filter: NodeFilter, op: Operation) -> Packet {
        Packet {
            op,
            original_src: String::new(),
            final_dst: String::new(),
            src: String::new(),
            dst: String::new(),
            broadcast: true,
            target_filter: Some(filter),
            id: Uuid::new_v4(),
            nb_hops: 0,
        }
    }
}
