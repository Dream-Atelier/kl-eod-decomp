# Removing `asm("")` Barriers in agbcc Decompilation

## Context

agbcc (GCC 2.95 for ARM7TDMI) has limited optimization capabilities and very specific register allocation behavior. Decompiled C code often uses `asm("")` inline assembly barriers to force the compiler to produce byte-identical output. This document catalogs when these barriers can be removed and when they cannot.

## Three Types of `asm("")` Barriers

### 1. Address Load Barrier: `asm("" : "=r"(ptr) : "0"(addr))`

Forces the compiler to load a constant address into a register at a specific point.

```c
// Before (with barrier):
u32 infoAddr = 0x0300081C;
u32 *info;
asm("" : "=r"(info) : "0"(infoAddr));
info[3] = 0;

// Equivalent to:
u32 *info = (u32 *)0x0300081C;
info[3] = 0;
```

**Why it exists**: Without the barrier, agbcc may:
- Load the address at a different point (earlier or later)
- Fold the address into a different register
- Combine it with other constants

**When removable**: When the address refers to an IWRAM global already defined as a `#define` cast (e.g., `#define gSoundInfo (*(u32 *)0x0300081C)`), and the resulting code happens to match.

### 2. Shift-Folding Barrier: `asm("" : "+r"(shifted))`

Prevents agbcc from optimizing `(x << N) >> M` into `x << (N-M)`.

```c
// Without barrier: agbcc compiles (idx << 24) >> 21 as idx << 3
// With barrier:
u32 shifted = idx << 24;
asm("" : "+r"(shifted));  // compiler must materialize shifted before continuing
shifted >>= 21;           // separate lsrs instruction
```

**Target assembly**:
```arm
lsls r0, r0, #24    ; shifted = idx << 24
ldr r2, =ADDR       ; <-- other instructions interleaved here
ldr r1, =ADDR2      ;
lsrs r0, r0, #21    ; shifted >>= 21
```

**Why it exists**: The original compiler emitted separate `lsls`/`lsrs` with `ldr` instructions between them. Without the barrier, agbcc folds the two shifts into one, changing the instruction count and literal pool layout.

**When removable**: Never, when `ldr` instructions must appear between the two shifts. No C construct can force instruction interleaving.

### 3. Register Pin: `register type var asm("rN")`

Forces a variable into a specific ARM register.

```c
register u32 *ptr asm("r0");
register s32 count asm("r1");
register s32 zero asm("r2");
```

**Why it exists**: The original code used specific registers for loop variables. Without pinning, agbcc freely chooses registers, producing different (but functionally equivalent) machine code.

**When removable**: Never, when the register choice affects the binary encoding. Thumb instructions encode registers in 3-bit fields; different register assignments produce different opcodes even for the same logical operation.

#### Removable sub-case: "address held across a call" pins

Most pins on a *global's address* (`register T *p asm("r4") = &gGlobal;`) exist
only to force that address into a **callee-saved** register *before* a `bl`, so
it survives the call. These are **removable** — a plain named pointer local
declared *before* the call reproduces the same code, because its live range
spans the call and agbcc is then forced to pick a callee-saved register (r4,
r5, …) on its own:

```c
// Before (pin):
register u32 *gfxBuf asm("r4") = &gGfxBufferPtr;
u32 *buf = alloc();
*gfxBuf = (u32)buf;          // r4 held across alloc()

// After (no pin) — identical machine code:
u32 *gfxBuf = &gGfxBufferPtr;  // declared before the call → callee-saved naturally
u32 *buf = alloc();
*gfxBuf = (u32)buf;
```

The same logic frees pins that merely hold `&gStreamPtr` / `&gBuffer_52A4`
across a call while the body re-reads the global: declare the pointer local
before the call and reference it; agbcc keeps it callee-saved. (Verified across
`AllocAndClearGfxBuffer`, `AllocAndClearBuffer_52A4`, `InitGfxStreamState`,
`SetupTextBGLayer`, `StreamCmd_InitHBlankWait` in gfx.c.)

