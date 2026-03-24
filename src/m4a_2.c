#include "global.h"
#include "gba.h"
#include "globals.h"

/* ══════════════════════════════════════════════════════════════════════
 * m4a_2 — TCC compilation unit of the MusicPlayer2000 sound engine
 *
 * These functions were compiled with ARM's Norcroft Thumb C Compiler
 * (tcc) at -O0 in the original SDK. The m4a library (mks4agbLib.o) was
 * split between GCC-compiled functions and tcc-compiled functions.
 *
 * This file is compiled with tcc -O0 -apcs /interwork to match the
 * original code generation. Only decompiled C functions go here;
 * INCLUDE_ASM functions stay in m4a.c since they produce raw assembly
 * unaffected by compiler choice.
 *
 * Requires: ARM SDT 2.51 tcc (tools/tcc/bin/tcc_linux or Wine for tcc.exe)
 * On macOS: brew install wine-stable
 * On Linux x86_64: sudo apt install libc6:i386
 *
 * See docs/m4a-investigation.md for the full investigation.
 * ══════════════════════════════════════════════════════════════════════ */

/**
 * SoundEffectParamInit (SDK: clear_modM): clear modulation state and set flags.
 *
 * Clears the modulation-related fields (offsets 0x16 and 0x1A), then sets
 * the channel status bits based on whether the channel mode (offset 0x18)
 * is zero (inactive -> 0x0C) or non-zero (active -> 0x03).
 *
 * @param unused   Unused first parameter (register r0 not referenced)
 * @param track    Pointer to track/channel structure
 */
void SoundEffectParamInit(u32 unused, u8 *track) {
    u8 orVal;
    track[0x16] = 0;
    track[0x1A] = 0;
    if (track[0x18] == 0) {
        orVal = 0x0C;
    } else {
        orVal = 0x03;
    }
    track[0x00] |= orVal;
}
