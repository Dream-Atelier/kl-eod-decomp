#!/bin/bash
set -e

echo "=== Klonoa: Empire of Dreams Decompilation - Setup ==="
echo ""

# Check for required tools
check_tool() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed."
        echo "$2"
        exit 1
    fi
}

check_tool arm-none-eabi-as "Install ARM GNU Toolchain: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads"
check_tool arm-none-eabi-ld "Install ARM GNU Toolchain: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads"
check_tool arm-none-eabi-objcopy "Install ARM GNU Toolchain: https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads"
check_tool make "Install make via your package manager."

# Check for baserom
if [ ! -f baserom.gba ]; then
    echo "Error: baserom.gba not found."
    echo "Place your Klonoa: Empire of Dreams (USA) ROM as baserom.gba in the project root."
    echo "Expected SHA1: a0a298d9dba1ba15d04a42fc2eb35893d1a9569b"
    exit 1
fi

# Verify baserom SHA1
echo "Verifying baserom.gba..."
SHA1_CMD=$(command -v sha1sum || command -v shasum)
ACTUAL_SHA1=$($SHA1_CMD baserom.gba | cut -d' ' -f1)
EXPECTED_SHA1="a0a298d9dba1ba15d04a42fc2eb35893d1a9569b"

if [ "$ACTUAL_SHA1" != "$EXPECTED_SHA1" ]; then
    echo "Error: baserom.gba SHA1 mismatch."
    echo "  Expected: $EXPECTED_SHA1"
    echo "  Got:      $ACTUAL_SHA1"
    echo "Make sure you have the correct USA ROM."
    exit 1
fi
echo "baserom.gba: OK"

# Clean generated files
echo "Cleaning generated files..."
rm -rf asm/matchings asm/nonmatchings asm/libgcc.s asm/rom_header.s build

# Init submodules
echo ""
echo "Initializing submodules..."
git submodule update --init

# Build agbcc (skip if cached and up-to-date)
echo ""
AGBCC_CACHE_FILE="tools/agbcc/.build_cache"
AGBCC_COMMIT=$(git -C tools/agbcc rev-parse HEAD 2>/dev/null || echo "unknown")

if [ -f tools/agbcc/bin/agbcc ] && [ -f "$AGBCC_CACHE_FILE" ] && [ "$(cat "$AGBCC_CACHE_FILE")" = "$AGBCC_COMMIT" ]; then
    echo "agbcc is up-to-date (cached), skipping build."
else
    echo "Building agbcc..."
    cd tools/agbcc
    # build.sh may return non-zero if the agbcc_arm variant fails to compile
    # on newer compilers, but the main agbcc binary will still be built.
    ./build.sh || true
    ./install.sh ../..
    cd ../..

    # Verify agbcc was built
    if [ ! -f tools/agbcc/bin/agbcc ]; then
        echo "Error: agbcc failed to build."
        exit 1
    fi

    echo "$AGBCC_COMMIT" > "$AGBCC_CACHE_FILE"
fi

# Check tcc (ARM Norcroft Thumb C Compiler for m4a)
echo ""
if [ -f tools/tcc/bin/tcc_linux ]; then
    echo "tcc: OK (ARM SDT 2.51 Norcroft Thumb C Compiler)"
else
    echo "Error: tools/tcc/bin/tcc_linux not found."
    echo "The ARM Norcroft tcc compiler is required for m4a decompilation."
    exit 1
fi

# On Linux x86_64, check for 32-bit library support (tcc is a 32-bit binary)
if [ "$(uname -s)" = "Linux" ] && [ "$(uname -m)" = "x86_64" ]; then
    if ! ldconfig -p 2>/dev/null | grep -q "libc.so.6.*i386"; then
        echo "Note: tcc requires 32-bit libraries. Install with:"
        echo "  sudo dpkg --add-architecture i386 && sudo apt install libc6:i386"
    fi
fi

# Set up Python venv for Luvdis
echo ""
echo "Setting up Python environment for Luvdis..."
if [ ! -d .venv ]; then
    python3 -m venv .venv
fi
source .venv/bin/activate
pip install -q "setuptools<81" click click-default-group tqdm 2>/dev/null

# Generate asm/ from ROM
echo ""
echo "Generating assembly from ROM..."
python3 scripts/generate_asm.py

echo ""
echo "=== Setup complete ==="
echo "Run 'make' to build the ROM and verify the match."
