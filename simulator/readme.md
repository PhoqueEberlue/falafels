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

|                        | StarNetworkManager | RingNetworkManager | FullyConnectedNetworkManager |
|------------------------|--------------------|--------------------|------------------------------|
| SimpleAggregator       |         ✔          |         ✔          |              ✔               |
| AsynchronousAggregator |         ✔          |         ✔          |              ✔               |
| HierarchicalAggregator |         ✔          |         ✔          |              ✔               |
| Trainer                |         ✔          |         ✔          |              ✔               |


## Cluster topologies

For now the HierarchicalAggregator can use whatever NetworkManager as a local cluster, but the connection to the central aggregator is made with a StarNetworkManager.
