FROM ubuntu:latest

RUN apt update && \
    apt upgrade -y && \
    apt install clang cmake libpugixml-dev git libboost-dev libevent-dev -y

# ------------------------------ Simgrid setup ------------------------------
RUN (git clone https://framagit.org/simgrid/simgrid.git)

WORKDIR /simgrid

RUN git reset --hard 534ecd55c0558492fff7560baa516b78f971f20a

RUN cmake . \
  -DCMAKE_INSTALL_PREFIX=/opt/simgrid \
  -Denable_compile_optimizations=ON \
  -Denable_lto=OFF \
  -Denable_smpi=OFF \
  -Denable_documentation=OFF \
  -Denable_python=OFF

RUN make -j $(nproc)
RUN make install

# For some reason include folder isn't generated correctly, so we have to copy it manually
RUN cp -r include/ /opt/simgrid/
RUN ldconfig
# ---------------------------------------------------------------------------

WORKDIR /falafels

COPY CMakeLists.txt .
COPY src/ ./src/
COPY xml/ ./xml/
COPY cmake/ ./cmake/

RUN (mkdir build && cd build && cmake .. && make)

CMD ["build/main", "xml/simgrid-platform.xml", "xml/fried-falafels.xml"]
