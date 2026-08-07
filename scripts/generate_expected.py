#!/usr/bin/env python3
"""Build objdiff's target object for one module: `expected/src/<module>.o`.

objdiff compares two objects per translation unit -- `build/src/X.o`, which is what your source
compiles to now, against `expected/src/X.o`, the ground truth.  objdiff.json has always declared
the second, but nothing produced it and there was no make rule, so the target side did not exist
and the tool could not be used.

There are no original object files to recover here, only a linked ROM, so the target has to be
synthesized.  NOT from raw ROM bytes: those cannot tell code from data, so literal pools come
back disassembled as instructions and every pool row reads as a mismatch.  The per-function .s
files generate_asm.py already writes carry that split (`.4byte 0x03007FF8`), so they are the
right source, and each module is a contiguous ROM range whose size matches its built object
exactly.

    generate_expected.py <module> [out.o]
"""
import pathlib
import re
import subprocess
import sys

mod = sys.argv[1]
out = sys.argv[2] if len(sys.argv) > 2 else f"expected/src/{mod}.o"

toml = pathlib.Path("klonoa-eod-decomp.toml").read_text()
mods = [(n, int(a, 16)) for n, a in
        re.findall(r'\[\[modules\]\]\s*\nname = "([^"]+)"\s*\nstart = (0x[0-9A-Fa-f]+)', toml)]
start = dict(mods)[mod]
end = next((a for n, a in mods if a > start), None)

# ldscript aliases: luvdis writes `bl sub_080518A4` where the C calls `__divsi3`.  Same bytes,
# and objdiff pairs on the name, so canonicalise or every such call reads as a mismatch.
alias = {v: k for k, v in re.findall(r"^([A-Za-z_]\w*)\s*=\s*(sub_[0-9A-Fa-f]+);",
                                     pathlib.Path("ldscript.in.txt").read_text(), re.M)}
# ...and the TOML's own [renames].  generate_asm.py applies these when it writes the .s files,
# but a CROSS-MODULE callee can still be referred to by its sub_ name, and objdiff pairs on the
# name -- so an unrenamed `bl sub_0804F8E8` reads as a mismatch against the base's
# `bl m4aSoundVSync` even though the bytes are identical.
alias.update(re.findall(r'^(sub_[0-9A-Fa-f]+)\s*=\s*"([^"]+)"',
                        pathlib.Path("klonoa-eod-decomp.toml").read_text(), re.M))

# Addresses come from the built ELF's symbol table, NOT from luvdis's `@ ADDR` comment: that
# comment carries the Thumb bit, so a half-word-aligned function reads two bytes high
# (MultiplyQ4 is at 0x08000990 and is annotated `@ 08000992`).  Placing on the annotated
# address puts the function two bytes late and the next `.org` moves backwards.
nm = subprocess.run(["arm-none-eabi-nm", "-n", "klonoa-eod.elf"],
                    capture_output=True, text=True, check=True).stdout
sym_addr = {}
for line in nm.splitlines():
    parts = line.split()
    if len(parts) == 3 and parts[1] in "Tt":
        sym_addr.setdefault(parts[2], int(parts[0], 16))

files, missing = [], []
for d in (f"asm/matchings/{mod}", f"asm/nonmatchings/{mod}"):
    p = pathlib.Path(d)
    if p.is_dir():
        for f in p.glob("*.s"):
            if f.stem in sym_addr:
                files.append((sym_addr[f.stem], f))
            else:
                missing.append(f.stem)
files.sort()
if missing:
    print(f"  {len(missing)} .s files have no ELF symbol: {missing[:3]}", file=sys.stderr)

lines = ["\t.syntax unified\n", "\t.text\n", "\t.align\t2, 0\n",
         pathlib.Path("asm/macros.inc").read_text()]
prev_addr = None
for addr, f in files:
    name = f.stem
    # Several .s files can share one address -- the repo has alias pairs where one file holds
    # the body and the other is an empty `.global` + label stub (AllocAndClearBuffer_52A4 and
    # DeadCode_0804bb86 both sit at 0x0804bb88).  Position once per address and let the bodies
    # concatenate, or the second `.org` tries to move backwards.
    if addr == prev_addr:
        lines += [l + "\n" for l in f.read_text().split("\n")]
        lines.append(f"\t.size\t{name}, .-{name}\n")
        continue
    prev_addr = addr
    # Place every function at its true offset rather than relying on the macros' alignment.
    # `thumb_func_start` carries `.align 2, 0` and `non_word_aligned_thumb_func_start` does not,
    # so padding between functions gets attributed to whichever side happens to declare it --
    # which shows up as a ±2 alternation and, over a module, a net shortfall.  `.org` sidesteps
    # the question: the ROM's inter-function padding is zero bytes, so fill with 0.
    lines.append(f"\t.org\t0x{addr - start:X}, 0\n")
    body = f.read_text().split("\n")
    # interior `.thumb_func` would make luvdis's mid-function labels into %function symbols,
    # which truncates objdiff's comparison at the first one
    body = [l for i, l in enumerate(body) if not (i and l.strip() == ".thumb_func")]
    body = [re.sub(r"sub_[0-9A-Fa-f]+", lambda m: alias.get(m.group(0), m.group(0)), l)
            for l in body]
    # `.org` has already placed the function exactly, so the start macro must not align again:
    # `thumb_func_start` carries `.align 2, 0`, which on a half-word-aligned function would push
    # it two bytes late and make the NEXT `.org` move backwards.  The non_word_aligned variant is
    # the same macro without the align.
    body = [re.sub(r"(?<!non_word_aligned_)\bthumb_func_start\b",
                   "non_word_aligned_thumb_func_start", l) for l in body]
    # An alignment halfword is data, but luvdis prints it as `lsls r0, r0, #0x00`.  Left as an
    # instruction it becomes an extra CODE row on this side only, which objdiff reports as a
    # spurious `delete`.  Rewrite the ones that end a function or precede a literal pool -- both
    # positions are unreachable, so neither can be live code.
    for i, l in enumerate(body):
        if not re.match(r"^[ \t]*lsls[ \t]+r0,[ \t]*r0,[ \t]*#0x0+[ \t]*$", l):
            continue
        j = next((k for k in range(i + 1, len(body)) if body[k].strip()), None)
        nxt = body[j] if j is not None else ""
        if re.match(r"^_?[0-9A-Fa-f]*:?[ \t]*\.4byte", nxt):
            body[i] = "\t.align\t2, 0"
    lines += [l + "\n" for l in body if l.strip() or True]
    lines.append(f"\t.size\t{name}, .-{name}\n")

pathlib.Path(out).parent.mkdir(parents=True, exist_ok=True)
src = out + ".s"
pathlib.Path(src).write_text("".join(lines))
r = subprocess.run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-mthumb-interwork", src, "-o", out],
                   capture_output=True, text=True)
if r.returncode:
    if "org backwards" in r.stderr:
        sys.exit(f"{mod}: functions overlap, so they cannot be laid out at their ROM addresses. "
                 f"m4a is built from several compilation units .include'd into one file "
                 f"(m4a_1.c, m4a_tst_*.c, m4a_nopush_*.c), so its symbols are not a single "
                 f"contiguous run and this generator does not model it.")
    sys.exit(f"{mod}: assembly failed\n{r.stderr.strip()}")

size = subprocess.run(["arm-none-eabi-size", out], capture_output=True, text=True)
got = int(size.stdout.split("\n")[1].split()[0])
want = end - start if end else None
print(f"{mod}: {len(files)} files -> {got} bytes"
      + (f", module range {want} ({'MATCH' if got == want else 'MISMATCH'})" if want else ""))
