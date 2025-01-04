#!/bin/bash

# Filename: run_tests.sh

# Ensure the script exits on any unexpected errors
set -e

# Initialize a counter
counter=1

# Loop from 1 to 1000
while [ $counter -le 10000 ]
do
    echo "Run #$counter:"
    
    # Execute the command and capture the exit code
    ./test/test_lock_free_mpsc_queue
    exit_code=$?
    
    # Check if the exit code indicates a segfault (139)
    if [ $exit_code -eq 139 ]; then
        echo "❌ Segmentation fault detected on run #$counter. Stopping further executions."
        echo "Last successful run: #$((counter - 1))"
        exit 1
    elif [ $exit_code -ne 0 ]; then
        echo "⚠️ Program exited with code $exit_code on run #$counter. Stopping further executions."
        exit 1
    else
        echo "✅ Run #$counter completed successfully."
    fi
    
    # Increment the counter
    counter=$((counter + 1))
done

echo "🎉 All 1,000 runs completed successfully!"
