#!/usr/bin/env python3
"""
Generate the asm/ directory from baserom.gba.

This script produces all ROM-derived assembly files that are not checked
into version control:
  - asm/rom_header.s
  - asm/libgcc.s
  - asm/nonmatchings/**/*.s
  - data/data.s

It requires:
  - baserom.gba (SHA1: a0a298d9dba1ba15d04a42fc2eb35893d1a9569b)
  - functions_merged.cfg (checked into git)
  - tools/luvdis (git submodule)

Usage:
  python3 scripts/generate_asm.py
"""

import hashlib
import os
import re
import subprocess
import sys
import tomllib

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BASEROM = os.path.join(ROOT, "baserom.gba")
EXPECTED_SHA1 = "a0a298d9dba1ba15d04a42fc2eb35893d1a9569b"
FUNCTIONS_CFG = os.path.join(ROOT, "functions_merged.cfg")
LUVDIS_DIR = os.path.join(ROOT, "tools", "luvdis")
DECOMP_TOML = os.path.join(ROOT, "klonoa-eod-decomp.toml")

CODE_END = 0x52000
DATA_START = 0x52000


def load_config():
    """Load modules and renames from klonoa-eod-decomp.toml."""
    with open(DECOMP_TOML, "rb") as f:
        config = tomllib.load(f)

    modules = [(m["name"], m["start"]) for m in config["modules"]]
    renames = config.get("renames", {})
    return modules, renames


MODULES, RENAMES = load_config()
LIBGCC_START_ADDR = dict(MODULES).get("libgcc", 0x08051868)


def sha1(path):
    h = hashlib.sha1()
    with open(path, "rb") as f:
        for chunk in iter(lambda: f.read(8192), b""):
            h.update(chunk)
    return h.hexdigest()


def validate_baserom():
    if not os.path.exists(BASEROM):
        print(f"ERROR: {BASEROM} not found.", file=sys.stderr)
        print("Place your Klonoa: Empire of Dreams (USA) ROM as baserom.gba", file=sys.stderr)
        sys.exit(1)

    actual = sha1(BASEROM)
    if actual != EXPECTED_SHA1:
        print(f"ERROR: baserom.gba SHA1 mismatch.", file=sys.stderr)
        print(f"  Expected: {EXPECTED_SHA1}", file=sys.stderr)
        print(f"  Got:      {actual}", file=sys.stderr)
        sys.exit(1)

    print(f"  baserom.gba: OK ({EXPECTED_SHA1})")


def run_luvdis():
    """Run Luvdis to disassemble the code section into a monolithic .s file."""
    output = os.path.join(ROOT, "build", "rom_disasm.s")
    os.makedirs(os.path.dirname(output), exist_ok=True)

    cmd = [
        sys.executable, "-m", "luvdis",
        BASEROM,
        "-c", FUNCTIONS_CFG,
        "-o", output,
        "--default-mode", "THUMB",
        "--stop", hex(0x08000000 + CODE_END),
    ]

    env = os.environ.copy()
    env["PYTHONPATH"] = LUVDIS_DIR + os.pathsep + env.get("PYTHONPATH", "")

    print(f"  Running Luvdis (stop=0x{0x08000000 + CODE_END:08X})...")
    result = subprocess.run(cmd, capture_output=True, text=True, cwd=ROOT, env=env)
    if result.returncode != 0:
        print(f"ERROR: Luvdis failed:\n{result.stderr}", file=sys.stderr)
        sys.exit(1)

    return output


def generate_rom_header():
    """Generate asm/rom_header.s from the first 0xC0 bytes of the ROM."""
    output = os.path.join(ROOT, "asm", "rom_header.s")

    with open(BASEROM, "rb") as f:
        header_bytes = f.read(0xC0)

    with open(output, "w") as f:
        f.write('.include "asm/macros.inc"\n')
        f.write(".syntax unified\n.text\n\n")
        f.write("@ GBA ROM Header\n")
        f.write("_08000000:\n")

        for i in range(0, len(header_bytes), 16):
            chunk = header_bytes[i:i + 16]
            hex_bytes = ", ".join(f"0x{b:02X}" for b in chunk)
            f.write(f"\t.byte {hex_bytes}\n")

    print(f"  Generated {output}")


def generate_data_s():
    """Generate data/data.s with .incbin for the data section."""
    output = os.path.join(ROOT, "data", "data.s")
    os.makedirs(os.path.dirname(output), exist_ok=True)

    with open(output, "w") as f:
        f.write('.syntax unified\n.text\n\n')
        f.write('@ Data section\n')
        f.write(f'\t.incbin "baserom.gba", 0x{DATA_START:X}\n')

    print(f"  Generated {output}")


