#include "global.h"
#include "gba.h"
#include "globals.h"

/* ══════════════════════════════════════════════════════════════════════
 * m4a_3 — TCC -O1 compilation unit of the MusicPlayer2000 sound engine
 *
 * These functions were compiled with ARM's Norcroft Thumb C Compiler
 * (tcc) at -O1 in the original SDK. Unlike the -O0 functions in m4a_2.c,
 * tcc -O1 reuses dead parameter registers for temporaries instead of
 * preserving them, resulting in tighter code without push/pop frames.
 *
 * This file is compiled with tcc -O1 -apcs /interwork to match the
 * original code generation. Only decompiled C functions go here;
 * INCLUDE_ASM functions stay in m4a.c since they produce raw assembly
 * unaffected by compiler choice.
 *
 * See docs/m4a-investigation.md for the full investigation.
 * ══════════════════════════════════════════════════════════════════════ */

/**
 * VoiceGetParams (SDK: ClearChain): unlink a voice node from a doubly-linked list.
 *
 * Removes the given node from its chain by updating the previous node's
 * next pointer and the next node's previous pointer. If the node has no
 * previous node (i.e., it's the head), updates the chain's head pointer
 * at offset 0x20. Finally clears the node's chain pointer (offset 0x2C).
 *
 * @param node  Pointer to the voice/channel node to unlink
 */
void VoiceGetParams(u32 *node) {
    u32 *chain = (u32 *)node[0x2C / 4];
    if (chain != 0) {
        u32 *next = (u32 *)node[0x34 / 4];
        u32 *prev = (u32 *)node[0x30 / 4];
        if (prev != 0) {
            prev[0x34 / 4] = (u32)next;
        } else {
            chain[0x20 / 4] = (u32)next;
        }
        if (next != 0) {
            next[0x30 / 4] = (u32)prev;
        }
        node[0x2C / 4] = 0;
    }
}
