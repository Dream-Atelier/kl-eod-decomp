#!/usr/bin/env python3
"""
Detect data regions embedded in GBA ROM code using Ghidra headless analysis.

Runs Ghidra's ARM Constant Reference Analyzer and Decompiler Switch Analysis
to identify jump tables, literal pools, and other non-executable data within
the code section.  Results are written as [[data_regions]] entries to a
decomp TOML config file.

This script is game-agnostic — it works with any GBA ROM that uses Thumb code.

Usage:
  python3 scripts/detect_data_regions.py \\
    --ghidra /path/to/ghidra \\
    --rom baserom.gba \\
    --config decomp.toml \\
    --code-end 0x52000 \\
    --functions functions_merged.cfg
"""

import argparse
import os
import re
import subprocess
import sys
import tempfile
import tomllib

# ---------------------------------------------------------------------------
# Ghidra Jython script (embedded)
# ---------------------------------------------------------------------------

GHIDRA_SCRIPT = r'''
# @category Analysis
# @runtime Jython

import json, os, sys

from ghidra.program.model.listing import Data
from ghidra.program.model.address import AddressSet
from ghidra.app.cmd.disassemble import ArmDisassembleCommand

program  = getCurrentProgram()
listing  = program.getListing()
memory   = program.getMemory()
space    = program.getAddressFactory().getDefaultAddressSpace()

def a(offset):
    return space.getAddress(offset)

# Read parameters from environment
CODE_END_ROM   = int(os.environ.get("GHIDRA_CODE_END", "0x52000"), 0)
ENTRIES_FILE   = os.environ.get("GHIDRA_ENTRIES_FILE", "")
OUTPUT_FILE    = os.environ.get("GHIDRA_OUTPUT_FILE", "")
ROM_BASE       = 0x08000000   # GBA ROM mapping

# BinaryLoader places the file at 0x00000000
LOAD_OFFSET = 0
CODE_START_LOAD = 0x000000C0              # ARM entry (after GBA header)
CODE_END_LOAD   = CODE_END_ROM            # code section end (file-relative)

# ---- Step 1: seed analysis with entry points ----

print("[detect_data_regions] Seeding ARM entry at 0x%X" % CODE_START_LOAD)
cmd = ArmDisassembleCommand(a(CODE_START_LOAD),
        AddressSet(a(CODE_START_LOAD), a(CODE_START_LOAD + 0x200)), True)
cmd.applyTo(program)
createFunction(a(CODE_START_LOAD), "start_vector")

# Read Thumb entry points from a file (one hex address per line)
if ENTRIES_FILE and os.path.isfile(ENTRIES_FILE):
    with open(ENTRIES_FILE) as f:
        lines = f.read().splitlines()
    count = 0
    for line in lines:
        line = line.strip()
        if not line or line.startswith("#"):
            continue
        try:
            rom_addr = int(line, 0)
        except ValueError:
            continue
        load_addr = rom_addr - ROM_BASE
        if load_addr < 0 or load_addr >= CODE_END_LOAD:
            continue
        try:
            program.getProgramContext().setValue(
                program.getProgramContext().getRegister("TMode"),
                a(load_addr), a(load_addr), 1)
            disassemble(a(load_addr))
            createFunction(a(load_addr), None)
            count += 1
        except:
            pass
    print("[detect_data_regions] Added %d Thumb entry points" % count)
else:
    print("[detect_data_regions] No entries file, using auto-analysis only")

# ---- Step 2: run full analysis ----

print("[detect_data_regions] Running analysis...")
analyzeAll(program)
print("[detect_data_regions] Analysis complete")

# ---- Step 3: collect data regions ----

data_regions = []
current_start = None
current_size  = 0

cu_iter = listing.getCodeUnits(a(CODE_START_LOAD), True)
while cu_iter.hasNext():
    cu = cu_iter.next()
    addr_val = cu.getMinAddress().getOffset()
    if addr_val >= CODE_END_LOAD:
        break

    is_data = isinstance(cu, Data)

    if is_data:
        if current_start is None:
            current_start = addr_val
            current_size  = cu.getLength()
        else:
            # extend contiguous region
            expected = current_start + current_size
            if addr_val == expected:
                current_size += cu.getLength()
            else:
                if current_size >= 4:
                    data_regions.append((current_start, current_size))
                current_start = addr_val
                current_size  = cu.getLength()
    else:
        if current_start is not None:
            if current_size >= 4:
                data_regions.append((current_start, current_size))
            current_start = None
            current_size  = 0

if current_start is not None and current_size >= 4:
    data_regions.append((current_start, current_size))

# Convert to ROM addresses
rom_regions = [(s + ROM_BASE, sz) for s, sz in data_regions]

print("[detect_data_regions] Found %d data regions (>= 4 bytes)" % len(rom_regions))

# ---- Step 4: write output ----

if OUTPUT_FILE:
    with open(OUTPUT_FILE, "w") as f:
        json.dump(rom_regions, f)
    print("[detect_data_regions] Wrote %s" % OUTPUT_FILE)
else:
    for start, size in rom_regions:
        print("  0x%08X %d" % (start, size))
'''

