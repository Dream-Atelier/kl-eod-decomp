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
#
# The file spells them in BOTH directions -- `__divsi3 = sub_080518A4;` (8 of them) and
# `sub_0804F8E8 = m4aSoundVSync;` (6 of them, all m4a) -- and either way the luvdis .s uses the
# `sub_` spelling while the C uses the readable one.  So the rule is simply: whichever side is
# the `sub_` name maps to the other.  Matching only one direction left a 1-point floor on the
# 15 functions that call into m4a; found by an agent whose byte-identical VBlankHandler_OamOnly
# would not score below 2.
_ld = pathlib.Path("ldscript.in.txt").read_text()
alias = {b: a for a, b in re.findall(r"^([A-Za-z_]\w*)\s*=\s*(sub_[0-9A-Fa-f]+);", _ld, re.M)}
alias.update(re.findall(r"^(sub_[0-9A-Fa-f]+)\s*=\s*([A-Za-z_]\w*);", _ld, re.M))
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
    # Several .s files can share one address, when one holds the body and the other is an empty
    # `.global` + label stub.  Position once per address and let the bodies concatenate, or the
    # second `.org` tries to move backwards.
    #
    # No such pair exists today: the only one there ever was, AllocAndClearBuffer_52A4 and the
    # empty DeadCode_0804bb86 stub both at 0x0804bb88, is gone now that 0x0804BB86 is no longer
    # declared a function (it was the high halfword of FreeGfxBuffer's literal-pool word).  This
    # stays as a safety net -- an empty stub is the natural shape for any future alias pair --
    # but it is currently unexercised, so do not treat it as tested.
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

# A backwards `.org` is not necessarily fatal, and treating it as fatal was a bug.
#
# `.org` places each function at its ELF address, which is right until a function is
# decompiled whose literal pool luvdis attributed to the NEXT function's file.  agbcc then
# emits that pool inside the C, the ELF address of the next symbol moves forward by the pool's
# size, and that file -- which still carries the pool words at its head -- is asked to start
# after bytes it already contains.  Measured on WaitHBlankAndClearBlendY / AcknowledgeInterrupt:
# the ELF says 0x08001144, the .s covers 0x0800113C onwards, 8 bytes of overlap.
#
# The files still tile the module exactly; only the boundary between two of them is disputed.
# So drop the offending `.org` and let those two concatenate, which is what the ROM does.  The
# MATCH line below is what proves the result is still the right length, and a module that ends
# up genuinely malformed fails there instead of silently shipping.
#
# The luvdis `@ ADDR` comments are NOT an alternative source of truth here: they are +0 on some
# functions (Abs) and +2 on others (ReturnOne) with no decompilation involved.
dropped = []
while r.returncode and "org backwards" in r.stderr:
    bad = [int(m) for m in re.findall(r"\.s:(\d+): Error: attempt to move \.org backwards",
                                     r.stderr)]
    if not bad:
        break
    # Remove the `.org` BEFORE the one that errored, not the one that errored.
    #
    # The disputed function is the one whose .s already contains the bytes: its own `.org`
    # jumps FORWARD over them (legal, and it opens a gap the ROM does not have), and the error
    # only surfaces on the NEXT function, which is then asked to start inside it.  Dropping the
    # erroring `.org` concatenates the wrong pair and leaves the gap, which is how engine came
    # out 8 bytes long.  Dropping the preceding one closes the gap at its source.
    #
    # One per iteration, because `as` reports only the first failure and later ones may resolve
    # themselves once it is gone.
    text = pathlib.Path(src).read_text().split("\n")
    n = min(bad)
    prev = next((i for i in range(n - 2, -1, -1) if text[i].lstrip().startswith(".org")), None)
    if prev is None:
        break
    dropped.append(text.pop(prev).strip())
    pathlib.Path(src).write_text("\n".join(text))
    r = subprocess.run(["arm-none-eabi-as", "-mcpu=arm7tdmi", "-mthumb-interwork", src, "-o", out],
                       capture_output=True, text=True)
if dropped:
    # Say what was done, not why. The cause is usually a decompiled neighbour absorbing this
    # function's literal pool, but the recovery fires on any overlap and does not know which.
    print(f"  {len(dropped)} .org placement(s) dropped, functions concatenated instead: "
          + ", ".join(dropped), file=sys.stderr)

