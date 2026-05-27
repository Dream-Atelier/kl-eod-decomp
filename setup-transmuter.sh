#!/bin/bash
set -e

echo "=== Transmuter Setup ==="
echo ""

echo "Initializing tools/transmuter submodule..."
git submodule update --init tools/transmuter

if ! command -v bun &> /dev/null; then
    echo "[tools/transmuter] bun not found, installing..."
    curl -fsSL https://bun.com/install | bash
    export PATH="$HOME/.bun/bin:$PATH"
fi

if ! command -v pnpm &> /dev/null; then
    echo "[tools/transmuter] pnpm not found, installing globally..."
    bun install -g pnpm
fi

echo "[tools/transmuter] Installing dependencies..."
(cd tools/transmuter && pnpm install)

echo "[tools/transmuter] Building..."
(cd tools/transmuter && pnpm run build)

if [ ! -f tools/transmuter/packages/cli/dist/index.js ]; then
    echo "Error: transmuter failed to build."
    exit 1
fi

echo ""
echo "=== Transmuter ready ==="
echo "Invoke with: bun tools/transmuter/packages/cli/dist/index.js ..."
