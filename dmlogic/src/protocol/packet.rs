use super::operations::Operation;
use super::filters::NodeFilter;

pub struct Packet {
    /// Packet's operation, storing potential values corresponding to the variant member
    op: Operation,

    /// Const src and dst
    original_src: String,
    final_dst: String,

    /// Writable src and dst, useful in a peer-to-peer scenario
    src: String,
    dst: String,

    /// Flag indicating if the packet should be broadcasted
    broadcast: bool,

    /// NodeFilter used to check if a Node should route this packet to their Role.
    /// In other words, it tells if a Node is the target for receiving this packet.
    target_filter: Option<NodeFilter>,

    /// Unique packet identifier
    id: u64,

    /// Used in p2p scenario when you want to count the hops from a redirected packet
    nb_hops: u32,

    /// Cache variable to prevent computing the packet's size multiple times
    packet_size: u64,
}

impl Packet {
    /// Clone a packet. Note that pointers in the data variant are also cloned,
    /// thus the pointed value will be accessible both by the cloned packet and the original one.
    pub fn clone(&self) -> Self {
        // Placeholder implementation
        Packet {
            op: self.op.clone(), // Assuming operations::Operation implements Clone
            original_src: self.original_src.clone(),
            final_dst: self.final_dst.clone(),
            src: self.src.clone(),
            dst: self.dst.clone(),
            broadcast: self.broadcast,
            target_filter: self.target_filter.clone(), // Assuming filters::NodeFilter implements Clone
            id: self.id,
            nb_hops: self.nb_hops,
            packet_size: self.packet_size,
        }
    }

    /// Packet constructor both final and intermediate destination, used for peer-to-peer communications with several hops
    pub fn new(dst: String, final_dst: String, op: Operation) -> Self {
        let mut packet = Packet {
            op,
            original_src: String::new(),
            final_dst: final_dst.clone(),
            src: dst.clone(),
            dst,
            broadcast: false,
            target_filter: None,
            id: Packet::generate_packet_id(),
            nb_hops: 0,
            packet_size: 0,
        };
        packet.init();
        packet
    }

    /// Broadcast packet constructor taking NodeFilter instead of concrete destinations
    pub fn new_broadcast(filter: NodeFilter, op: Operation) -> Self {
        let mut packet = Packet {
            op,
            original_src: String::new(),
            final_dst: String::new(),
            src: String::new(),
            dst: String::new(),
            broadcast: true,
            target_filter: Some(filter),
            id: Packet::generate_packet_id(),
            nb_hops: 0,
            packet_size: 0,
        };
        packet.init();
        packet
    }

    /// Compute the simulated size of a packet by following the pointers stored in the data union
    pub fn get_packet_size(&self) -> u64 {
        // Placeholder implementation
        self.packet_size
    }

    /// Get the printable name of Packet's Operation
    pub fn get_op_name(&self) -> &str {
        // Placeholder implementation
        "Operation Name"
    }

    /// Initialize fields of a packet
    fn init(&mut self) {
        // Placeholder implementation
    }

    /// Use to generate new packet ids
    fn generate_packet_id() -> u64 {
        // Placeholder implementation
        // Ideally, this should use a thread-safe counter or atomic operations
        0
    }
}
