# CLAUDE.md

Guidance for Claude Code when working with this repository.

## Quick Reference

```bash
make                # Build from scratch and verify SHA1 match — the only trustworthy check
make rom            # Incremental build, no verification (fast iteration)
make tidy           # Clean build artifacts
make format         # Auto-format C/H files (run before committing)
make compare        # Same as `make`
make ctx            # Generate ctx.c for decomp.me/m2c/mizuchi
make expected       # objdiff's target objects — best effort, one per module
make verify-asm     # Does asm/ still reproduce the cartridge? Hard gate, runs in CI
```

`expected` and `verify-asm` are deliberately separate, and the difference matters when something
is wrong. `expected` exists to hand objdiff a file: it writes every module it can build, and
names any symbol whose bytes disagree with the ROM in `expected/src/<module>.o.tainted` rather
than withholding the whole module — objdiff and `asmlift --score-against` compare **one symbol**,
so a bad byte inside an already-decompiled function says nothing about the others. `verify-asm`
is the module-level question, and it fails on any tainted symbol anywhere.

Fusing the two used to mean one wrong byte in `FreeGfxBuffer` withheld the scoring target from
all 42 undecompiled `gfx` functions, and two functions in `code_3` did the same to 30 more.

Setup: `./setup.sh` after cloning with `--recurse-submodules`. Requires `arm-none-eabi` toolchain, Python 3.13+, legally obtained `baserom.gba`.

A `git worktree` needs no special tooling: run `./setup.sh` in it and it builds. Submodules
initialise normally there, and `baserom.gba` is copied from the main checkout automatically. Expect
about 90s the first time, most of it rebuilding agbcc — the build cache lives inside the submodule,
so each worktree builds its own compiler.

## Project Overview

Matching decompilation of **Klonoa: Empire of Dreams** (GBA, USA). Goal: C source that compiles to a byte-for-byte identical ROM using `agbcc` (GCC 2.95 fork for GBA).

## Policies

### Code Quality
- **Every decompiled function must have a semantic name and docstring.** No `sub_XXXXXXXX` in committed C code. Add rename in `klonoa-eod-decomp.toml` and `/** docstring */` above the function.
- **Run `make format` before every commit** touching C/H files. CI enforces `make check_format`.
- **One commit per matched function.** Descriptive message explaining the matching technique.
- **No `asm("")` barriers in committed C.** A barrier is never load-bearing — it is a workaround for
  not having found the right C, and it can always be made to work, which is exactly the trap: it ends
  the search and leaves behind something the original source could not have contained. A function
  carrying one is unfinished. Try the plain-C levers first:
  [`docs/learnings/agbcc-source-shape-levers.md`](docs/learnings/agbcc-source-shape-levers.md).
- **All policies must be public.** Add to this CLAUDE.md, not just memory.

### Verifying a Match
- **`make compare` is the only verdict**, and it rebuilds from scratch. A score from objdiff or any
  other differ is evidence, not proof — objdiff cannot see the literal pool, and it compares one
  symbol rather than the ROM.
- **A byte-exact function is not sufficient.** agbcc's codegen couples across a whole translation
  unit, so a change that leaves the function you edited byte-identical can still change a *different*
  function in the same `.c` and break the ROM. Measured in `src/code_1.c`: spelling the object at
  `0x03003510` as an extern rather than an address-cast macro leaves `TransitionReturnToWorldMap`
  byte-identical — same 192 bytes at the same offset — while `VBlankDMA_Level21`, **1833 lines
  further down**, goes 1644 → 1640 bytes and the ROM fails. No differ can catch that, because the
  damage is not in the symbol being compared. (This is why `gCallbackQueueAt3510` in that file is a
  macro and not the extern; the reason is recorded next to it.)
- **Run `make compare` after every function you add to a branch**, not once at the end, and again
  after every conflict resolution when merging several functions together. Otherwise you learn which
  of ten functions broke the ROM only after all ten are in.

### Workflow
- **Always use the Python venv.** `source .venv/bin/activate` before `python3`/`pip`.
- **Check GitHub issues before decompiling.** Reference issues in commits/PRs. Post findings on success or failure.
- **Issue closing comments must reference the fix commit** (e.g., "Fixed in abc1234").
- **PRs use feature branches** from `main`. Delete after merge. Never push directly to `main`.
- **PR titles must stay accurate** when updated with new commits.

### Documentation
- **Update the website** (gh-pages) when learning about architecture/subsystems.
- **Always push gh-pages immediately** after every commit so changes go live.
- **"decomp more"** = expand from known functions, name symbols, write docstrings.
- **"more symbols"** = name ALL addressable things (functions, globals, data tables, struct fields, constants).

### Environment
- **Keep the terminal title updated.** `printf '\033]0;DESCRIPTION\007'`
- **Drop caches regularly.** `sudo /usr/local/sbin/drop-caches` during long sessions (virtiofs).

## Decompilation Workflow

1. Pick an `INCLUDE_ASM("asm/nonmatchings/...", sub_XXXXXXXX)` in `src/*.c`
2. Replace with equivalent C that compiles to matching assembly
3. Add rename in `klonoa-eod-decomp.toml`: `sub_XXXXXXXX = "MeaningfulName"`
4. `make compare` → verify `klonoa-eod.gba: OK`
5. `python3 scripts/generate_asm.py` → update assembly labels

## Code Style

- 4-space indent, 120-char line limit, K&R braces
- Right-aligned pointers (`u8 *ptr`)
- Types: `u8/u16/u32`, `s8/s16/s32`, `vu8` etc. from `include/global.h`
- `TRUE`/`FALSE`/`NULL` = 1/0/0
- Include order must never be reordered (affects matching)

## Architecture

Modules in `src/` defined in `klonoa-eod-decomp.toml`. Link order: rom_header → crt0 → system → math → engine → code_0 → code_1 → code_3 → gfx → m4a → syscalls → util → libgcc → data.

### Toolchain
- **agbcc** (tools/agbcc/) — GCC 2.95 for ARM7TDMI Thumb, `-O2`
- **old_agbcc** — older compiler variant, used for m4a module
- **arm-none-eabi-as/ld** — assembler and linker
- **Luvdis** (tools/luvdis/) — disassembler

### Split Compilation Units
Some m4a functions need different compiler flags. They're compiled separately and `.include`'d into m4a.c:
- **m4a_1.c** — compiled with `old_agbcc -ftst` (TST instruction optimization)
- **m4a_tst_*.c** — per-function `-ftst` units (e.g., SoundContextRef)
- **m4a_nopush_*.c** — per-function `-fprologue-bugfix` units (leaf functions without push lr)

## Key Files

- **klonoa-eod-decomp.toml** — module addresses + function renames
- **functions_merged.cfg** — function boundaries for Luvdis
- **ldscript.txt** — linker script (ROM at 0x08000000)
- **mizuchi.yaml** — Mizuchi AI decompilation config
- **mizuchi-db.json** — Mizuchi's index. It is tracked here but produced by
  [Mizuchi](https://github.com/macabeus/mizuchi), not by anything in this repo, and each entry
  embeds a verbatim copy of that function's `.s` alongside its C. So a change to
  `scripts/generate_asm.py` that alters how `asm/` is spelled leaves it stale until Mizuchi
  re-indexes: nothing in the Makefile or CI regenerates or checks it. Say so when you land
  such a change, and give the count.
- **config.mk** — ROM metadata (KLONOA, AKLE, AF)
- **scripts/generate_asm.py** — generates asm/ from baserom.gba
- **scripts/update_stats.py** — auto-updates website stats
