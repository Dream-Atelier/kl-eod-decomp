# Source-Shape Levers in agbcc Decompilation

## Context

agbcc (GCC 2.95 for ARM7TDMI) compiles semantically identical C into different bytes depending
on how the source is *spelled*. Two versions of a function that any reviewer would call the
same code can differ by a register, a prologue, or an entire block layout.

This document catalogs the spellings that decide matches. **Everything here is a plain-C
change** — no barriers, no pinned registers, nothing that would look out of place in original
source.

That restriction is the point, and it is worth stating plainly:

> **An `asm("")` barrier is never load-bearing.** It is a workaround for not having found the
> right C yet. A barrier can always be made to work — that is exactly why it is a trap: it ends
> the search instead of solving it, and the result is not something the original source could
> have contained.

So a function still carrying a barrier is unfinished, not matched-with-a-caveat, and this
catalogue is the list of things to try before reaching for one.
[`agbcc-asm-barriers.md`](agbcc-asm-barriers.md) covers removing the ones already in the tree.

Every entry was measured by ablation: take a byte-exact function, revert exactly one thing,
recompile, record the score. Where the mechanism inside agbcc is not established, it says so
rather than guessing — a wrong mechanism is worse than none, because the next person cites it.

**Verify with `make compare`.** A score of 0 is evidence, not proof; and a byte-exact function
is not sufficient on its own (see the last section).

---

## 1. A named extern is not the same as a cast address constant

The single highest-value lever, and the one most likely to be missed, because the two spellings
are *numerically identical*:

```c
extern const u8 gBgLayerLookup[][2][2];     /* a symbol_ref */
((const u8 (*)[2][2])0x08057ACC)            /* a CONST_INT — same address, different codegen */
```

agbcc treats a relocated symbol and a large integer constant as different beasts. With a
symbol_ref it will keep the address live in a callee-saved register across a loop or a call;
with a CONST_INT it prefers to rematerialise or strength-reduce it.

Measured, each by reverting only the spelling:

| function | named extern | cast constant |
|---|---:|---:|
| `DispatchLevelLayerSetup` | **1** | 29 |
| `LoadBGTileData` | **0** | 26 |
| `FindNextUnlockedVision` | **1** | 20 |
| `StreamCmd_ConfigureSprite` (array symbol vs pointer local) | **12** | 45 |

On `DispatchLevelLayerSetup` the observed effect of the CONST_INT form is that agbcc
strength-reduces the table address into a pointer induction variable (`adds rN, #2` per
iteration) and the function shrinks 100 → 92 bytes, where the original recomputes the whole
address every iteration.

On `StreamCmd_ConfigureSprite` the array symbol must also be indexed *directly* — going through
a `struct ... *` local lets agbcc reassociate `(i + 0xD) * 0x1C` into `i * 0x1C + 0x16C`,
materialise `0x16C` once into a callee-saved register, and burn `r7`, giving a five-register
push where the ROM has four. That one edit is worth 33 points.

**Corollary:** if a table only exists as a `#define ADDR 0x08057ACC`, promoting it to a real
`extern` object in a header plus a line in `ldscript.in.txt` is often the whole match.

## 2. Multi-dimensional shape is load-bearing

Declaring the *dimensions*, not just the element type, changes address arithmetic:

```c
extern const u8 gBgLayerLookup[][2][2];   /* folds +1 into the symbol: adds r0, r4, #1 */
extern const u8 gBgLayerLookup[];         /* emits a separate ldrb r0, [r0, #1] */
```

**A 2-byte struct cannot express this.** ARM's `STRUCTURE_SIZE_BOUNDARY` is 32 bits, so
`struct { u8 entry; u8 layer; }` is padded to **4** — `sizeof` returns 4, and the candidate
emits stride 8 where the ROM has 2. Verified on `DispatchLevelLayerSetup`. Use a flat or
multi-dimensional array of `u8`.

Likewise, a struct of scalars and a run-time-indexed array are not interchangeable. In
`LevelWindowBounds`, eight declared halfwords let agbcc fold the member offset into the store's
immediate (`strh r0, [r1, #0x8]`), while the ROM computes `base + 8` as a *value* and stores at
offset 0 — which is what indexing produces. Respelling the two edge groups as
`s16 leftTop[2][2]` / `s16 rightBottom[2][2]`, indexed on both dimensions, took
`StreamCmd_SetWindowCorner` from 12 to 0 and left `InitLevelStateDefaults` byte-exact.

## 3. A bitfield group's container type is visible in the bytes

For `field ^= K` on a bitfield, agbcc synthesises the insert's negated mask differently
depending on the **container type of the group**:

| container | codegen |
|---|---|
| `u8` | `movs r0, #M` + `negs r0, r0` |
| `u32` | CSEs against the register already holding K: `movs r0, #K` … `subs r0, #n` |

Swept 71 (position × width × K) combinations: under a `u8` container the CSE form fired **0**
times; under `u32`, 60 of 71.

