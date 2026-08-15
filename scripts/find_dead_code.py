#!/usr/bin/env python3
"""
Find unreachable functions in the ROM.

Builds a whole-cartridge reference graph and reports every named function that
nothing can ever reach.  A function is reported only when none of the following
finds a way in:

  * no branch of any form (bl / blx / b / bCC) anywhere in the code region,
  * no ARM B/BL inside a `bx pc; nop; b <arm>` interwork thunk (objdump renders
    those as a raw .word, because it is in Thumb mode at that address),
  * no `adr rN, X; bx rN` inline Thumb->ARM mode switch,
  * no literal-pool or jump-table word holding the address,
  * no pointer at ANY byte alignment anywhere in the 4 MB cartridge.

The last check is what makes the result trustworthy: a callback installed
through a table in data/ is still a reference, and scanning unaligned as well
means a stored pointer cannot hide from it.

Known gap: an address the game *computes* rather than stores — a base plus a
runtime offset, or a table of halfword deltas — is not modelled, so a function
reachable only that way would be reported dead.  Every other approximation here
errs the safe way (it over-counts references), so treat the output as a
shortlist to confirm by hand rather than as proof on its own.

Inventory comes from the linked ELF's symbol table rather than from
functions_merged.cfg, because the cfg both misses real functions (they get
merged into a neighbour's extent) and invents ones that are really literal
pools.  Entries are kept only when they look like genuine function starts:
a prologue (push / stmdb / sub sp), or a leaf that reaches its return before
it reaches any data, and never a target the previous instruction falls into.

Usage:
  python3 scripts/find_dead_code.py
  python3 scripts/find_dead_code.py --json      # machine-readable

Requires a built klonoa-eod.elf (run `make` first).
"""

import argparse
import bisect
import collections
import json
import os
import re
import struct
import subprocess
import sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
ELF = os.path.join(ROOT, "klonoa-eod.elf")
ROM = os.path.join(ROOT, "baserom.gba")
CONFIG = os.path.join(ROOT, "klonoa-eod-decomp.toml")

ROM_BASE = 0x08000000
CODE_END = 0x08052000          # end of asm/libgcc.o(.text); data/ follows
CROSS = os.environ.get("CROSS", "arm-none-eabi-")
OBJDUMP = os.environ.get("OBJDUMP", CROSS + "objdump")
NM = os.environ.get("NM", CROSS + "nm")

DATA_DIRECTIVES = (".word", ".short", ".byte")
INSN_RE = re.compile(r"^\s*([0-9a-f]+):\s+([0-9a-f ]+)\t(\S+)\s*(.*)$")
ADR_RE = re.compile(r"\(adr\s+\w+,\s*([0-9a-f]+)")
LOCAL_LABEL_RE = re.compile(r"^_0[0-9A-F]{7}$")


def disassemble():
    """Disassemble the code region.  objdump honours the assembler's mapping
    symbols, so literal pools come back as .word rather than as bogus code."""
    out = subprocess.run(
        [OBJDUMP, "-d", "--start-address=0x%X" % ROM_BASE,
         "--stop-address=0x%X" % CODE_END, ELF],
        capture_output=True, text=True, check=True).stdout
    by, order = {}, []
    for line in out.splitlines():
        m = INSN_RE.match(line)
        if not m:
            continue
        addr = int(m.group(1), 16)
        width = len(m.group(2).replace(" ", "")) // 2
        by[addr] = (m.group(3), m.group(4).strip(), width)
        order.append(addr)
    order.sort()
    return by, order


def load_symbols():
    """Named function symbols in the code region, minus luvdis' intra-function
    `_0XXXXXXX` labels and the interwork call-via thunks."""
    out = subprocess.run([NM, ELF],
                         capture_output=True, text=True, check=True).stdout
    named = collections.defaultdict(set)
    for line in out.splitlines():
        m = re.match(r"^([0-9a-f]{8}) ([tTwW]) (\S+)$", line)
        if not m:
            continue
        addr, name = int(m.group(1), 16), m.group(3)
        if not (ROM_BASE <= addr < CODE_END):
            continue
        if LOCAL_LABEL_RE.match(name) or name == ".gcc2_compiled." \
                or name.startswith("_call_via"):
            continue
        named[addr].add(name)
    return named


def terminates(mnem, ops):
    """Does this instruction end a basic block unconditionally?"""
    return (mnem in DATA_DIRECTIVES or mnem in ("bx", "b", "b.n", "b.w")
            or (mnem in ("pop", "ldmia") and "pc" in ops)
            or (mnem.startswith("mov") and ops.startswith("pc")))


