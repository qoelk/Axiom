#!/bin/bash

# Resolve script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# === Step 1: Start server and wait a moment for it to initialize ===
echo "Starting server..."
(cd "$SCRIPT_DIR/server" && go run cmd/server/main.go) &
SERVER_PID=$!

# Optional: give server a moment to bind ports, etc.
sleep 20

# === Step 2: Start player and bots ===
echo "Starting player..."
(cd "$SCRIPT_DIR/player" && python3 player.py) &
PLAYER_PID=$!

echo "Starting bot..."
(cd "$SCRIPT_DIR/bots" && python3 test.py) &
BOT_PID=$!

# === Cleanup function ===
cleanup() {
    echo
    echo "Shutting down..."
    # Kill in reverse order (optional)
    kill $BOT_PID $PLAYER_PID $SERVER_PID 2>/dev/null
    wait $BOT_PID $PLAYER_PID $SERVER_PID 2>/dev/null
    echo "Done."
    exit 0
}

# Trap signals
trap cleanup SIGINT SIGTERM

# Wait for any of the child processes to exit
# (If one crashes, the script exits and cleanup runs)
wait $SERVER_PID $PLAYER_PID $BOT_PID