**This cuts both ways inside one struct.** In `GfxControlFlags` the group at byte 2 needs `u32`
(as `u8` it emits `movs r0,#7 / negs`, 92 → 96 bytes, no match) while the group at `0x1C` needs
`u8` — making that one `u32` emits `subs r0, #0x42` and **breaks the already-matched
`StreamCmd_ToggleFadeDirection`**. Choose per group, and re-run `make compare` on the
neighbours after any change.

Related: a byte mask is not a field value. `sceneExit` is 2 bits at bit offset 1, so
`sceneExit ^= 2` toggles the field's *high* bit — byte bit 2 — not the `0x02` flag. The two
coincide only for a field at bit offset 0.

## 4. Do not cache a global in a local

Several stream commands only match when the global is **re-read** at each use rather than held
in a local. `StreamCmd_SetBGPriority` re-reads `gStreamPtr[2]` in all four switch arms; caching
the byte collapses all four to `lsr #4`, cross-jumps them into one block, and drops the function
116 → 104 bytes.

Two refinements worth knowing:

- Only caching the **value** is load-bearing. A *pointer* local (`u8 *p = gStreamPtr; … p[2]`)
  still matches, and is what the neighbouring `StreamCmd_SetWindowCorner` already does.
- It is not all-or-nothing. In `LoadBGTileData`, putting **both** table bytes in locals collapses
  the prologue to `push {lr}` (35 bytes off), while **one** local still leaves
  `push {r4, r5, lr}` and is only 4 bytes off. The prologue comes from register pressure at a
  specific expression, so the number of live values is what matters.

**The mechanism inside agbcc is not established.** An earlier version of this note blamed a
reg-to-reg copy from global CSE evaluated while combine's `nonzero_sign_valid` was 0; that was
wrong — agbcc's RTL shows all four arms reading one pseudo set exactly once from the MEM, and
`-fno-gcse` does not change the asymmetry. The behaviour is reproducible; the cause is open.

## 5. Operand order

Commutative in C, not in the output:

- **Index terms.** `gBgLayerLookup[i * 2 + sceneIdx * 4]` is byte-identical;
  `[sceneIdx * 4 + i * 2]` lets agbcc strength-reduce the address into a pointer induction
  variable and shrinks the function 100 → 92 bytes. Writing the *varying* term first suppresses
  the reduction. Worth 19 points on `DispatchLevelLayerSetup`.
- **Multiplication.** `unk16 * unk18` and `unk18 * unk16` differ by which of `ldrh`/`ldrb` is
  emitted first — a 4-byte, 2-instruction difference, and the entire remaining gap on
  `LoadBGTileData`. Note the identical product is spelled the *other* way round in already-
  matching code at `src/code_3.c`, so copying the idiom from a matching neighbour points the
  wrong way.

## 6. Intermediate width

The declared width of a temporary decides whether a constant is loaded from the pool or
synthesised. In `StreamCmd_DisableVBlankAndStopMusic`, an `s32` intermediate makes agbcc
materialise `0xFFF7` as `mov r3, #9 / neg r3, r3`; a `u16` makes it load the pool word, which
is what the ROM does. Score 18 → 12 (byte-exact) from the type alone.

A regalloc intermediate can also be needed for its own sake: `StreamCmd_FillBGTilemap` requires
a `u16 entry` local holding `0xF002` — inlining the constant at both `DmaFill16` call sites
costs the match.

## 7. Stack frame layout follows size, not declaration order

Probed with 12 variants: agbcc places a `u16` local at `sp+0` and a `u32` at `sp+4`
**regardless of declaration order**, and regardless of which address is taken first. Two locals
of the *same* size do follow declaration order.

An aggregate is not a substitute for two locals. A `struct { u32; u16; }` gives the right
offsets but the wrong addressing — gcc pins `sp` in a register and emits `strh r0, [r3, #4]`,
where the target materialises `add r0, sp, #4` and reuses that value. Two independent objects
are required.

## What does *not* matter

Recorded because each cost someone time, or was written down as a constraint and turned out to
be invented:

- **`~FLAG` vs a literal mask.** `REG_IE &= ~IE_VBLANK` and `REG_IE &= 0xFFFE` are byte-identical.
  Both registers are `vu16`, so the `&=` narrows the constant to 16 bits and the pool word is
  `0x0000FFFE` either way. A docstring once claimed the literal was mandatory; it is not.
- **XOR operand order.** `v ^ 2`, `2 ^ v`, `^= 2` and `^= FLAG` all produce identical output —
  agbcc canonicalises.
- **The project's agent instrumentation flags** (`-finstrument-src-locs` and friends) are
  byte-neutral, as documented in the Makefile. A/B tested.

## A byte-exact function is not sufficient

agbcc's codegen is coupled across a whole translation unit. `TransitionReturnToWorldMap`
compiled byte-identical and the ROM still failed by 4 bytes — in `VBlankDMA_Level21`, **1833
lines further down the same `.c`**. Bisected with function size held constant: referencing the
*extern symbol* at `0x03003510` perturbs it, while reaching the identical cell through an
address-cast macro does not. A second function moved too.

objdiff cannot see this, because the damage is not in the symbol being scored. Only the
full-ROM check catches it, which is why `make compare` rebuilds from scratch and why it should
be run after **every** function added to a branch, not once at the end.

The cause inside agbcc is not identified — only the effect is reproduced.
