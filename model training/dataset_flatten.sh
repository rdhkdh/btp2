#!/bin/bash

# Source directory containing TC folders
src_dir="dataset/attack"

# Destination directory
dest_dir="dataset/training"

# Create destination directory if it doesn't exist
mkdir -p "$dest_dir"

# Initialize counter
count=1

# Loop through TC1 to TC90 explicitly
for tc_num in {1..90}; do
    tc_folder="$src_dir/TC${tc_num}"
    
    # Check if the folder exists
    if [ -d "$tc_folder" ]; then
        # First process all RSSI files, then all SINR files
        for ue_file in "$tc_folder"/UE*_rssi.csv; do
            # Skip if no files found
            [ -e "$ue_file" ] || continue
            
            # New filenames
            rssi_name="rssi_${count}.csv"
            sinr_name="sinr_${count}.csv"
            
            # Copy RSSI file
            cp "$ue_file" "$dest_dir/$rssi_name"
            
            # Find and copy corresponding SINR file
            sinr_file="${ue_file%_rssi.csv}_sinr.csv"
            if [ -e "$sinr_file" ]; then
                cp "$sinr_file" "$dest_dir/$sinr_name"
            else
                echo "Warning: Missing SINR file for $ue_file"
            fi
            
            # Increment counter
            ((count++))
        done
    else
        echo "Warning: Folder $tc_folder does not exist"
    fi
done

echo "Finished processing. Total UEs processed: $((count-1))"
echo "Expected files: $((2*(count-1))) (rssi + sinr for each UE)"