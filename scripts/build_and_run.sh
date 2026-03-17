#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT_DIR"

cmake -S . -B build
cmake --build build -j
./build/frogquant_market_app
ctest --test-dir build --output-on-failure
