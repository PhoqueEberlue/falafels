## Compile & Run 

Compile
```sh
mkdir build
cd build
cmake ..
make
```

Run
```sh
./main ../../xml/simgrid-platform.xml ../../xml/fried-falafels.xml
```

## Compatibility between algorithms and NetworkManagers

| Roles                  | StarNM | RingNM | FullyConnectedNM | HierarchicalNM |
|------------------------|--------|--------|------------------|----------------|
| SimpleAggregator       |   ✅   |   ✅   |        ✅        |       ✅       |
| AsynchronousAggregator |   ✅   |   ✅   |        ✅        |       ✅       |
| HierarchicalAggregator |   ✅   |   ✅   |        ✅        |       ❌       |
| Trainer                |   ✅   |   ✅   |        ✅        |       ✅       |

### Note on Hierarchical Aggregator/NetworkManager:

Hierarchical determine both a topology (edge servers connected to a main one) and an aggregator algorithm (aggregate local models then send to a main server, wait for local model, then distribute to our cluster).

HierarchicalAggregator is the role assigned on the host acting as edge servers, but its NetworkManager depends on its cluster.
By default the HierarchicalAggregator will create another NetworkManager fully dedicated for the hierarchical network.

The central server that performs aggregation of the models sent by edge servers should however use a non-hierarchical aggregator, but its NetworkManager is Hierarchical because it connects to the HierarchicalNetworkManager of the edge servers.

## Cluster topologies

For now the HierarchicalAggregator can use whatever NetworkManager as a local cluster, but the connection to the central aggregator is made with a StarNetworkManager.