#### Removable sub-case: pins riding on anti-folding intermediates

When a function already uses `u32` intermediate variables to stop agbcc from
constant-folding a macro address (e.g. `cAddr = (u32)gControlBlock; x =
*(u32*)(cAddr+4)` to keep `base + [r0,#4]` instead of a folded `0x...4`
literal), those intermediates *also* fix the register the value lands in — so a
pin layered on top is redundant. Drop the `asm()` qualifier; keep the
intermediates (and comment that they are load-bearing). (Verified:
`UpdateCursorBlink`.)

#### Removable sub-case: a pin faking a "redundant move" is often a *shared load*

When a pin seems to force an otherwise-pointless register-to-register move (the
target does `adds r3, r0, #0` to save a value our agbcc happily leaves in r0),
the move usually exists because the original compiler **reused one memory load
for two purposes**, and that reuse is what evicted the earlier value. Reproduce
the *reuse* in C and the move appears on its own — no pin.

`StreamCmd_SetBGScroll` is the worked example. It re-reads `gStreamPtr` after
each `ReadUnalignedU16` call (the call clobbers the cached pointer). The target's
**second** re-read (in r0) feeds *both* the scrollX store index (`p[2]`) *and*
the scrollY call argument (`p + 5`); holding that pointer in r0 for the call is
what pushes the first call's result out into r3 — exactly what the `scrollX →
r3` pin was faking. The fix is to share one `p`:

```c
u16 scrollX = ReadUnalignedU16(gStreamPtr + 3);
struct BGLayerState *tbl = gBGLayerState;   // declared before p → r5 load hoisted
u8 *p = gStreamPtr;                          // the shared re-read
tbl[p[2]].scrollX = scrollX << 4;            // uses p[2]
scrollY = ReadUnalignedU16(p + 5);           // ...and p + 5 (same p!)
```

Two levers combined: (1) **share the load** — bind the re-read to a named local
and use it for every consumer in that window, instead of re-referencing the
global (which agbcc may load into a fresh register, so nothing gets evicted);
(2) **declaration order hoists literal-pool loads** — declaring `tbl` before `p`
moves its `ldr r5, =gBGLayerState` ahead of the re-read, matching the target's
early base load. Result: zero pins, no orphan block, plain struct indexing.

**Takeaway:** before declaring a register-choice pin irreducible, check whether
it is compensating for a load the original shared across two uses. Diff down to
the single divergent instruction first; "one redundant move" and "one
mis-scheduled `ldr`" are reducible signatures, not dead ends.

#### The biggest lesson: a hand-rolled DMA pointer *creates* the pin — use the macro

`SetupTextBGLayer` looked like a permanently irreducible register pin and was
written up as one. **It wasn't.** The whole problem was self-inflicted by
hand-rolling the DMA register writes through a `volatile u32 *dma` pointer.

The target keeps the DMA3 base in r1 and reuses it for the BGxCNT write via
`(base - 0xCA)` → `subs r1, #0xca`, with r0 as the value scratch. My hand-rolled
`register volatile u32 *dma; ...; dma = (T*)((u8*)dma - 0xCA)` put the base in
r0 instead — an r0↔r1 swap. The in-place self-update earns the pointer an r0
*suggestion* in agbcc `local-alloc.c` (the `qty_sugg_compare` pre-pass runs
before the `QTY_CMP_PRI` priority pass), so no reference-count lever flips it:
~8 manual variants (single/unified scratch, value-first, read-capture, a
13-reference scratch spanning the scroll block) and a Transmuter `no-asm-pin`
refine of **239k iterations** all stayed at the swap (best non-pin score 7).

The fix was not a cleverer C structure — it was the **canonical `DmaCopy16`
macro the original devs actually used**:

