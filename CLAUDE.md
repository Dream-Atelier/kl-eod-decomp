# CLAUDE.md

Guidance for Claude Code when working with this repository.

## Quick Reference

```bash
make                # Build ROM and verify SHA1 match
make tidy           # Clean build artifacts
make format         # Auto-format C/H files (run before committing)
make compare        # Same as `make` — build + SHA1 verify
make ctx            # Generate ctx.c for decomp.me/m2c/mizuchi
```

Setup: `./setup.sh` after cloning with `--recurse-submodules`. Requires `arm-none-eabi` toolchain, Python 3.13+, legally obtained `baserom.gba`.

## Project Overview

Matching decompilation of **Klonoa: Empire of Dreams** (GBA, USA). Goal: C source that compiles to a byte-for-byte identical ROM using `agbcc` (GCC 2.95 fork for GBA).

## Policies

### Code Quality
- **Every decompiled function must have a semantic name and docstring.** No `FUN_XXXXXXXX` in committed C code. Add rename in `klonoa-eod-decomp.toml` and `/** docstring */` above the function.
- **Run `make format` before every commit** touching C/H files. CI enforces `make check_format`.
- **One commit per matched function.** Descriptive message explaining the matching technique.
- **All policies must be public.** Add to this CLAUDE.md, not just memory.

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

1. Pick an `INCLUDE_ASM("asm/nonmatchings/...", FUN_XXXXXXXX)` in `src/*.c`
2. Replace with equivalent C that compiles to matching assembly
3. Add rename in `klonoa-eod-decomp.toml`: `FUN_XXXXXXXX = "MeaningfulName"`
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
- **config.mk** — ROM metadata (KLONOA, AKLE, AF)
- **scripts/generate_asm.py** — generates asm/ from baserom.gba
- **scripts/update_stats.py** — auto-updates website stats