# ---------------------------------------------------------------------------
# Functions config parser
# ---------------------------------------------------------------------------


def _parse_functions_cfg(path: str) -> list[int]:
    """Extract Thumb function addresses from a functions_merged.cfg file."""
    addrs = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split()
            if len(parts) >= 2 and parts[0] in ("thumb_func", "arm_func"):
                try:
                    addrs.append(int(parts[1], 0))
                except ValueError:
                    pass
    return addrs


def _parse_toml_modules(path: str) -> list[int]:
    """Extract module start addresses from a decomp TOML config."""
    with open(path, "rb") as f:
        config = tomllib.load(f)
    return [m["start"] for m in config.get("modules", [])]


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------


def main():
    parser = argparse.ArgumentParser(
        description="Detect data regions in a GBA ROM using Ghidra analysis."
    )
    parser.add_argument("--ghidra", required=True,
                        help="Path to the Ghidra installation directory")
    parser.add_argument("--rom", required=True,
                        help="Path to the GBA ROM file")
    parser.add_argument("--config", required=True,
                        help="Path to the decomp TOML config file (will be updated)")
    parser.add_argument("--code-end", default="0x52000",
                        help="End of the code section as a ROM offset (default: 0x52000)")
    parser.add_argument("--functions",
                        help="Path to functions_merged.cfg (for Thumb entry points)")
    args = parser.parse_args()

    ghidra_dir = os.path.abspath(args.ghidra)
    rom_path = os.path.abspath(args.rom)
    config_path = os.path.abspath(args.config)
    code_end = args.code_end

    analyze_headless = os.path.join(ghidra_dir, "support", "analyzeHeadless")
    if not os.path.isfile(analyze_headless):
        print(
            f"ERROR: analyzeHeadless not found at {analyze_headless}", file=sys.stderr)
        sys.exit(1)

    if not os.path.isfile(rom_path):
        print(f"ERROR: ROM not found: {rom_path}", file=sys.stderr)
        sys.exit(1)

    # ---- Collect Thumb entry points ----
    entry_addrs: list[int] = []
    if args.functions and os.path.isfile(args.functions):
        entry_addrs = _parse_functions_cfg(args.functions)
        print(f"  Read {len(entry_addrs)} entry points from {args.functions}")
    elif os.path.isfile(config_path):
        entry_addrs = _parse_toml_modules(config_path)
        print(f"  Read {len(entry_addrs)} module starts from {config_path}")

    with tempfile.TemporaryDirectory(prefix="ghidra-detect-") as tmpdir:
        # Write entry points to a temp file
        entries_file = os.path.join(tmpdir, "entries.txt")
        with open(entries_file, "w") as f:
            for addr in entry_addrs:
                f.write(f"0x{addr:08X}\n")

        # Write the Ghidra script
        script_file = os.path.join(tmpdir, "detect_data.py")
        with open(script_file, "w") as f:
            f.write(GHIDRA_SCRIPT)

        # Output file for results
        output_file = os.path.join(tmpdir, "regions.json")

        # Project directory
        project_dir = os.path.join(tmpdir, "project")
        os.makedirs(project_dir)

        # Run Ghidra headless
        env = os.environ.copy()
        env["GHIDRA_CODE_END"] = code_end
        env["GHIDRA_ENTRIES_FILE"] = entries_file
        env["GHIDRA_OUTPUT_FILE"] = output_file

        cmd = [
            analyze_headless,
            project_dir, "detect_data",
            "-import", rom_path,
            "-processor", "ARM:LE:32:v4t",
            "-postScript", script_file,
            "-deleteProject",
        ]

        print(f"  Running Ghidra headless analysis...")
        result = subprocess.run(cmd, capture_output=True, text=True, env=env)

        # Print script output (filtered)
        for line in result.stdout.split("\n"):
            if "[detect_data_regions]" in line:
                print(f"  {line.strip()}")

        if result.returncode != 0:
            print("ERROR: Ghidra analysis failed", file=sys.stderr)
            # Show last few lines of output
            for line in result.stdout.split("\n")[-10:]:
                print(f"  {line}", file=sys.stderr)
            sys.exit(1)

        if not os.path.isfile(output_file):
            print("ERROR: Ghidra script did not produce output", file=sys.stderr)
            sys.exit(1)

        # ---- Read results ----
        import json
        with open(output_file) as f:
            raw_regions = json.load(f)

        print(f"  Ghidra found {len(raw_regions)} data regions")

        # ---- Filter: remove regions that are outside the code section ----
        rom_base = 0x08000000
        code_end_addr = rom_base + int(code_end, 0)
        code_start_addr = rom_base + 0xC0

        regions = [
            (start, size)
            for start, size in raw_regions
            if code_start_addr <= start < code_end_addr and size >= 4
        ]
        print(
            f"  {len(regions)} regions in code section (0x{code_start_addr:08X}-0x{code_end_addr:08X})")

        # ---- Filter: remove regions that Luvdis already handles ----
        # Luvdis correctly emits .4byte for literal pools at labeled addresses
        # (like _08005CF0: .4byte 0x030051C4).  We only need regions that
        # Luvdis decoded as instructions.
        #
        # Heuristic: if a region starts at a 4-byte aligned address and its
        # first word is a valid pointer (0x02-0x08 prefix), it's a jump table
        # or pointer array that Luvdis may have missed.  Keep it.
        # Small 4-byte or 8-byte regions are almost always literal pool entries
        # that Luvdis handles correctly — skip them.
        filtered = [
            (start, size)
            for start, size in regions
            if size > 8  # skip tiny regions (likely already .4byte in source)
        ]
        print(f"  {len(filtered)} regions after filtering (> 8 bytes)")

        # ---- Filter: remove regions that overlap with known function entries ----
        # Ghidra sometimes marks function bodies as data when it doesn't
        # recognise the prologue (e.g. leaf functions without push {lr}).
        # A data region that contains a known function entry point is invalid.
        # Include both cfg entries AND renamed functions from the TOML config.
        func_addrs = set(entry_addrs)
        if os.path.isfile(config_path):
            with open(config_path, "rb") as f:
                toml_cfg = tomllib.load(f)
            for old_name in toml_cfg.get("renames", {}):
                m = re.match(r"(?:FUN_|sub_)([0-9A-Fa-f]+)", old_name)
                if m:
                    func_addrs.add(int(m.group(1), 16))
        safe = []
        removed = 0
        for start, size in filtered:
            end = start + size
            overlap = any(start <= fa < end for fa in func_addrs)
            if overlap:
                removed += 1
            else:
                safe.append((start, size))
        if removed:
            print(
                f"  Removed {removed} regions overlapping known function entries")
        filtered = safe

        # ---- Write to TOML ----
        _update_toml(config_path, filtered)
        print(
            f"  Updated {config_path} with {len(filtered)} data_regions entries")