```c
DmaCopy16(3, (void *)0x080576B4, (void *)0x050001E0, 0x40);  // ctrl 0x80000020
REG_BG1CNT = 0x700;     // CSE reuses the DMA base: subs r1, #0xca
REG_BG1HOFS = zero;
REG_BG1VOFS = zero;
```

The macro's own scoped `vu32 *dmaRegs` lands in r1 (it never becomes an
in-place-updated pointer, so it gets no r0 suggestion), and CSE carries the
`0x040000d4` base across the macro boundary so the plain `REG_BG1CNT = 0x700`
compiles to the exact `subs r1, #0xca` reuse. **Zero pins, and four readable
lines replace the whole orphan block.** gfx.c now has no register pins at all.

*Takeaway: when a register pin sits on a value you hand-rolled (a DMA pointer, a
raw IO-register pointer), the smell is the hand-rolling, not the register. Reach
for the library macro the original used **before** concluding a pin is
irreducible — the canonical expansion frequently allocates registers the way the
target does precisely because the target came from the same macro. The
"suggestion-pin is a hard stop" conclusion in the git history was wrong; it only
holds for a value you can't express through a macro.*

#### Replacing hand-rolled DMA orphan blocks with the canonical macros

A `register volatile u32 *dma asm("r1"); dma[0]=src; dma[1]=dst; dma[2]=ctrl;
dma[2];` orphan block is just an inlined `DmaFill16`/`DmaCopy16`/`DmaCopy32`.
Decode the control word and replace with the macro (this is how a human, and
pokeemerald/sa3, write it) — the DMA body comes out byte-identical:

| ctrl word | bits | macro |
|---|---|---|
| `0x81000010` | ENABLE\|SRC_FIXED, 16-bit, count 0x10 | `DmaFill16(3, 0, buf, 0x20)` |
| `0x80000020` | ENABLE, 16-bit, count 0x20, SRC_INC | `DmaCopy16(3, src, dst, 0x40)` |
| `0x84000100` | ENABLE\|32-bit, count 0x100 | `DmaCopy32(3, src, dst, 0x400)` |

**Exception**: if the DMA base register is *reused* after a function call (e.g.
`InitGfxStreamState` reuses the DMA3 base across `ClearVideoState()` for a
trailing OAM copy), the macro can't be used — each expansion reloads the base,
but the target loads it once and reuses it. Keep one shared `volatile u32 *dma`
local across the call (no pin needed) and write the registers by hand.

## The Extern Symbol Technique

The most powerful technique for removing address-load barriers: replace `#define` integer constants with `extern` linker symbols.

### Problem

```c
#define ROM_MUSIC_TABLE 0x08118AB4

void m4aSongNumStart(u32 idx) {
    u32 addr = ROM_MUSIC_TABLE;
    u8 *table;
    asm("" : "=r"(table) : "0"(addr));  // forces ldr at this point
    // ...
}
```

agbcc treats `0x08118AB4` as an integer constant. It may inline it, fold it with arithmetic, or schedule the load anywhere. The `asm` barrier forces the load to happen at a specific point.

### Solution

Define the symbol in the linker script and declare it as `extern` in C:

**ldscript.txt**:
```
gMPlayTable = 0x08118AB4;
gSongTable = 0x08118AE4;
```

**globals.h**:
```c
struct MusicPlayer { u32 info; u32 track; u16 numTracks; u8 unk_A; u8 pad; };
struct Song { u32 header; u16 ms; u16 pad; };

extern const struct MusicPlayer gMPlayTable[];
extern const struct Song gSongTable[];
```

**m4a.c**:
```c
void m4aSongNumStart(u32 idx) {
    const struct MusicPlayer *mplayTable = gMPlayTable;  // ldr via literal pool
    const struct Song *songTable = gSongTable;            // ldr via literal pool
    // ...
}
```

### Why It Works

When agbcc sees `extern const struct Song gSongTable[]`, it cannot inline the address — it must emit `ldr rN, =gSongTable` through the literal pool, which the linker resolves to `0x08118AE4`. This naturally creates the same instruction the `asm` barrier was forcing.

