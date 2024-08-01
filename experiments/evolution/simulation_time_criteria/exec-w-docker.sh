docker run -v $(pwd):/io -ti falafels-beagle:latest \
    --simulation-name evolution_simulation_time_criteria \
    --output-dir /io/output \
    --constants-path /io/constants.xml \
    --clusters-path /io/cluster_no_profiles.xml \
    --profiles-path /io/profiles.xml \
    --platform-specs /io/platform_specs.xml \
    evolution \
    --total-number-gen 15 \
    --evolution-criteria simulation_time
