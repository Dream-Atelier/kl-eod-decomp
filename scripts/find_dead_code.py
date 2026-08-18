#!/usr/bin/env python3
"""
Find unreachable functions in the ROM.

Builds a whole-cartridge reference graph and reports every named function that
nothing can ever reach.  A function is reported only when none of the following
finds a way in:

  * no branch of any form (bl / blx / b / bCC) anywhere in the code region --
    read both from the disassembly AND decoded straight from cartridge bytes,
    because generate_asm.py emits some calls as raw `.4byte` data that no
    disassembly-based scan can see,
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

Reports two lists. "mid-function splits" are cfg entries the previous
instruction falls into, so nothing can call them; "unreachable" are real
function entries no reference reaches.

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


def fallen_into(addr, by, order):
    """Is this address code that the instruction above it runs straight into?

    Such an address cannot be a function: control reaches it without a call, so
    whatever named it cut a real function in two and gave the tail a name. The
    reachability pass below drops these from its inventory; report_splits() also
    prints them, because a silent filter is how one survives for a whole release.
    """
    head = by.get(addr)
    if head is None or head[0] in DATA_DIRECTIVES:
        return False                      # the address is data, not a split
    i = bisect.bisect_left(order, addr) - 1
    if i < 0:
        return False
    pm, po, _ = by[order[i]]
    return not terminates(pm, po)


def is_function_entry(addr, by, order):
    head = by.get(addr)
    if head is None or head[0] in DATA_DIRECTIVES:
        return False                      # the address is data, not code

    if fallen_into(addr, by, order):
        return False                      # a mid-function split

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


def raw_rom_calls(rom, ref):
    """Decode every Thumb BL/BLX pair straight out of the cartridge.

    This cannot be skipped in favour of reading objdump's output. Wherever a
    branch target is not a function start in functions_merged.cfg,
    generate_asm.py emits the call as a raw `.4byte 0x........ @ bl _0XXXXXXX`,
    so the assembler stores data and objdump renders data — the call is
    invisible to any scan that trusts the disassembly. There are 73 such sites,
    and one of them (0x08047A20 -> 0x0804713C, the save-erase routine reached
    from the hidden boot menu) is the only caller its callee has: reading the
    disassembly alone reports that function as unreachable when it is not.

    Scanning is limited to the code region; the data region would produce
    false BL pairs out of graphics bytes.
    """
    for addr in range(ROM_BASE, CODE_END - 4, 2):
        off = addr - ROM_BASE
        hi, lo = struct.unpack_from("<HH", rom, off)
        if (hi >> 11) != 0x1E:
            continue
        kind = lo >> 11
        if kind not in (0x1F, 0x1D):        # BL, BLX
            continue
        disp = ((hi & 0x7FF) << 12) | ((lo & 0x7FF) << 1)
        if disp & 0x400000:
            disp -= 0x800000
        target = (addr + 4 + disp) & 0xFFFFFFFF
        if kind == 0x1D:
            target &= ~3
        if target != addr:
            ref[target] += 1


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
    raw_rom_calls(rom, ref)

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
        json.dump({"unreachable": [{"addr": "%08X" % a, "name": n, "module": module_of(a),
                                    "bytes": size_of(a)} for a, n in dead],
                   "splits": [{"addr": "%08X" % a, "name": sorted(ns)[0],
                               "module": module_of(a)}
                              for a, ns in sorted(named.items())
                              if fallen_into(a, by, order)]}, sys.stdout, indent=2)
        print()
        return

    splits = [(a, sorted(ns)[0]) for a, ns in sorted(named.items())
              if fallen_into(a, by, order)]

    total = sum(size_of(a) for a, _ in dead)
    print("named function entries : %d" % len(inventory))
    print("mid-function splits    : %d" % len(splits))
    print("unreachable            : %d\n" % len(dead))
    if splits:
        print("%-10s  %-9s  %-30s %s" % ("ADDR", "MODULE", "NAME", "INSTRUCTION ABOVE"))
        for a, n in splits:
            pm, po, _ = by[order[bisect.bisect_left(order, a) - 1]]
            print("%08X  %-9s  %-30s %s %s" % (a, module_of(a), n, pm, po))
        print("\n^ these are not functions: drop them from functions_merged.cfg.\n")
    print("%-10s  %-9s  %-30s %5s" % ("ADDR", "MODULE", "NAME", "BYTES"))
    for a, n in dead:
        print("%08X  %-9s  %-30s %5d" % (a, module_of(a), n, size_of(a)))
    print("\n%d functions, %d bytes (%.2f%% of the %d KB code region)"
          % (len(dead), total, 100.0 * total / (CODE_END - ROM_BASE),
             (CODE_END - ROM_BASE) // 1024))


if __name__ == "__main__":
    main()
