# Detecting Hand-Written Assembly Functions in This Decomp

Not every function in the ROM was originally written in C. A handful were
written by the developers directly in ARM/Thumb assembly. Trying to
decompile those to C produces a function full of `asm("…")` scaffolding
that no compiler would ever emit naturally — and that no C-level rewrite
can eliminate while preserving the SHA1 match.

This document explains how to recognize the fingerprint of hand-written
assembly, lists the three confirmed cases so far, and gives the policy
for handling them.

## The fingerprint

A function is almost certainly hand-written assembly when the
matching-C decomp requires **all four** of the following at once:

1. **Specific register pins on intermediate values** — e.g.
   `register u32 wiConst asm("r2")` for what's logically an unnamed
   temporary. A compiler picks registers via its allocator; a human
   asm author picks them by hand and is consistent across calls.

2. **An explicit MOV between a constant materialization and its use**
   that any optimizer folds. The asm equivalent of
   ```
     mov  r2, #0x1F23
     mov  r0, r2
     strh r0, [r1]
   ```
   when the C-equivalent fold is
   ```
     mov  r0, #0x1F23
     strh r0, [r1]
   ```
   No semantic difference, two extra bytes, zero compiler will emit the
   3-insn form from straight C source.

3. **Inline `asm("add\t%0, #N" : "+r"(ptr))` pointer arithmetic** for
   what `ptr++` or `ptr += N/sizeof(*ptr)` would compute naturally.
   The injection forces the exact opcode encoding the original used;
   it's how a human author writes "increment this pointer by N bytes"
   in asm.

4. **Nested block scoping that defeats register-allocation folding** —
   the C decomp uses `{ ... { ... } }` not because the language requires
   it but because flatter code lets agbcc reshuffle registers across
   the inner scope and lose the original allocation.

If you find one of these alone, it might just be agbcc's
optimizer-vs-target divergence (the documented agbcc gap). If you find
**three or four together** in a tight function, it's hand-written asm.

## Empirical proof in our case

The campaign in commits `9221089` through `82b3f05` (8 commits, ~50
asm directives in scope) achieved a ~88% strip rate by deriving the
empirical rules in `memory/feedback_agbcc_asm_barrier_strip_rules.md`.
Three functions resisted every C-level construct tested:

- Fusion form (`register T x asm("rN") = init`)
- Plain assignment at scaffold position
- Plain assignment at use site
- Init-reorder (which DID work elsewhere)
- Byte-cast for inline-add (which DID work elsewhere)
- `volatile` qualifier on intermediates
- `const` qualifier
- Function-call indirection
- Structural flattening of nested blocks
- Union aliasing
- Ternary identity
- Comma operator
- Multi-pin block fusion
- Helper-variable insertion
- Removal of intermediate variable
- `(void)expr` barrier
- Separate scope blocks
- Wrapping in `if (1) {}`

That stripping pattern — works on most functions, fails on a specific
small set with a consistent fingerprint — is the signal. It's not a
limitation of the rules; it's a property of the source.

## Confirmed cases

Reverted to `INCLUDE_ASM` in commit `204b484`:

### `InitLevelStateDefaults` (`gfx.c` → `asm/nonmatchings/gfx/`)

Sets default level dimensions, scroll, and window registers. Why
hand-written:
- `mov r0, r2; strh r0, [r1]` forced between `wiConst = 0x1F23` and the
  REG_WININ store
- `asm("add\t%0, #0x02")` pointer increment from REG_WININ → REG_WINOUT
  (and again from REG_BG3HOFS → REG_BG3VOFS in the surrounding scope)

### `ReadKeyInput` (`system.c` → `asm/nonmatchings/system/`)

Reads the keypad, computes edge-triggered new presses, tracks A-hold
duration. Why hand-written:
- Four `register asm("rN")` pins on r1-r4 for trivial intermediates
- Two `+r` barriers preventing the compiler from folding
  `pressed = mask; pressed ^= raw` into a single XOR

### `LoadSpriteFrame` (`system.c` → `asm/nonmatchings/system/`)

DMA-copies sprite tile data from ROM to OBJ VRAM. Why hand-written:
- r2 and r5 pinned for ROM table base pointers (DMA-calling-convention
  hand-pattern)
- Nested-block scoping around `spriteTable[idx]` that defeats every
  attempt to flatten

## Policy: when to identify and revert

When you encounter a function during decomp and the matching C body
requires multiple irreducible asm barriers despite full application of
the strip rules:

1. **Don't keep adding workarounds.** Decomp-as-asm-rewritten-in-C is a
   sign you're over-decompiling.
2. **Re-read the asm.** Hand-written code usually has stylistic tells:
   consistent register use across the function, no spills where the
   compiler would spill, "clever" arithmetic via `add #N`.
3. **Leave it as `INCLUDE_ASM`** with a docstring explaining why. The
   `.s` file is the source of truth for these.

The decompilation guideline is: produce C that compiles to matching
bytes via a C compiler's natural output. If natural output is
unreachable, the original wasn't C. Document and move on.

## See also

- `memory/feedback_agbcc_asm_barrier_strip_rules.md` — the 10 empirical
  rules for what asm-barrier patterns ARE strippable
- Commit `204b484` — the three functions reverted
- Commits `9221089` → `82b3f05` — the ~44 successful strips that
  produced the rules and gave us the comparison baseline
