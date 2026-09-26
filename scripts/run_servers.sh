#!/usr/bin/env bash
# Starts the whole server side of the simulation: root, every TLD server, every
# authoritative server, and the local resolver. Logs go to logs/<name>.log.
# Ctrl+C stops everything. Run dns_client in a second terminal.
set -euo pipefail

cd "$(dirname "$0")/.."
BIN=build/bin

for exe in root_server tld_server authoritative_server resolver; do
    if [[ ! -x "$BIN/$exe" ]]; then
        echo "Missing $BIN/$exe -- build first: cmake -S . -B build && cmake --build build" >&2
        exit 1
    fi
done

mkdir -p logs
pids=()

start() {
    local name=$1; shift
    "$@" > "logs/$name.log" 2>&1 &
    pids+=($!)
    echo "started $name (pid $!)"
}

stop_all() {
    echo
    echo "stopping servers..."
    kill "${pids[@]}" 2>/dev/null || true
    wait 2>/dev/null || true
}
trap stop_all EXIT INT TERM

start root "$BIN/root_server" root/cfg/root.json
for cfg in tld/cfg/*.json; do
    start "tld-$(basename "$cfg" .json)" "$BIN/tld_server" "$cfg"
done
for cfg in authoritative/cfg/*.json; do
    start "auth-$(basename "$cfg" .json)" "$BIN/authoritative_server" "$cfg"
done
start resolver "$BIN/resolver" resolver/cfg/resolver.json

sleep 0.5
for pid in "${pids[@]}"; do
    if ! kill -0 "$pid" 2>/dev/null; then
        echo "A server exited on startup -- check logs/ (is a port already in use?)" >&2
        exit 1
    fi
done

echo
echo "All servers up. In another terminal run: ./build/bin/dns_client"
echo "Press Ctrl+C to stop."
wait
