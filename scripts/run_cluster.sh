#!/bin/bash
set -e

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_ROOT="$(dirname "$DIR")"
BUILD_DIR="${PROJECT_ROOT}/build/release"
CONFIG_PATH="${PROJECT_ROOT}/config/system_nominal.json"
STATIC_DIR="${PROJECT_ROOT}/src/operator_ui/static"

echo "Using config: $CONFIG_PATH"

mkdir -p "${PROJECT_ROOT}/run_logs"
rm -f "${PROJECT_ROOT}/run_logs/"*

# Array to keep track of PIDs
PIDS=()

function cleanup() {
    echo "Shutting down cluster..."
    for pid in "${PIDS[@]}"; do
        kill -TERM "$pid" 2>/dev/null || true
    done
    exit 0
}

trap cleanup SIGINT SIGTERM

echo "Starting Central Processor..."
"${BUILD_DIR}/central_processor" "$CONFIG_PATH" &
PIDS+=($!)
sleep 0.5

echo "Starting Network Emulator..."
"${BUILD_DIR}/network_emulator" "$CONFIG_PATH" &
PIDS+=($!)
sleep 0.5

echo "Starting 10 Sensor Nodes..."
for i in {0..9}; do
    "${BUILD_DIR}/sensor_node" "$CONFIG_PATH" "sensor_$i" "$i" &
    PIDS+=($!)
done

echo "Starting Operator UI (http://127.0.0.1:8080)..."
"${BUILD_DIR}/operator_ui" "$CONFIG_PATH" "$STATIC_DIR" &
PIDS+=($!)

echo "Cluster is running! Press Ctrl+C to stop."
echo "Visit http://127.0.0.1:8080 in your browser to view the Operator UI."

wait
