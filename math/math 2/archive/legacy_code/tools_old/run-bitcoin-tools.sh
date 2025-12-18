#!/bin/bash
# Wrapper script to run Bitcoin tools with correct library path

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LIB_DIR="$(cd "$SCRIPT_DIR/../lib" && pwd)"

# Set library path
export LD_LIBRARY_PATH="$LIB_DIR/recovery_core:$LIB_DIR/recovery_crypto:$LIB_DIR/recovery_network:$LIB_DIR/recovery_signal:$LD_LIBRARY_PATH"

# Determine which tool to run based on script name or first argument
TOOL_NAME=$(basename "$0")

case "$TOOL_NAME" in
    bitcoin-recovery|bitcoin-recovery.sh)
        exec "$SCRIPT_DIR/bitcoin-recovery" "$@"
        ;;
    bitcoin-miner|bitcoin-miner.sh)
        exec "$SCRIPT_DIR/bitcoin-miner" "$@"
        ;;
    bitcoin-network|bitcoin-network.sh)
        exec "$SCRIPT_DIR/bitcoin-network" "$@"
        ;;
    universal-recovery|universal-recovery.sh)
        exec "$SCRIPT_DIR/universal-recovery" "$@"
        ;;
    run-bitcoin-tools.sh)
        # Called directly - first arg is tool name
        if [ $# -lt 1 ]; then
            echo "Usage: $0 <tool> [args...]"
            echo "Tools: bitcoin-recovery, bitcoin-miner, bitcoin-network, universal-recovery"
            exit 1
        fi
        TOOL="$1"
        shift
        exec "$SCRIPT_DIR/$TOOL" "$@"
        ;;
    *)
        echo "Unknown tool: $TOOL_NAME"
        exit 1
        ;;
esac