#[allow(unused_must_use)]

const BINDINGS_PATH: &str = "src/bridge.rs";

fn main() {
    let _ = cxx_build::bridge(BINDINGS_PATH);
    println!("cargo:rerun-if-changed={}", BINDINGS_PATH);
}