def split_functions(luvdis_output):
    """
    Split the monolithic Luvdis output into:
      - Individual function .s files in asm/nonmatchings/{module}/
      - asm/libgcc.s
    """
    with open(luvdis_output) as f:
        lines = f.readlines()

    # Skip the Luvdis header (macros, .syntax, .text)
    content_start = 0
    for i, line in enumerate(lines):
        stripped = line.strip()
        if stripped and not stripped.startswith("@") and not stripped.startswith("."):
            content_start = i
            break

    # Find all function boundaries
    func_start_re = re.compile(
        r"^\t(thumb_func_start|non_word_aligned_thumb_func_start)\s+(\S+)"
    )
    addr_re = re.compile(r"@ ([0-9A-Fa-f]{8})")
    name_addr_re = re.compile(r"(?:FUN_|sub_|thunk_FUN_|thunk_sub_)([0-9A-Fa-f]{6,8})")

    func_spans = []  # (line_idx, name, addr)
    for i in range(content_start, len(lines)):
        m = func_start_re.match(lines[i])
        if m:
            name = m.group(2)
            # Get address from next line or from name
            addr = 0
            if i + 1 < len(lines):
                am = addr_re.search(lines[i + 1])
                if am:
                    addr = int(am.group(1), 16)
            if addr == 0:
                nm = name_addr_re.search(name)
                if nm:
                    addr = int(nm.group(1), 16)
            func_spans.append((i, name, addr))

    # Determine module for each function
    module_starts = [(addr, name) for name, addr in MODULES]
    module_starts.sort()

    def get_module(addr):
        mod = module_starts[0][1]
        for mod_addr, mod_name in module_starts:
            if addr >= mod_addr:
                mod = mod_name
            else:
                break
        return mod

    # Create output directories
    for mod_name, _ in MODULES:
        if mod_name != "libgcc":
            os.makedirs(os.path.join(ROOT, "asm", "nonmatchings", mod_name), exist_ok=True)

    # Also capture pre-function content per module
    pre_func_content = {}  # module -> lines

    # Split into files
    libgcc_lines = []
    module_funcs = {}  # module -> [(addr, name)]
    func_count = 0

    for idx, (start_line, name, addr) in enumerate(func_spans):
        end_line = func_spans[idx + 1][0] if idx + 1 < len(func_spans) else len(lines)
        func_lines = lines[start_line:end_line]
        module = get_module(addr)

        if module == "libgcc":
            libgcc_lines.extend(func_lines)
            continue

        # Check for pre-function content (data before first function in module)
        if module not in module_funcs:
            module_funcs[module] = []
            # Find content between module start and this function
            if idx > 0:
                prev_end = func_spans[idx - 1][0]
                # Find lines between previous function's start and this one
                # that aren't part of the previous function
            # Check for pre-function data from the start of content
            mod_start_line = content_start if idx == 0 else func_spans[idx - 1][0]
            pre_lines = []
            for j in range(content_start if idx == 0 else end_line, start_line):
                pass  # Complex - handle below

        # Write individual .s file
        nm_dir = os.path.join(ROOT, "asm", "nonmatchings", module)
        s_path = os.path.join(nm_dir, f"{name}.s")
        with open(s_path, "w") as f:
            f.writelines(func_lines)

        module_funcs.setdefault(module, []).append((addr, name))
        func_count += 1

    # Handle pre-function content for each module
    # Find content before the first function that isn't part of any function
    first_func_line = func_spans[0][0] if func_spans else len(lines)
    if first_func_line > content_start:
        pre_lines = lines[content_start:first_func_line]
        if any(l.strip() for l in pre_lines):
            # This goes into the first module's _pre.s
            first_module = get_module(func_spans[0][2]) if func_spans else "system"
            pre_path = os.path.join(ROOT, "asm", "nonmatchings", first_module, "_pre.s")
            with open(pre_path, "w") as f:
                f.writelines(pre_lines)

    # Write libgcc.s
    libgcc_path = os.path.join(ROOT, "asm", "libgcc.s")
    with open(libgcc_path, "w") as f:
        f.write('.include "asm/macros.inc"\n')
        f.write(".syntax unified\n.text\n\n")
        f.write("@ libgcc runtime (thunks, division, modulo)\n")
        f.writelines(libgcc_lines)

    print(f"  Split {func_count} functions into asm/nonmatchings/")
    print(f"  Generated asm/libgcc.s ({len(libgcc_lines)} lines)")

    return module_funcs