if r.returncode:
    if "org backwards" in r.stderr:
        sys.exit(f"{mod}: functions overlap and dropping the .org did not resolve it. If this "
                 f"is m4a: it is built from several compilation units .include'd into one file "
                 f"(m4a_1.c, m4a_tst_*.c, m4a_nopush_*.c), so its symbols are not a single "
                 f"contiguous run and this generator does not model it. Otherwise the .s files "
                 f"for this module do not tile its address range.")
    sys.exit(f"{mod}: assembly failed\n{r.stderr.strip()}")

# Verify the bytes against the ROM. Length alone proves nothing: `.org` places functions
# absolutely, so the total is fixed by the LAST placement plus its body and any interior gap is
# invisible by construction. m4a passed the length check with 3823 zero bytes inside it, and
# objdiff scored against that object because objdiff.json sets build_target on it -- a loud
# failure turned into a silent wrong answer, which is worse than the failure it replaced.
rom = pathlib.Path("baserom.gba").read_bytes()
subprocess.run(["arm-none-eabi-objcopy", "-O", "binary", "--only-section=.text", out, out + ".bin"],
               check=True)
got_bytes = pathlib.Path(out + ".bin").read_bytes()
want_bytes = rom[start - 0x08000000:][:len(got_bytes)]
# A relocation slot is unresolved in the object and resolved in the ROM, so its bytes cannot be
# compared and have to be masked.  Mask exactly the relocated field, not a flat four bytes: this
# object is the project's measurement instrument, and every over-masked byte is a byte of the
# target that nothing ever checks.
#
# Widths, for the relocation types these objects actually carry (the complete set across all
# nine modules is R_ARM_ABS32, R_ARM_THM_CALL, R_ARM_THM_JUMP11, R_ARM_THM_JUMP8):
#   R_ARM_ABS32      4 -- a .4byte address word
#   R_ARM_THM_CALL   4 -- the Thumb BL *pair* of 16-bit halves
#   R_ARM_THM_JUMP11 2 -- Thumb-1 unconditional `b label`,   one 16-bit instruction
#   R_ARM_THM_JUMP8  2 -- Thumb-1 conditional  `b<cond> label`, one 16-bit instruction
#
# How the two 2-byte ones were confirmed rather than assumed.  By encoding: JUMP11 relocates the
# 11-bit immediate of Thumb-1 `B` and JUMP8 the 8-bit immediate of Thumb-1 `B<cond>`, both of
# which are a single halfword by construction; the 32-bit Thumb branch relocations are the
# separate JUMP19/JUMP24 types, which ARMv4T cannot encode and which appear in none of these
# objects.  Empirically: narrowing to 2 unmasks the 2 bytes after every JUMP11/JUMP8 site (37
# such relocations across the nine modules, so 74 bytes) and every module still reports MATCH --
# those bytes already equalled the ROM and had simply been going unchecked.  That the unmasking
# is not vacuous was shown on code_3 while its .s files were still truncated: the flat 4-byte
# mask reported 636 differing bytes there and this one reports 638.  The same narrowing applied
# to R_ARM_THM_CALL does NOT hold -- its second halfword carries part of the relocated offset
# and differs from the ROM (code_3 then reports 1218 differences) -- which is the control
# showing this check is really reading the bytes it claims to.
#
# Anything not listed falls back to 4, which is the conservative direction: a too-wide mask
# under-reports differences, a too-narrow one would report differences that are not real.
_RELOC_WIDTH = {"R_ARM_THM_JUMP11": 2, "R_ARM_THM_JUMP8": 2}
relocs = subprocess.run(["arm-none-eabi-objdump", "-r", out], capture_output=True, text=True).stdout
masked = {i for m in re.finditer(r"^([0-9a-f]{8})\s+(\S+)", relocs, re.M)
          for i in range(int(m.group(1), 16),
                         int(m.group(1), 16) + _RELOC_WIDTH.get(m.group(2), 4))}
bad = [i for i, (a, b) in enumerate(zip(got_bytes, want_bytes)) if a != b and i not in masked]

want = end - start if end else None
status = "MATCH" if (want is None or len(got_bytes) == want) and not bad else "WRONG"
print(f"{mod}: {len(files)} files -> {len(got_bytes)} bytes"
      + (f", module range {want}" if want else "") + f" ({status})")
if status == "WRONG":
    detail = (f"length {len(got_bytes)} != {want}" if want and len(got_bytes) != want
              else f"{len(bad)} byte(s) differ from the ROM, first at "
                   f"0x{start + bad[0]:08X}" if bad else "")
    sys.exit(f"{mod}: this object does not reproduce the ROM ({detail}). It is a TARGET -- "
             f"anything scored against it would be measured against the wrong bytes.")