def is_function_entry(addr, by, order):
    head = by.get(addr)
    if head is None or head[0] in DATA_DIRECTIVES:
        return False                      # the address is data, not code

    i = bisect.bisect_left(order, addr) - 1
    if i >= 0:
        pm, po, _ = by[order[i]]
        if not terminates(pm, po):
            return False                  # fallen into: a mid-function split

    if head[0] in ("push", "stmdb") or (head[0] == "sub" and head[1].startswith("sp")):
        return True                       # prologue

    # Otherwise accept only a leaf that returns before it reaches any data.
    # A literal-pool word that objdump happens to render as an instruction
    # runs into more .word/.short within an instruction or two, never having
    # returned, and is rejected here.
    j = bisect.bisect_left(order, addr)
    while j < len(order):
        mnem, ops, _ = by[order[j]]
        if mnem in DATA_DIRECTIVES:
            return False
        if mnem == "bx" or (mnem in ("pop", "ldmia") and "pc" in ops) \
                or (mnem.startswith("mov") and ops.startswith("pc")):
            return True
        j += 1
    return False


def collect_references(by, order):
    """Count every way control can arrive at each address."""
    ref = collections.Counter()
    for addr in order:
        mnem, ops, _ = by[addr]
        if mnem[0] == "b" and not mnem.startswith("bic"):
            m = re.match(r"^(?:0x)?([0-9a-f]+)\b", ops.split(", ")[-1])
            if m:
                target = int(m.group(1), 16)
                # Only a self-loop at the identical address is not a reference;
                # a branch from elsewhere inside an over-large extent still is.
                if target != addr:
                    ref[target] += 1
        elif mnem == ".word":
            value = int(ops.split()[0], 16)
            ref[value - 1 if value & 1 else value] += 1
            if (value >> 24) in (0xEA, 0xEB):
                imm = value & 0xFFFFFF
                imm -= 0x1000000 if imm & 0x800000 else 0
                ref[(addr + 8 + imm * 4) & 0xFFFFFFFF] += 1
        m = ADR_RE.search(ops)
        if m:
            ref[int(m.group(1), 16)] += 1
    return ref


def load_modules():
    modules = [(0x08000000, "rom_header"), (0x080000C0, "crt0")]
    with open(CONFIG) as f:
        text = f.read()
    for m in re.finditer(r'\[\[modules\]\]\s*\nname = "([^"]+)"\s*\nstart = (0x[0-9A-Fa-f]+)',
                         text):
        modules.append((int(m.group(2), 16), m.group(1)))
    modules.sort()
    return modules


def main():
    ap = argparse.ArgumentParser(description=__doc__,
                                 formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("--json", action="store_true", help="emit JSON instead of a table")
    args = ap.parse_args()

    for path in (ELF, ROM):
        if not os.path.exists(path):
            sys.exit("missing %s — run `make` first" % os.path.relpath(path, ROOT))

    # A stale ELF gives silently wrong answers, so say so rather than lie.
    elf_mtime = os.path.getmtime(ELF)
    newer = [os.path.relpath(p, ROOT)
             for p in [os.path.join(ROOT, "functions_merged.cfg"), CONFIG]
             + [os.path.join(ROOT, "src", f) for f in os.listdir(os.path.join(ROOT, "src"))]
             if os.path.getmtime(p) > elf_mtime]
    if newer:
        print("warning: klonoa-eod.elf is older than %s — run `make` for current results\n"
              % ", ".join(sorted(newer)[:3]), file=sys.stderr)

    by, order = disassemble()
    named = load_symbols()
    inventory = {a: sorted(ns)[0] for a, ns in named.items()
                 if is_function_entry(a, by, order)}
    ref = collect_references(by, order)

    with open(ROM, "rb") as f:
        rom = f.read()

    dead = [(a, n) for a, n in sorted(inventory.items())
            if not ref.get(a)
            and rom.find(struct.pack("<I", a | 1)) < 0
            and rom.find(struct.pack("<I", a)) < 0]

    modules = load_modules()

    def module_of(addr):
        name = "?"
        for start, n in modules:
            if addr >= start:
                name = n
        return name

    boundaries = sorted(named)

    def size_of(addr):
        i = bisect.bisect_right(boundaries, addr)
        return (boundaries[i] if i < len(boundaries) else CODE_END) - addr

    if args.json:
        json.dump([{"addr": "%08X" % a, "name": n, "module": module_of(a),
                    "bytes": size_of(a)} for a, n in dead], sys.stdout, indent=2)
        print()
        return

    total = sum(size_of(a) for a, _ in dead)
    print("named function entries : %d" % len(inventory))
    print("unreachable            : %d\n" % len(dead))
    print("%-10s  %-9s  %-30s %5s" % ("ADDR", "MODULE", "NAME", "BYTES"))
    for a, n in dead:
        print("%08X  %-9s  %-30s %5d" % (a, module_of(a), n, size_of(a)))
    print("\n%d functions, %d bytes (%.2f%% of the %d KB code region)"
          % (len(dead), total, 100.0 * total / (CODE_END - ROM_BASE),
             (CODE_END - ROM_BASE) // 1024))


if __name__ == "__main__":
    main()
