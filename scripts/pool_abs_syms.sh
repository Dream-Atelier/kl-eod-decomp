#!/bin/sh
# pool_abs_syms.sh <candidate.o> <candidate.s> [elf]
#
# Appends `.set NAME, 0xADDR` lines to a candidate's assembly for the DATA globals it
# references, so re-assembling resolves them to absolute values instead of leaving
# relocations behind.
#
# WHY. Scoring targets in this project are disassembled from the LINKED ROM, so a data
# address in a target's literal pool is a bare number (`.4byte 0x03005428`). A candidate that
# spells the same cell as a named extern assembles to `.word gUnk_03005428` plus an
# R_ARM_ABS32 relocation, and objdiff scores that row as a mismatch against the target's
# number — even though the linker would emit identical bytes.
#
# The penalty therefore lands ONLY on candidates that use names, which is backwards: naming a
# global is the single most productive matching lever in this repo. Two measured consequences
# before this existed:
#
#   * asmlift ranked its own raw-address candidate above its named one and printed the worse
#     of the two (LoadBGTileData: the two tie once this runs);
#   * Transmuter could not report a perfect match at all — a byte-exact source scored 2 with
#     `perfectMatch` false, both residual rows being named pool words.
#
# Effect on already-matched functions, scored against their own luvdis targets:
# RenderDialogSprites 42 -> 0, AnimatePaletteEffects 24 -> 2, PlayerRespawnOrDeath 6 -> 0,
# InitOamEntries 3 -> 0, CopyBGScrollTiles 2 -> 0.
#
# LIMIT. Symbols the ELF types as FUNC are left alone, because a `bl` is a relocation on both
# sides and must stay that way. luvdis sometimes emits a function POINTER parked in a literal
# pool as a bare number rather than a named `.4byte`, and those rows still mismatch — the
# residual 2 on AnimatePaletteEffects is exactly that. Removing it needs the target object,
# which a decomp.yaml compiler template is not given; a caller that has one can filter more
# precisely.
set -eu

OUT="$1"
ASM="$2"
ELF="${3:-klonoa-eod.elf}"
TMP="${TMPDIR:-/tmp}/pool_abs_syms.$$"

[ -r "$ELF" ] || exit 0
trap 'rm -f "$TMP"' EXIT

arm-none-eabi-nm -u "$OUT" | awk '{print $NF}' | sort -u >"$TMP"
[ -s "$TMP" ] || exit 0

arm-none-eabi-readelf -sW "$ELF" |
    awk 'NR > 3 && $4 != "FUNC" && $8 != "" {print $8, $2}' |
    sort -u -k1,1 |
    join -j 1 - "$TMP" |
    awk '{printf "\t.set\t%s, 0x%s\n", $1, $2}' >>"$ASM"
