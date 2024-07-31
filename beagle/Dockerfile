# Build from simulator image
FROM falafels-simulator:latest

RUN apt install curl pkg-config libssl-dev -y
# Rust install
RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
ENV PATH="/root/.cargo/bin:$PATH"

WORKDIR /falafels/beagle
COPY ./beagle/src/ ./src
COPY ./beagle/Cargo.toml ./
COPY ./beagle/Cargo.lock ./

WORKDIR /falafels/fryer
COPY ./fryer/src ./src
COPY ./fryer/Cargo.toml ./
COPY ./fryer/Cargo.lock ./

RUN cargo install --path /falafels/beagle 

ENTRYPOINT ["beagle"]