def apply_fixups():
    """Apply known assembly fixups for matching."""
    # Fix the bx r0 at 0x0802806C: SBZ bit must be preserved
    # This is in gfx module, in the function whose pool contains address 0x0802806C
    # Find the file and fix it
    target_label = "_0802806C"
    for dirpath, _, filenames in os.walk(os.path.join(ROOT, "asm", "nonmatchings")):
        for fname in filenames:
            fpath = os.path.join(dirpath, fname)
            with open(fpath) as f:
                content = f.read()
            if f"\n{target_label}:\n\tbx r0\n" in content:
                content = content.replace(
                    f"\n{target_label}:\n\tbx r0\n",
                    f"\n{target_label}:\n\t.2byte 0x4704 @ bx r0 (SBZ bit preserved)\n",
                )
                with open(fpath, "w") as f:
                    f.write(content)
                print(f"  Applied SBZ fix in {os.path.relpath(fpath, ROOT)}")
                break
        else:
            continue
        break
    else:
        print("  WARNING: Could not find bx r0 SBZ fixup target")

    # Add .global for labels referenced across compilation units (cross-module branches)
    cross_module_globals = [
        "_080482B4",
        "_0804831C",
    ]
    nm_root = os.path.join(ROOT, "asm", "nonmatchings")
    for label in cross_module_globals:
        for dirpath, _, filenames in os.walk(nm_root):
            for fname in filenames:
                fpath = os.path.join(dirpath, fname)
                with open(fpath) as f:
                    content = f.read()
                if f"\n{label}:\n" in content and f".global {label}" not in content:
                    content = content.replace(
                        f"\n{label}:\n",
                        f"\n.global {label}\n{label}:\n",
                    )
                    with open(fpath, "w") as f:
                        f.write(content)
                    print(f"  Added .global {label} in {os.path.relpath(fpath, ROOT)}")


def apply_renames():
    """Apply function renames defined in klonoa-eod-decomp.toml."""
    for old_name, new_name in RENAMES.items():
        # Rename the .s file
        for module in os.listdir(os.path.join(ROOT, "asm", "nonmatchings")):
            old_path = os.path.join(ROOT, "asm", "nonmatchings", module, f"{old_name}.s")
            new_path = os.path.join(ROOT, "asm", "nonmatchings", module, f"{new_name}.s")
            if os.path.exists(old_path):
                # Rename label inside the file
                with open(old_path) as f:
                    content = f.read()
                content = content.replace(old_name, new_name)
                with open(new_path, "w") as f:
                    f.write(content)
                os.remove(old_path)

        # Update references in all .s files
        nm_root = os.path.join(ROOT, "asm", "nonmatchings")
        for dirpath, _, filenames in os.walk(nm_root):
            for fname in filenames:
                if not fname.endswith(".s"):
                    continue
                fpath = os.path.join(dirpath, fname)
                with open(fpath) as f:
                    content = f.read()
                if old_name in content:
                    content = content.replace(old_name, new_name)
                    with open(fpath, "w") as f:
                        f.write(content)

    if RENAMES:
        print(f"  Applied {len(RENAMES)} function renames")


def move_matched_functions():
    """Move .s files for decompiled functions from nonmatchings/ to matchings/.

    A function is considered matched if its .s file exists in nonmatchings/
    but is NOT referenced by any INCLUDE_ASM call in src/*.c.
    """
    # Collect all INCLUDE_ASM references from src/*.c
    include_asm_re = re.compile(r'INCLUDE_ASM\("asm/nonmatchings/(\w+)",\s*(\w+)\)')
    referenced = set()  # (module, func_name)

    src_dir = os.path.join(ROOT, "src")
    for fname in os.listdir(src_dir):
        if not fname.endswith(".c"):
            continue
        with open(os.path.join(src_dir, fname)) as f:
            for line in f:
                m = include_asm_re.search(line)
                if m:
                    referenced.add((m.group(1), m.group(2)))

    # Walk nonmatchings/ and move unreferenced .s files to matchings/
    nm_root = os.path.join(ROOT, "asm", "nonmatchings")
    match_root = os.path.join(ROOT, "asm", "matchings")
    moved = 0

    for module in os.listdir(nm_root):
        module_dir = os.path.join(nm_root, module)
        if not os.path.isdir(module_dir):
            continue
        for fname in os.listdir(module_dir):
            if not fname.endswith(".s") or fname == "_pre.s":
                continue
            func_name = fname[:-2]  # strip .s
            if (module, func_name) not in referenced:
                # This function was decompiled to C — move to matchings/
                dest_dir = os.path.join(match_root, module)
                os.makedirs(dest_dir, exist_ok=True)
                src_path = os.path.join(module_dir, fname)
                dest_path = os.path.join(dest_dir, fname)
                os.rename(src_path, dest_path)
                moved += 1

    if moved:
        print(f"  Moved {moved} matched functions to asm/matchings/")


def main():
    os.chdir(ROOT)

    print("Generating asm/ from baserom.gba...")
    print()

    print("[1/7] Validating baserom.gba...")
    validate_baserom()

    print("[2/7] Running Luvdis disassembly...")
    luvdis_output = run_luvdis()

    print("[3/7] Generating rom_header.s...")
    generate_rom_header()

    print("[4/7] Splitting functions into nonmatchings/...")
    split_functions(luvdis_output)

    print("[5/7] Applying fixups...")
    apply_fixups()
    apply_renames()

    print("[6/7] Generating data/data.s...")
    generate_data_s()

    print("[7/7] Moving matched functions...")
    move_matched_functions()

    # Cleanup
    os.remove(luvdis_output)

    print()
    print("Done! Run 'make compare' to verify.")


if __name__ == "__main__":
    main()
