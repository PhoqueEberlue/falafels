docker run -v $(pwd):/io -ti falafels-beagle:latest \
    --simulation-name 10_100_heterogeneous_big_workload_async_threshold_0.9 \
    --output-dir /io/output \
    --constants-path /io/constants.xml \
    --clusters-path /io/clusters.xml \
    --profiles-path /io/profiles.xml \
    varying --step 10 --total-number-gen 10
