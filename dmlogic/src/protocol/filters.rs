use crate::bridge::ffi::{NodeInfo, RoleEnum};

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
            NodeFilter::Trainers => node_info.role == RoleEnum::Trainer,
            NodeFilter::Aggregators => {
                node_info.role == RoleEnum::Aggregator || node_info.role == RoleEnum::MainAggregator
            }
            NodeFilter::TrainersAndAggregators => {
                matches!(node_info.role, RoleEnum::Trainer | RoleEnum::Aggregator)
            }
            NodeFilter::Everyone => true,
        }
    }
}

// TODO: review and add some tests because it has been generated with ChatGPT
#[cfg(test)]
mod tests {
    use super::*;
    use NodeFilter;

    #[test]
    fn test_trainers_filter() {
        let node_info = NodeInfo {
            name: "Node1".to_string(),
            role: RoleEnum::Trainer,
        };

        let trainers_filter = NodeFilter::Trainers;
        assert!(trainers_filter.filter(&node_info));
    }

    #[test]
    fn test_aggregators_filter() {
        let node_info = NodeInfo {
            name: "Node2".to_string(),
            role: RoleEnum::Aggregator,
        };

        let aggregators_filter = NodeFilter::Aggregators;
        assert!(aggregators_filter.filter(&node_info));
    }

    #[test]
    fn test_main_aggregator_filter() {
        let node_info = NodeInfo {
            name: "Node3".to_string(),
            role: RoleEnum::MainAggregator,
        };

        let aggregators_filter = NodeFilter::Aggregators;
        assert!(aggregators_filter.filter(&node_info));
    }

    #[test]
    fn test_everyone_filter() {
        let node_info = NodeInfo {
            name: "Node4".to_string(),
            role: RoleEnum::Trainer,
        };

        let everyone_filter = NodeFilter::Everyone;
        assert!(everyone_filter.filter(&node_info));

        let node_info = NodeInfo {
            name: "Node5".to_string(),
            role: RoleEnum::Aggregator,
        };

        assert!(everyone_filter.filter(&node_info));

        let node_info = NodeInfo {
            name: "Node6".to_string(),
            role: RoleEnum::MainAggregator,
        };

        assert!(everyone_filter.filter(&node_info));
    }
}
