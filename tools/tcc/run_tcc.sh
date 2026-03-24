#!/bin/bash
# Run ARM SDT 2.51 tcc (Norcroft Thumb C Compiler) on any platform.
#
# On Linux x86/x86_64: runs the native Linux binary (needs libc6:i386 on x86_64)
# On macOS / other: runs the Windows binary via Wine
#
# Usage: run_tcc.sh [tcc flags] -o output.s input.c
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BIN_DIR="$SCRIPT_DIR/bin"

# Detect platform and run tcc
case "$(uname -s)-$(uname -m)" in
    Linux-x86_64|Linux-i*86)
        exec "$BIN_DIR/tcc_linux" "$@"
        ;;
    *)
        # macOS, ARM Linux, etc. — use Wine
        if ! command -v wine &>/dev/null; then
            echo "Error: Wine is required to run tcc on $(uname -s)-$(uname -m)." >&2
            echo "Install Wine: brew install wine-stable (macOS) or apt install wine (Linux)" >&2
            exit 1
        fi
        # tcc.exe needs tcc.dll and tcc.err in its directory
        cd "$BIN_DIR"
        exec wine ./tcc.exe "$@"
        ;;
esac
