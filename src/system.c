#include "global.h"
#include "globals.h"
#include "structs/variables.h"
#include "include_asm.h"

/** Abs: returns absolute value of a signed integer. */
INCLUDE_ASM("asm/nonmatchings/system", Abs);

/** StrCmp: byte-by-byte string comparison. Returns 0 if equal, 1 if different. */
INCLUDE_ASM("asm/nonmatchings/system", StrCmp);
/** ReturnOne: unconditionally returns 1. */
s32 ReturnOne(void) {
    return 1;
}
/** StrCpy: copies a null-terminated string from src to dst. */
void StrCpy(u8 *dst, u8 *src) {
    u32 c;
    do {
        c = *src;
        *dst = c;
        c = *src;
        dst++;
        src++;
    } while (c != 0);
}

/**
 * AgbMain: game entry point.
 *
 * Clears IWRAM/VRAM/OAM/palette via DMA, initializes display, sound,
 * and input systems, then enters the main game loop dispatching
 * through a state machine.
 */
INCLUDE_ASM("asm/nonmatchings/system", AgbMain);

/**
 * ReadKeyInput: read GBA buttons, debounce, and check for reset combo.
 *
 * Reads REG_KEYINPUT (active-low), XORs with 0x3FF to get active-high
 * pressed state, edge-detects new presses against previous frame,
 * checks A+B+Start+Select combo (0x0F) for soft reset, and tracks
 * A-button hold duration for repeat input.
 */
void ReadKeyInput(void) {
    u16 pressed = REG_KEYINPUT ^ 0x3FF;

    gKeysPressed = pressed & ~gKeysPrevious;
    gKeysPrevious = pressed;
    if ((pressed & 0x0F) == 0x0F) {
        SoftResetRom(0xFF);
    }
    if (gKeysPrevious & 1) {
        gAButtonHold++;
    } else {
        gAButtonHold = 0;
    }
}

/**
 * ProcessInputAndTimers: extended input handler with timer management.
 *
 * Reads keys, processes directional input with acceleration, decrements
 * scene control timer, dispatches through state table at 0x080D9150.
 */
INCLUDE_ASM("asm/nonmatchings/system", ProcessInputAndTimers);

/**
 * LoadSpriteFrame: DMAs sprite frame data from ROM to OBJ VRAM.
 *
 * Looks up the source tile data via ROM_SPRITE_SUBTABLE[tilesetIdx],
 * computes the destination in OBJ VRAM from the frame index using
 * gUnk_0818B8E0[world-1][level]->unk4[], then DMAs the sprite tile data.
 */
void LoadSpriteFrame(u8 frame, u8 tilesetIdx) {
    vu32 *dma = (vu32 *)REG_ADDR_DMA3SAD;

    dma[0] = gUnk_0818B8A8[tilesetIdx];
    dma[1] = OBJ_VRAM + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[frame - 0xD].tileNum << 5);
    dma[2] = 0x80000010;
    dma[2];
}

/**
 * FreeAllDecompBuffers: frees all 6 decomp buffers + collision map.
 *
 * Frees gDecompBufferCtrl entries [0]-[5] (offset -4 for sub-header),
 * then conditionally frees gCollisionMapPtr.
 */
void thunk_HeapFree(u32);
void m4aSongNumStart(u16);
void FreeAllDecompBuffers(void) {
    u32 *decompBuffers = (u32 *)gDecompBufferCtrl;

    thunk_HeapFree(decompBuffers[1] - 4);
    thunk_HeapFree(decompBuffers[0] - 4);
    thunk_HeapFree(decompBuffers[3] - 4);
    thunk_HeapFree(decompBuffers[2] - 4);
    thunk_HeapFree(decompBuffers[5] - 4);
    thunk_HeapFree(decompBuffers[4] - 4);

    if (gCollisionMapPtr != 0) {
        thunk_HeapFree(gCollisionMapPtr - 4);
        gCollisionMapPtr = 0;
    }

    m4aSongNumStart(0x8D);
    m4aSongNumStart(0x8E);
    m4aSongNumStart(0x8F);
    m4aSongNumStart(0x90);
}

/**
 * MultiplyQ8: 8.8 fixed-point signed multiply (s16*s16 >> 8).
 * Rounds negative results toward zero by adding 255 before shift.
 */
s16 MultiplyQ8(s16 num1, s16 num2) {
    s32 product;
    s32 rounded;

    product = num1 * num2;
    rounded = product;
    if (rounded < 0) {
        rounded += 0xFF;
    }
    product = rounded >> 8;
    return product;
}
