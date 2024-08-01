docker run -v $(pwd):/io -ti falafels-beagle:latest \
    --simulation-name 10_100_homogeneous_big_workload \
    --output-dir /io/output \
    --constants-path /io/constants.xml \
    --clusters-path /io/clusters.xml \
    --profiles-path /io/profiles.xml \
    varying --step 10 --total-number-gen 10