The linker symbol approach also enables proper struct access (`song->ms`, `mplay->info`) instead of raw pointer arithmetic, making the code much more readable.

### When It Doesn't Work

The extern approach fails when:
1. **The load must happen at a very specific instruction position** (e.g., `SoundCmd_Dispatch` where the table load must occur after a store, not before). The compiler hoists extern loads earlier than the `asm` barrier would place them.
2. **Register assignment matters**. `extern` loads go into whichever register the compiler chooses. `asm("" : "=r"(x) : "0"(addr))` forces `x` into the same register that held `addr`, giving indirect control over register assignment.
3. **Constant offset folding**. `((u32 *)gExternSym)[1]` compiles to `ldr r1, =(gExternSym+4); str r0, [r1, #0]` — the compiler folds the `+4` into the literal pool address. The target may need `ldr r1, =gExternSym; str r0, [r1, #4]` (base + offset). Neither `extern` nor `#define` can prevent this folding; only `asm("" : "=r"(ptr) : "0"(addr))` loads the raw base address without offset.

## The Orphan Block Pattern

C89 (which agbcc implements) requires variable declarations at the start of a block. Orphan `{ }` blocks exist for two reasons:

### 1. Mid-function variable declarations

```c
void func(void) {
    // ... code ...
    {
        u32 newVar = something;  // C89: must be at block start
        // use newVar
    }
}
```

**Removable when**: The variable can be hoisted to the function top without changing register allocation. Test by moving the declaration and running `make compare`.

### 2. Register lifetime control

```c
void func(void) {
    u32 *a = (u32 *)ADDR_A;
    a[0] = val1;
    a[1] = val2;

    {
        u32 *b = (u32 *)ADDR_B;  // b allocated AFTER a is no longer needed
        b[0] = val3;
    }
}
```

The block scope tells the compiler that `b` doesn't overlap with `a`'s lifetime, allowing register reuse. Without the block, the compiler may allocate `b` to a different register.

**Removable when**: The compiler naturally makes the same register choice with declarations hoisted. In practice, agbcc often makes different choices, so these blocks tend to be load-bearing.

### 3. Volatile DMA scope

```c
{
    vu32 *dma = (vu32 *)0x040000D4;
    dma[0] = src;
    dma[1] = dest;
    dma[2] = control;
    (void)dma[2];  // volatile read: wait for DMA
}
```

Volatile DMA scope blocks are **sometimes removable**. It depends on what happens before the block:

- **Removable** when the volatile pointer is only used after all function calls are complete (e.g., `m4aSoundVSyncOff` — DMA access happens after all setup). Declare the `vu32 *` at function top and reassign as needed.
- **Not removable** when the volatile pointer sits between function calls (e.g., `DecompressAndDmaCopy` — DMA happens between `DecompressData()` and `thunk_FUN_0800020c()`). Hoisting `vu32 *` to function top changes register allocation across the calls.

The key is whether the volatile pointer's lifetime overlaps with a function call that clobbers registers.

## The Volatile Store Technique

For address ordering barriers where code must store to address A before loading address B:

```c
// Before (with asm barrier):
u32 pauseFlagAddr = 0x030034E4;
u32 controlBlockAddr = 0x03004C20;
u8 *pauseFlag;
u32 *sceneCtrl;
asm("" : "=r"(pauseFlag) : "0"(pauseFlagAddr));
*pauseFlag = 1;
asm("" : "=r"(sceneCtrl) : "0"(controlBlockAddr));

// After (with volatile store):
*(vu8 *)0x030034E4 = 1;
sceneCtrl = (u32 *)0x03004C20;
```

**Why it works**: A volatile write is a side effect that the compiler cannot reorder past. agbcc must complete the `strb` before evaluating subsequent expressions. This prevents hoisting the `sceneCtrl` load before the store.

**When it works**: When the barrier's only purpose is to order a store before a load. The volatile qualifier on the store pointer is sufficient.

