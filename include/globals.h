#ifndef GUARD_GLOBALS_H
#define GUARD_GLOBALS_H

#include "global.h"

/* ── Graphics Stream ── */

/* Pointer to the current position in the graphics/music data stream.
 * Double indirection: the u32 at this address holds a u8* into the stream.
 * Nearly all gfx stream command functions read/advance this pointer. */
#define gStreamPtr         (*(u8 **)0x03004D84)

/* Pointer to the graphics buffer control struct.
 * Dereferenced for palette state, flags, and buffer management. */
#define gGfxBufferPtr      (*(u32 *)0x030034A0)

/* Secondary stream output: palette/color value mirror. */
#define gStreamColorOut    (*(u16 *)0x03005420)

/* Stream color mirror (written alongside gStreamColorOut). */
#define gStreamColorMirror (*(u16 *)0x030034AC)

/* Decompressed data buffer pointer (allocated by LoadAndDecompress functions). */
#define gDecompBuffer      (*(u32 *)0x030007D0)

/* Graphics buffer freed by ShutdownGfxStream. */
#define gGfxStreamBuffer   (*(u32 *)0x030007C8)

/* Buffer freed by FreeBuffer_52A4. */
#define gBuffer_52A4       (*(u32 *)0x030052A4)

/* ── Sound / Music ── */

/* Sound context pointers used by PlaySoundWithContext functions. */
#define gSoundCtx_D8       (*(u32 *)0x030064D8)
#define gSoundCtx_DC       (*(u32 *)0x030064DC)

/* Sound command dispatch table pointer. */
#define gSoundTablePtr     (*(u32 *)0x03006450)

/* Sound struct pointer (for FreeSoundStruct). */
#define gSoundStructPtr    (*(u32 *)0x0300081C)

/* ── Game State ── */

/* Pause flag: when non-zero, GameUpdate skips the main update loop. */
#define gPauseFlag         (*(u8 *)0x030034E4)

/* Frame/tick counter — decremented each frame, byte-sized. */
#define gFrameCounter      (*(u8 *)0x03005498)

/* ── Entity / Object System ── */

/* Base of the main entity/sprite struct array (~36 bytes per element).
 * Most-referenced address in the entire codebase (1634 refs). */
#define gEntityArray       ((u8 *)0x03002920)

/* Global control/state block — flags and status at word/byte offsets. */
#define gControlBlock      ((u8 *)0x03004C20)

/* Game state struct array (byte-field access, ~200+ bytes per entry). */
#define gGameStateArray    ((u8 *)0x03005220)

/* Game state flags struct (byte fields at various offsets). */
#define gGameFlags         ((u8 *)0x03005400)

/* Configuration/output buffer struct (16-bit fields, dimensions/coords). */
#define gConfigBuffer      ((u8 *)0x03003430)

/* Pointer/handle array for buffers. */
#define gHandleArray       ((u8 *)0x03004790)

/* Entity/object pointer (double indirection). */
#define gEntityPtr         (*(u32 *)0x03004654)

/* OAM source data / lookup table (halfword-indexed). */
#define gOamSourceTable    ((u16 *)0x03004680)

/* Status byte lookup table. */
#define gStatusTable       ((u8 *)0x03000830)

/* Mixed-mode struct (byte + halfword fields). */
#define gMixedState        ((u8 *)0x03003510)

/* DMA buffer source address. */
#define gDmaBufferAddr     (*(u32 *)0x030007DC)

#endif /* GUARD_GLOBALS_H */
