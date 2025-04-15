#!/bin/bash

ATTACK_DIR="$HOME/dataset/attack"
PARAM_LOG="$ATTACK_DIR/attack_parameters.csv"
NONATTACK_PARAMS="$HOME/dataset/nonattack/simulation_parameters.csv"

# Create directory and parameter log
mkdir -p "$ATTACK_DIR"
echo "testcase,ue_list,noise_db,tx_power_db,start_time,end_time" > "$PARAM_LOG"

# Function to generate random double in range [min,max]
random_double() {
    local min=$1
    local max=$2
    echo "$min + $RANDOM * ($max - $min) / 32767" | bc -l
}

# Function to generate random integer in range [min,max]
random_int() {
    local min=$1
    local max=$2
    echo $((RANDOM % (max - min + 1) + min))
}

# Read the nonattack parameters file into an array (skipping header)
mapfile -t nonattack_params < <(tail -n +2 "$NONATTACK_PARAMS")

# Run 550 test cases
for ((i=1; i<=550; i++)); do
    # Get the line corresponding to this test case (array is 0-based)
    line="${nonattack_params[$((i-1))]}"
    
    # Extract n value from the line (second column)
    n=$(echo "$line" | cut -d',' -f2)
    
    # Generate UE list (1-n UEs, each UE numbered 1-n)
    ue_count=$(random_int 1 "$n")
    ue_list=""
    for ((j=1; j<=ue_count; j++)); do
        ue=$(random_int 1 "$n")
        # Ensure no duplicates in UE list
        while [[ "$ue_list" =~ (^|,)$ue(,|$) ]]; do
            ue=$(random_int 1 "$n")
        done
        ue_list+="${ue},"
    done
    ue_list="${ue_list%,}"  # Remove trailing comma

    # Generate random attack parameters
    noise_db=$(printf "%.2f" $(random_double 10 30))
    tx_power=$(printf "%.2f" $(random_double 10 30))
    
    # Generate valid time window
    while true; do
        start=$(printf "%.6f" $(random_double 0 1.7))
        end=$(printf "%.6f" $(random_double 0 2))
        duration=$(echo "$end - $start" | bc -l)
        
        # Ensure duration is between 0.3 and 1.3 seconds and end > start
        if (( $(echo "$duration >= 0.3 && $duration <= 1.3 && $end > $start" | bc -l) )); then
            break
        fi
    done

    # Log parameters
    echo "$i,\"$ue_list\",$noise_db,$tx_power,$start,$end" >> "$PARAM_LOG"
    
    # Create test case directory (if not exists)
    mkdir -p "$ATTACK_DIR/TC$i"
    
    # Copy original files from nonattack to attack directory
    cp -r "$HOME/dataset/nonattack/TC$i"/* "$ATTACK_DIR/TC$i/" 2>/dev/null
    
    # Run attack simulation
    ./attack_siml "$i" "$ue_list" "$noise_db" "$tx_power" "$start" "$end"
    
    echo "Processed test case $i: n=$n, UEs [$ue_list] from $start to $end s"
done

echo "All 550 test cases processed successfully!"
echo "Parameter log saved to: $PARAM_LOG"