def _update_toml(config_path: str, regions: list[tuple[int, int]]):
    """Update the decomp TOML config with [[data_regions]] entries.

    Replaces any existing [[data_regions]] section.  Preserves all other
    content (modules, renames, etc.) by doing a text-level splice.
    """
    with open(config_path) as f:
        content = f.read()

    # Remove existing [[data_regions]] section if present
    content = re.sub(
        r'\n*# Data regions within the code section\.\n'
        r'# Generated by scripts/detect_data_regions\.py.*?'
        r'(?=\n\[(?!\[data_regions\])|\n# [A-Z]|\Z)',
        '',
        content,
        flags=re.DOTALL,
    )
    # Also remove any stray [[data_regions]] entries
    content = re.sub(
        r'\[\[data_regions\]\]\nstart = 0x[0-9A-Fa-f]+\nsize = \d+\n*', '', content)

    # Build new section
    lines = [
        "",
        "# Data regions within the code section.",
        "# Generated by scripts/detect_data_regions.py using Ghidra analysis.",
        "# These are jump tables, literal pools, or other non-executable data",
        "# that should be written as .4byte/.2byte, not as instruction mnemonics.",
        "",
    ]
    for start, size in sorted(regions):
        lines.append("[[data_regions]]")
        lines.append(f"start = 0x{start:08X}")
        lines.append(f"size = {size}")
        lines.append("")

    content = content.rstrip() + "\n" + "\n".join(lines)

    with open(config_path, "w") as f:
        f.write(content)


if __name__ == "__main__":
    main()
