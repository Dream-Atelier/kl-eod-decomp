#!/usr/bin/env bash
set -e

echo "Initializing tools submodules..."
git submodule update --init

if ! command -v pnpm &> /dev/null; then
  echo "[tools/transmuter] pnpm not found, installing globally..."
  npm install -g pnpm
fi

echo "[tools/transmuter] Installing npm dependencies..."
cd tools/transmuter
pnpm install

echo "[tools/transmuter] Building..."
pnpm run build

echo "[tools/transmuter] Done!"
