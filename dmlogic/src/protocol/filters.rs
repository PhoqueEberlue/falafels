use super::{NodeInfo, NodeRole};

#[derive(Debug, Clone)]
pub enum NodeFilter {
    Trainers,
    Aggregators,
    TrainersAndAggregators,
    Everyone,
}

impl NodeFilter {
    pub fn filter(&self, node_info: &NodeInfo) -> bool {
        match self {
            NodeFilter::Trainers => node_info.role == NodeRole::Trainer,
            NodeFilter::Aggregators => node_info.role == NodeRole::Aggregator || node_info.role == NodeRole::MainAggregator,
            NodeFilter::TrainersAndAggregators => matches!(node_info.role, NodeRole::Trainer | NodeRole::Aggregator),
            NodeFilter::Everyone => true,
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use NodeFilter;

    #[test]
    fn test_trainers_filter() {
        let node_info = NodeInfo {
            name: "Node1".to_string(),
            role: NodeRole::Trainer,
        };

        let trainers_filter = NodeFilter::Trainers;
        assert!(trainers_filter.filter(&node_info));
    }

    #[test]
    fn test_aggregators_filter() {
        let node_info = NodeInfo {
            name: "Node2".to_string(),
            role: NodeRole::Aggregator,
        };

        let aggregators_filter = NodeFilter::Aggregators;
        assert!(aggregators_filter.filter(&node_info));
    }

    #[test]
    fn test_main_aggregator_filter() {
        let node_info = NodeInfo {
            name: "Node3".to_string(),
            role: NodeRole::MainAggregator,
        };

        let aggregators_filter = NodeFilter::Aggregators;
        assert!(aggregators_filter.filter(&node_info));
    }

    #[test]
    fn test_everyone_filter() {
        let node_info = NodeInfo {
            name: "Node4".to_string(),
            role: NodeRole::Trainer,
        };

        let everyone_filter = NodeFilter::Everyone;
        assert!(everyone_filter.filter(&node_info));

        let node_info = NodeInfo {
            name: "Node5".to_string(),
            role: NodeRole::Aggregator,
        };

        assert!(everyone_filter.filter(&node_info));

        let node_info = NodeInfo {
            name: "Node6".to_string(),
            role: NodeRole::MainAggregator,
        };

        assert!(everyone_filter.filter(&node_info));
    }
}
