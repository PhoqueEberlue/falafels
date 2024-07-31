# FaLaFEℓS

Federated Learning Frugality and Efficiency via Simulation (FaLaFEℓS 🧆).

## Project structure

```
├── beagle       Analysis tool making use of the fryer and the simulator
├── diagrams
├── fryer        Tool and library to manage Simgrid platforms and FaLaFEℓS config files
├── simulator    The simulator based on Simgrid
│   ├── cmake    Contains cmake utils to find the libraries
│   ├── doc      Simulator documentation
│   └── src      Simulator source code
└── xml
    ├── fried
    ├── platform
    └── raw
```

## Building

The `beagle` and the `fryer` are built with Cargo which is Rust's package manager.

The `simulator` is developed in C++ and we use CMake to build it. 

You can either use docker and docker-compose to build everything, or build it from source.

### Building in docker

Using docker-compose:

```sh
cd falafels
docker-compose build
```

Two images will be built: `falafels-simulator` that only contains the simulator itself, and `falafels-beagle` that packages the `simulator`, the `fryer` and the `beagle`.

### Building the simulator from source

Download Simgrid directly from their repository and checkout this specific commit:
```sh
git clone https://framagit.org/simgrid/simgrid.git
git checkout 534ecd55c0558492fff7560baa516b78f971f20a
```

Simgrid release 3.35 is not working because of some bugs and features that are only implemented into recent developments.

Take a look at this [page](https://simgrid.org/doc/latest/Installing_SimGrid.html) to download Simgrid dependencies.
Here we only use mandatory ones.

Then compile and install:
```sh
cmake . \
  -DCMAKE_INSTALL_PREFIX=/usr \
  -Denable_compile_optimizations=ON \
  -Denable_lto=OFF \
  -Denable_smpi=OFF \
  -Denable_documentation=OFF \
  -Denable_python=OFF

make -j $(nproc)
make install

# You may have to re-link libraries
sudo ldconfig
```

Build the simulator:
```sh
cd simulator
mkdir build
cd build
cmake ..
make
make install
```

Test a run:
```sh
make && ./main ../xml/simgrid-platform.xml ../xml/fried-falafels.xml
```

### Building the beagle from source

See [Rust installation](https://www.rust-lang.org/tools/install) to install Rust and Cargo.

The following dependencies are required to build the beagle: `pkg-config` `libssl-dev`

```sh
cd beagle
cargo install --path .
```
