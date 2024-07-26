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

See [Rust installation](https://www.rust-lang.org/tools/install) and hit `cargo build` in the respective folders `beagle/` and `fryer/`.

The simulator is developed in C++ so we use docker to build it smoothly. However if you want to build it on your host you only have to deal with installing the correct version of Simgrid.

### Building in docker

Using buildx:

```sh
cd simulator
docker buildx build . -t falafels
```

Then try running
```sh
docker run -ti falafels:latest
```

### Building from source

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
  -DCMAKE_INSTALL_PREFIX=/opt/simgrid \
  -Denable_compile_optimizations=ON \
  -Denable_lto=OFF \
  -Denable_smpi=OFF \
  -Denable_documentation=OFF \
  -Denable_python=OFF

make -j $(nproc)
make install

# You may have to re-link libraries
ldconfig
```

Build the simulator:
```sh
cd falafels
mkdir build
cd build
cmake ..
make
```

Test a run:
```sh
make && ./main ../xml/simgrid-platform.xml ../xml/fried-falafels.xml
```
