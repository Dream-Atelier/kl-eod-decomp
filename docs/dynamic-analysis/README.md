# Dynamic analysis

Runtime experiments that establish what a field, global or function actually
does, used as the evidence of record for the names and comments in `include/`.

Each `scripts/prove-*.mjs` is a self-contained experiment: it boots the ROM in
the [gba-kit](https://github.com/macabeus/gba-kit) headless emulator, resolves
every address and struct offset from the build's own DWARF (nothing is
hand-typed), then runs an A/B intervention — change one thing between two
otherwise identical runs and observe what moves. A header comment that cites one
of these scripts is claiming exactly what that script prints.

## Running them

They need a **built** gba-kit checkout and a `klonoa-eod.elf` from `make`:

```sh
make                                    # produces klonoa-eod.elf with DWARF
GBA_KIT=../gba-kit node docs/dynamic-analysis/scripts/prove-button-wait.mjs
```

| variable | default | meaning |
|---|---|---|
| `GBA_KIT` | `../gba-kit`, resolved against **this repo**, not the current directory | built gba-kit checkout — set it explicitly unless gba-kit really is this repo's sibling |
| `GBA_KIT_SAVESTATE` | `$GBA_KIT/klonoa-analysis/savestate-in-level-idle.json` | savestate to resume from |
| `KLONOA_ROOT` | this repo | where to find `baserom.gba` / `klonoa-eod.elf` |

Scripts that resume from a savestate need one captured at the right point; the
default is an in-level idle frame.

Two files in `scripts/` are not experiments: `gba-kit.mjs` locates the emulator,
and `_harness.mjs` exports `ROM` / `ELF` / `SAVES` plus `readField` / `writeField`,
which decode a DWARF `MemberLocation` (bitfields included) through the emulator bus.
Every script must go through those helpers rather than hand-coding an offset or a
mask.

## Caveats

These scripts read struct member names out of DWARF, so **renaming a field in
`include/` breaks the script that cites it** — there is no build gate for that.
If you rename something, re-run the scripts that mention it. Likewise, a script
that fails to *start* (a missing import, a helper that was never committed, a
sequence file that lives outside the repo) proves nothing at all — a header
comment citing it is unsupported until the script runs end to end. Run all of
them, not just the one you touched.

A script proves what it measures and no more. Where a name generalises beyond
the observation (one install site, one entry type), the header comment says so;
prefer weakening a name to overstating it. At least one earlier round of this
work produced a confidently wrong rename because the *script* had two hardware
register addresses transposed — the observations were right and the labels were
not. Check the register map in `include/io_reg.h` before trusting a label.
