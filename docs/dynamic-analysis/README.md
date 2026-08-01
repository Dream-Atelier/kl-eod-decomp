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
| `GBA_KIT` | `../gba-kit` | built gba-kit checkout |
| `GBA_KIT_SAVESTATE` | `$GBA_KIT/klonoa-analysis/savestate-in-level-idle.json` | savestate to resume from |
| `KLONOA_ROOT` | this repo | where to find `baserom.gba` / `klonoa-eod.elf` |

Scripts that resume from a savestate need one captured at the right point; the
default is an in-level idle frame.

## Caveats

These scripts read struct member names out of DWARF, so **renaming a field in
`include/` breaks the script that cites it** — there is no build gate for that.
If you rename something, re-run the scripts that mention it.

A script proves what it measures and no more. Where a name generalises beyond
the observation (one install site, one entry type), the header comment says so;
prefer weakening a name to overstating it. At least one earlier round of this
work produced a confidently wrong rename because the *script* had two hardware
register addresses transposed — the observations were right and the labels were
not. Check the register map in `include/io_reg.h` before trusting a label.
