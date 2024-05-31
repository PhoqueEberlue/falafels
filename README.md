# FaLaFElS

Federated Learning Frugality and Efficiency via Simulation (FaLaFElS 🧆).

README WORK IN PROGRESS

## Project structure

```
├── beagle       Analysis tool making use of the fryer and the simulator
├── diagrams
├── fryer        Tool and library to manage Simgrid platforms and FaLaFElS config files
├── simulator    The simulator based on Simgrid
│   ├── cmake    Contains cmake utils to find the libraries
│   ├── doc      Simulator documentation
│   └── src      Simulator source code
└── xml
    ├── fried
    ├── platform
    └── raw
```

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
./main ../xml/simgrid-platform.xml ../xml/fried-falafels.xml
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

