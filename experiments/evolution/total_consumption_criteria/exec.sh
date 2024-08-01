beagle --simulation-name evolution_total_consumption_criteria \
    --output-dir ./output \
    --constants-path ./constants.xml \
    --clusters-path ./cluster_no_profiles.xml \
    --profiles-path ./profiles.xml \
    --platform-specs ./platform_specs.xml \
    evolution \
    --total-number-gen 15 \
    --evolution-criteria total_consumption