**Also works as volatile read**: `asm("" : "+r"(var))` barriers that prevent reordering a load past a subsequent address computation can be replaced by `*(volatile type *)ptr` reads:

```c
// Before:
fadeTimer = sceneCtrl[0];
asm("" : "+r"(fadeTimer));   // prevent reordering past next ldr
fadeCounter = (u8 *)0x03005498;

// After:
fadeTimer = *(vu32 *)sceneCtrl;  // volatile read = compiler barrier
fadeCounter = (u8 *)0x03005498;
```

**When it doesn't work**: When the barrier also controls which register the address goes into (the `"=r" : "0"` constraint ties output to input register).

## The Parameter Type Technique

For shift-folding barriers caused by `u32` parameter + `<<N >>M`:

```c
// Before (u32 param, needs shift barrier):
void m4aSongNumStart(u32 idx) {
    u32 shifted = idx << 16;
    asm("" : "+r"(shifted));           // prevent folding
    song = ... + (shifted >> 13);      // separate lsrs
}

// After (u16 param, no barrier needed):
void m4aSongNumStart(u16 idx) {
    song = &songTable[idx];            // compiler generates lsls+lsrs naturally
}
```

**Why it works**: When agbcc receives a `u16` parameter, it generates `lsls r0, r0, #16` (zero-extend) at function entry. The subsequent array indexing generates a separate shift. The compiler naturally places `ldr` instructions between these shifts because the loads are needed for the array base.

With `u32`, the compiler can fold `(x << 16) >> 13` into `x << 3` because no truncation is needed. With `u16`, the `<<16` is part of the ABI zero-extension and cannot be folded.

**Key insight**: pokeemerald and sa3 define `m4aSongNumStart(u16 n)` — the `u16` type is what the original source used. The `u32` parameter in Klonoa's initial decompilation was a type inference error that required shift barriers to compensate.

## Reference: How Other Decomp Projects Handle This

### pokeemerald (Pokemon Emerald)
- **Zero** `asm("")` barriers in all of `src/`
- Uses `extern` symbols extensively (`gMPlayTable`, `gSongTable`, etc.)
- Proper structs for all data tables
- Uses `u16` parameter types for song functions, eliminating shift patterns
- DMA access via macros (`DmaSet`, `DmaFill`) that use orphan blocks with volatile locals — this is considered acceptable

### sa3 (Sonic Advance 3)
- Only **10** `asm("")` barriers across entire codebase
- Wraps them in `#define MATCH_BREAK asm("")` behind `#ifndef NON_MATCHING`
- Uses `extern` symbols and structs like pokeemerald
- Uses `register ... asm("rN")` in a few places, also behind `NON_MATCHING` guards
- Their m4a.c has **zero** `asm` barriers — identical struct-based approach to pokeemerald

## Decision Tree

When you encounter an `asm("")` barrier:

1. **Is the address a ROM data table constant (0x08xxxxxx)?**
   - Try the extern symbol approach: add to ldscript.txt + extern declaration
   - Define a struct if the table has a fixed stride (8, 12, etc.)

2. **Is it an `asm("" : "+r"(var))` between two shifts?**
   - This prevents shift folding. Cannot be removed.
   - Check if extern symbols for surrounding loads eliminate the *need* for interleaved loads

3. **Is it `register ... asm("rN")`?**
   - This pins a specific register. Cannot be removed if the register choice affects encoding.
   - Verify by removing and checking objdiff — if the same registers are chosen naturally, it can go.

4. **Is it an IWRAM address (0x03xxxxxx) load barrier?**
   - Check if a `#define` global already exists in globals.h
   - Try direct assignment: `u32 *ptr = (u32 *)0x03004C20;`
   - If that fails, the barrier controls load ordering — keep it.

5. **Is it inside an orphan `{ }` with `vu32 *`?**
   - Volatile DMA scopes are always load-bearing. Keep the block.
