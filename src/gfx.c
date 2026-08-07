#include "global.h"
#include "gba.h"
#include "globals.h"
#include "structs/variables.h"
#include "include_asm.h"
#include "data/trig.h"

INCLUDE_ASM("asm/nonmatchings/gfx", InitGfxState);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateBGScrollRegisters);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateBGTileAnimation);
void UpdateBGScrollRegisters(void);
void ProcessFrameAnimation(void);
u32 ProcessMotionStep(u32 idx);
void ProcessStaticBGScroll(void);
extern s16 ReciprocalQ8(s16 a);
extern s16 MultiplyQ8(s16 a, s16 b);
void m4aSoundVSyncOff(void);
void m4aMPlayAllStop(void);
void UpdateSceneTransition(void);
/**
 * FadeOutController: manages screen fade-out, updating fade counter
 * and switching to scene transition when complete.
 */
void FadeOutController(void) {
    u32 *sceneCtrl = (u32 *)gControlBlock;
    u32 fadeTimer;
    u8 *fadeCounter;

    if (sceneCtrl[0] == 0)
        UpdateBGScrollRegisters();

    fadeTimer = *(vu32 *)sceneCtrl;
    fadeCounter = &gFrameCounter;

    if (fadeTimer > 0x0F)
        *fadeCounter = (fadeTimer - 0x10) >> 1;

    if (*fadeCounter > 0x0F) {
        sceneCtrl[0] = (u32)-1;
        gCallbackStateArray[1] = (u32)UpdateSceneTransition;
    }

    m4aSoundVSyncOff();
    m4aMPlayAllStop();
}
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateSceneTransition);
INCLUDE_ASM("asm/nonmatchings/gfx", SetupSceneGfx);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateUIElementAnimation);
INCLUDE_ASM("asm/nonmatchings/gfx", InitSceneGfxByType);
INCLUDE_ASM("asm/nonmatchings/gfx", LoadBGPalette);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateBGPaletteAnimation);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateMenuCursorInput);
INCLUDE_ASM("asm/nonmatchings/gfx", SetupLevelTilemap);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateWorldMapScene);

/**
 * ReadUnalignedU16: reads an unsigned 16-bit value from a potentially
 * unaligned address. Assembles two bytes in little-endian order.
 */
u32 ReadUnalignedU16(u8 *ptr) {
    return ptr[0] | (ptr[1] << 8);
}

/**
 * ReadUnalignedS16: reads a signed 16-bit value from a potentially
 * unaligned address. Assembles two bytes in little-endian order with sign extension.
 */
s16 ReadUnalignedS16(u8 *ptr) {
    return (s16)(ptr[0] + (ptr[1] << 8));
}

/**
 * ReadUnalignedU32: reads an unsigned 32-bit value from a potentially
 * unaligned address. Assembles four bytes in little-endian order.
 */
u32 ReadUnalignedU32(u8 *ptr) {
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 16) + (ptr[3] << 24);
}
s32 ReadUnalignedU32_Alt(u8 *ptr) {
    return ptr[0] + (ptr[1] << 8) + (ptr[2] << 0x10) + (ptr[3] << 0x18);
}
/**
 * CalcBGScrollMapSize: pick the BGCNT screen-size field for a background of
 * the given tile dimensions.
 *
 * Affine backgrounds are square, so the field is the smallest of 16/32/64/128
 * tiles that holds both dimensions. Text backgrounds encode the two axes
 * independently: bit 0 marks a map wider than 32 tiles, bit 1 a taller one.
 */
u8 CalcBGScrollMapSize(u8 isAffine, u16 width, u16 height) {
    u8 wideBit;
    u8 size;

    if (isAffine) {
        if (width > 16 || height > 16) {
            if (width > 32 || height > 32) {
                return (width <= 64 && height <= 64) ? 2 : 3;
            } else {
                return 1;
            }
        } else {
            return 0;
        }
    } else {
        wideBit = 0;
        if (width > 32)
            wideBit = 1;
        if (height > 32)
            size = 2 | wideBit;
        else
            size = wideBit;
        return size;
    }
}
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateAffineRegisters);
/**
 * DecompressAndDmaCopy: decompress ROM data and DMA to a VRAM destination.
 *
 * Allocates a temp buffer, decompresses the source data (processing the
 * 4-byte sub-header), DMAs the payload (skipping the sub-header) to the
 * destination, waits for DMA completion, then frees the temp buffer.
 */
void DecompressAndDmaCopy(void *src, void *dest, u32 size) {
    void *buf = DecompressAlloc(src);
    Decompress(buf, src);
    DmaCopy16Wait(3, (u8 *)buf + 4, dest, size);
    thunk_HeapFree(buf);
}
/**
 * LoadBGTileData: load per-level tile data for one BG layer.
 *
 * Looks up the ROM tile pointer and VRAM destination from configuration
 * tables, then calls DecompressAndDmaCopy to decompress and DMA the
 * tile character data into the layer's assigned charblock.
 *
 *   levelIdx: level index (0-based)
 *   sublevel: sublevel / layer pair index
 *
 * MATCHING: every lookup is written out in full instead of being cached in a
 * local. agbcc expands the call's argument trees in source order, so keeping
 * the table reads inside the argument expression is what materialises both ROM
 * base addresses at function entry (r4/r5, hence `push {r4, r5, lr}`); hoisting
 * the two bytes into locals drops the register pressure to four live values,
 * agbcc then keeps everything in r0-r3 and the prologue loses r4/r5.
 * The `unk16 * unk18` operand order is also load-bearing: it fixes ldrh-before-
 * ldrb, and swapping it is the only difference between this and a 3-byte miss.
 */
void LoadBGTileData(s32 levelIdx, s32 sublevel) {
    DecompressAndDmaCopy((void *)gBgTileSubtable[gBgLayerLookup[levelIdx][sublevel][0]][gBgLayerLookup[levelIdx][sublevel][1] - 2],
                         gBgInfo[gBgLayerLookup[levelIdx][sublevel][1]].pTiles,
                         gBgInfo[gBgLayerLookup[levelIdx][sublevel][1]].unk16 * gBgInfo[gBgLayerLookup[levelIdx][sublevel][1]].unk18);
}
INCLUDE_ASM("asm/nonmatchings/gfx", LoadBGTilemapData);
INCLUDE_ASM("asm/nonmatchings/gfx", SetupLevelLayerConfig);
/**
 * FinalizeLevelLayerSetup: loads the level's BG palette into palette RAM.
 *
 * Looks up a compressed palette from ROM_LEVEL_PALETTE_TABLE (0x08189B4C)
 * by index, decompresses and DMAs 0x1C0 bytes to palette RAM (0x05000000).
 *
 *   idx: level palette index (u8, shifted to u32 table offset)
 */
void FinalizeLevelLayerSetup(u8 idx) {
    DecompressDma((void *)gLevelPaletteTable[idx], (void *)0x05000000, 0x1C0);
}
/**
 * LoadAndDecompressStream: decompress a data stream from ROM table entry.
 *
 * Looks up a compressed data pointer from gStreamDataTable by index,
 * allocates and decompresses it, stores the raw buffer in gDecompBuffer
 * and sets gStreamPtr to buffer+4 (past the header).
 */
void LoadAndDecompressStream(u32 idx) {
    void *buf = DecompressAlloc(gStreamDataTable[idx]);
    gDecompBuffer = buf;
    gStreamPtr = (u8 *)buf + 4;
}

/**
 * FreeDecompStreamBuffer: frees the decompressed stream buffer.
 */
void FreeDecompStreamBuffer(void) {
    thunk_HeapFree(gDecompBuffer);
}

/**
 * ClearScreenBufferB: DMA3-fills the IWRAM screenblock staging buffer B.
 *
 * Fills all 0x400 halfwords of gScreenBufferB (0x03001100) with 0xF000 in
 * fixed-source mode, using the macro's own stack-local halfword as the source.
 */
void ClearScreenBufferB(void) {
    DmaFill16(3, 0xF000, gScreenBufferB, 0x800);
}

/**
 * AllocAndClearGfxBuffer: allocate and DMA-fill a 32-byte GFX buffer.
 *
 * Allocates 32 bytes via thunk_HeapAlloc (malloc), stores the pointer
 * at gGfxBufferPtr, then DMA3-fills the buffer with zero using a
 * stack-local halfword as the fill source.
 */
void AllocAndClearGfxBuffer(void) {
    u32 *gfxBuf = &gGfxBufferPtr;
    u32 *buf = (u32 *)thunk_HeapAlloc(0x20, 0);
    *gfxBuf = (u32)buf;
    DmaFill16(3, 0, buf, 0x20);
}

/**
 * FreeGfxBuffer: frees the GFX buffer struct at gGfxBufferPtr.
 */
void FreeGfxBuffer(void) {
    thunk_HeapFree((void *)gGfxBufferPtr);
}

INCLUDE_ASM("asm/nonmatchings/gfx", DeadCode_0804bb86);

/**
 * AllocAndClearBuffer_52A4: allocate and DMA-fill a 1152-byte buffer.
 *
 * Allocates 0x480 bytes (0x90 << 3) via thunk_HeapAlloc, stores the
 * pointer at gBuffer_52A4, then DMA3-fills with zero using a stack-local
 * halfword as fill source.
 */
void AllocAndClearBuffer_52A4(void) {
    u32 *bufPtr = &gBuffer_52A4;
    u32 *buf = (u32 *)thunk_HeapAlloc(0x90 << 3, 0);
    *bufPtr = (u32)buf;
    DmaFill16(3, 0, buf, 0x480);
}
/** FreeBuffer_52A4: frees the memory buffer at gBuffer_52A4. */
void FreeBuffer_52A4(void) {
    thunk_HeapFree(gBuffer_52A4);
}
INCLUDE_ASM("asm/nonmatchings/gfx", SetupWorldMapBG);
/**
 * SetupTextBGLayer: initialize the text/UI BG layer for rendering.
 *
 * Sets up the BG layer state table entry: tile base, map base, scroll, map
 * width. Calls SoundDmaInit to load font tiles, DmaCopy16's the text palette
 * from ROM (0x080576B4) into palette RAM, sets REG_BG1CNT=0x700, clears the
 * BG1 scroll registers.
 */
void SetupTextBGLayer(void) {
    u8 *tbl = (u8 *)gBGLayerState;
    u32 base = *(u16 *)(tbl + 0x16) + 0x20;
    u16 zero;
    SoundDmaInit(base, 0x0804BB11, 0x1D, 0x10);
    *(u32 *)(tbl + 0x1C) = 0xC0 << 19;
    *(u32 *)(tbl + 0x20) = 0x06003800;
    zero = 0;
    *(u16 *)(tbl + 0x24) = zero;
    *(u16 *)(tbl + 0x26) = zero;
    *(u8 *)(tbl + 0x34) = 0x20;
    *(u16 *)(tbl + 0x30) = *(u16 *)(tbl + 0x16);
    DmaCopy16(3, (void *)0x080576B4, (void *)0x050001E0, 0x40);
    REG_BG1CNT = 0x700;
    REG_BG1HOFS = zero;
    REG_BG1VOFS = zero;
}
/**
 * ClearScreenBufferB_Alt: second, identical copy of the ClearScreenBufferB fill.
 *
 * Byte-for-byte the same routine as ClearScreenBufferB, emitted twice by the
 * original build.
 */
void ClearScreenBufferB_Alt(void) {
    DmaFill16(3, 0xF000, gScreenBufferB, 0x800);
}
/**
 * InitLevelStateDefaults: set the level's default window clip bounds and regs.
 *
 * Fills gLevelStatePtr's window-bounds fields (consumed by the HBlank handler
 * that drives REG_WIN0H/V and REG_WIN1H/V): window 0 spans (0,0x700)-(0xE80,
 * 0xA00), window 1 left/bottom = 0x700/0xA00. Then calls UpdateAffineRegisters,
 * sets REG_WININ=0x1F23 and REG_WINOUT=0x003D, and clears the OBJ-window enable
 * (bit 14) of REG_DISPCNT.
 */
void InitLevelStateDefaults(void) {
    struct LevelWindowBounds *win = gLevelStatePtr;

    win->leftTop[0][0] = 0; /* win0 left */
    win->rightBottom[0][0] = 0xE80; /* win0 right */
    win->leftTop[0][1] = 0x700; /* win0 top */
    win->rightBottom[0][1] = 0xA00; /* win0 bottom */
    win->leftTop[1][0] = 0x700; /* win1 left */
    win->rightBottom[1][1] = 0xA00; /* win1 bottom */
    UpdateAffineRegisters();
    REG_WININ = 0x1F23;
    REG_WINOUT = 0x3D;
    REG_DISPCNT &= 0xBFFF;
}
void VBlankHandler_WithWindowScroll(void);
void UpdateBGScrollWithWave(void);
void ReadKeyInput(void);
void sub_0804EB64(void);
void VBlankCallback_MapScreen(void);
/**
 * SetupGfxCallbacks: initializes VBlank/HBlank handlers and callback state
 * for the world map screen.
 */
void SetupGfxCallbacks(void) {
    u32 *vblankHandlers = (u32 *)gVBlankCallbackArray;
    u32 *callbackState;
    u32 slotIdx;
    vblankHandlers[0] = (u32)VBlankHandler_WithWindowScroll;
    vblankHandlers[1] = (u32)UpdateBGScrollWithWave;

    callbackState = gCallbackStateArray;
    callbackState[0x28 / 4] = (u32)ReadKeyInput;
    callbackState[0x2C / 4] = (u32)sub_0804EB64;
    callbackState[0x30 / 4] = (u32)VBlankCallback_MapScreen;
    callbackState[0x34 / 4] = 1;
    slotIdx = *((u8 *)callbackState + 0x78) - 1;
    callbackState[slotIdx] = 0;
    *((u8 *)callbackState + 0x79) = 4;
}
INCLUDE_ASM("asm/nonmatchings/gfx", InitWorldMapGfx);
/**
 * ShutdownGfxSubsystem: tears down the graphics subsystem on scene exit.
 *
 * Saves the current scene callback, disables HBlank IRQ and HBlank STAT,
 * then shuts down the graphics stream, sound, and frees all three
 * dynamically allocated graphics buffers.
 */
void ShutdownGfxSubsystem(void) {
    vu32 *dest = (vu32 *)&gUnk_03000814;
    u32 *src = (u32 *)gControlBlock;
    *dest = src[1];

    REG_IE &= 0xFFFD; /* clear INT_HBLANK */
    REG_DISPSTAT &= 0xFFEF; /* clear HBLANK_IRQ */

    ShutdownGfxStream();
    FreeSoundStruct();
    FreeBuffer_52A4();
    FreeGfxBuffer();
    FreeDecompStreamBuffer();
    m4aMPlayAllStop();
}
/**
 * InitGfxStreamState: allocate stream buffer, clear OAM, reset state.
 *
 * Allocates 0x100 bytes for gGfxStreamBuffer, DMA-fills with zero,
 * calls ClearVideoState, DMA-copies OAM shadow to hardware OAM,
 * sets stream mode to 0x0D, initializes two write cursor pointers.
 */
void InitGfxStreamState(void) {
    u16 zero_src;
    u32 *bufPtr = &gGfxStreamBuffer;
    u32 *buf = (u32 *)thunk_HeapAlloc(0x80 << 1, 0);
    volatile u32 *dma;
    u32 sp_ptr = (u32)&zero_src;
    *bufPtr = (u32)buf;
    zero_src = 0;
    dma = (volatile u32 *)REG_ADDR_DMA3SAD;
    dma[0] = sp_ptr;
    dma[1] = (u32)buf;
    dma[2] = 0x81000080;
    dma[2];
    ClearVideoState();
    dma[0] = (u32)gOamBuffer;
    dma[1] = 0xE0 << 19;
    dma[2] = 0x84000100;
    dma[2];
    gUnk_03005428 = 0x0D;
    gVramWriteCursor = gVramCursorInit;
    gPaletteVramCursor = gPaletteCursorInit;
}
/**
 * ResetGfxStreamEntries: releases every OBJ tile allocation the graphics stream
 * owns and rewinds the stream to its initial state.
 *
 * Walks the 32-slot allocation table at *gGfxStreamBuffer from the back, and for
 * each slot still holding tiles (tileCount != 0) frees its heap block — the block
 * starts 4 bytes before pTiles — then blanks the slot. Finally clears OAM, puts
 * the renderer back in mode 0x0D and rewinds both VRAM write cursors, so the next
 * LoadGfxStreamEntry starts allocating OBJ tiles from the top again.
 */
void ResetGfxStreamEntries(void) {
    struct GfxStreamAlloc *entry;
    s32 i;

    for (i = 32; i > 0; i--) {
        u32 base = gGfxStreamBuffer;

        entry = (struct GfxStreamAlloc *)(i * sizeof(struct GfxStreamAlloc) + base) - 1;
        if (entry->tileCount != 0) {
            thunk_HeapFree(entry->pTiles - 4);
            /* The call forces gGfxStreamBuffer to be re-read, so the original
             * recomputes the slot address here instead of reusing `entry`. */
            base = i * sizeof(struct GfxStreamAlloc) + gGfxStreamBuffer;
            base -= sizeof(struct GfxStreamAlloc);
            entry = (struct GfxStreamAlloc *)base;
            entry->tileCount = 0;
            entry->pTiles = 0;
            entry->tileIndex = 0;
        }
    }
    ClearVideoState();
    gUnk_03005428 = 0x0D;
    gVramWriteCursor = gVramCursorInit;
    gPaletteVramCursor = gPaletteCursorInit;
}
/**
 * StreamCmd_ResetEntries: stream command handler that resets all entries.
 *
 * Calls ResetGfxStreamEntries to free all active stream entries,
 * then advances the data stream pointer by 2.
 */
void StreamCmd_ResetEntries(void) {
    ResetGfxStreamEntries();
    gStreamPtr += 2;
}
/*
 * Shuts down the graphics stream: calls ResetGfxStreamEntries to finalize,
 * then frees the buffer at 0x030007C8 via thunk_HeapFree.
 *   no parameters
 *   no return value
 */
void ShutdownGfxStream(void) {
    ResetGfxStreamEntries();
    thunk_HeapFree(gGfxStreamBuffer);
}
/**
 * LoadGfxStreamEntry: give one OBJ tileset a slot in the stream's tile allocator.
 *
 * Finds the first free GfxStreamAlloc slot by walking the table until a slot with
 * tileCount == 0, accumulating every live slot's tileCount into the OBJ tile index
 * the new slot gets. The walk starts at gPaletteCursorInit converted from an OBJ
 * VRAM address to a tile id ((cursor - 0x06010000) / 32), which is the same
 * tiles-not-bytes convention DmaSpriteToObjVram reverses.
 *
 * Then it decompresses the tileset onto the heap and stores buf + 4 (past the heap
 * header), copies the ROM row's tileCount, and writes the tile index it computed.
 * With loadPalette set it also DMAs the tileset's 32-byte OBJ palette to
 * gVramWriteCursor and advances that cursor one palette bank.
 *
 *   idx:         tileset id, the low 7 bits of the stream command byte
 *   loadPalette: the command byte's high bit
 *
 * MATCHING: the allocator table must be reached through the `gGfxStreamAllocs`
 * symbol_ref, and the three post-call stores must index it by name. Measured by
 * ablation, one change at a time, rebuilding the module each time and scoring
 * build/src/gfx.o against expected/src/gfx.o:
 *
 *     as shipped                                          4
 *     no pointer local at all, named array everywhere      4
 *     pointer local used for the post-call stores too     35
 *     gGfxStreamBuffer macro (a CONST_INT) instead        18
 *
 * So the pointer local in the loop is inert -- keep it or drop it -- while reusing
 * it after the DecompressAlloc call costs 31, and the cast-address spelling costs
 * 14. The baseline is 4 rather than 0 because a plain module build does not run
 * scripts/pool_abs_syms.sh, so the four named data globals are still undefined
 * relocations where the ROM-derived target has bare numbers; only the deltas mean
 * anything. `make compare` is what says this function is byte-exact.
 *
 * The mechanism behind the 31 is not established. What is observed is that after
 * the call agbcc reloads the base from the pool when the array is named and reuses
 * the stale local when it is not; do not repeat the earlier draft's claim about
 * partial redundancy elimination on the loop guard, which the first two rows above
 * refute.
 */
void LoadGfxStreamEntry(u32 idx, u32 loadPalette) {
    struct GfxStreamAlloc *slot;
    u32 buf;
    u16 tileIndex;
    s32 i;

    i = 0;
    tileIndex = (gPaletteCursorInit - OBJ_VRAM) >> 5;
    slot = gGfxStreamAllocs;
    while (slot[i].tileCount != 0) {
        tileIndex += slot[i].tileCount;
        i++;
    }
    buf = (u32)DecompressAlloc(gStreamTilesetTable[idx].pTiles);
    gGfxStreamAllocs[i].pTiles = buf + 4;
    gGfxStreamAllocs[i].tileCount = gStreamTilesetTable[idx].tileCount;
    gGfxStreamAllocs[i].tileIndex = tileIndex;

    if (loadPalette != 0) {
        DmaCopy16Wait(3, gStreamPaletteTable[idx], gVramWriteCursor, 0x20);
        gVramWriteCursor += 0x20;
    }
}
/*
 * Reads a command byte from the data stream, splits it into a 7-bit value
 * and a 1-bit flag, then dispatches to LoadGfxStreamEntry. Advances stream by 3.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void DispatchStreamCommand_C0EC(void) {
    u8 **gp = &gStreamPtr;
    u8 *ptr = *gp;
    u8 byte = ptr[2];
    u8 val = byte & 0x7F;
    u8 flag = byte >> 7;
    *gp = ptr + 3;
    LoadGfxStreamEntry(val, flag);
}
/**
 * DmaSpriteToObjVram: DMA sprite tile data from ROM to OBJ VRAM.
 */
void DmaSpriteToObjVram(u32 entryIdx, u32 frameIdx) {
    vu32 *dma3 = &REG_DMA3SAD;
    u32 base = *(vu32 *)&gGfxStreamBuffer;
    u8 *entry = (u8 *)(entryIdx * 8 + base);
    u16 tileCount = *(u16 *)(entry + 6);
    dma3[0] = *(u32 *)entry + tileCount * (frameIdx << 5);

    dma3[1] = (u32) * (u16 *)(entry + 4) * 32 + OBJ_VRAM;

    dma3[2] = (*(u16 *)(entry + 6) << 4) | (0x80 << 24);
    dma3[2];
}
/**
 * StreamCmd_DmaSpriteData: reads sprite entry/frame from stream, DMAs to OBJ VRAM.
 */
void StreamCmd_DmaSpriteData(void) {
    u8 *ptr = gStreamPtr;
    u8 a = ptr[2];
    u8 b = ptr[3];
    gStreamPtr = ptr + 4;
    DmaSpriteToObjVram(a, b);
}
void SetSpriteTableFromIndex(u32 arg0) {
    gSpriteRenderPtr = gSpriteDataTable[arg0];
}
/*
 * Reads a command byte from the data stream and processes it via SetSpriteTableFromIndex.
 * Byte[2] is the command argument. Advances the stream pointer by 3.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void ProcessStreamCommand_C218(void) {
    SetSpriteTableFromIndex(gStreamPtr[2]);
    gStreamPtr += 3;
}
/**
 * StreamCmd_ConfigureSprite: place a stream-owned entity on screen.
 *
 * Stream layout (7 bytes):
 *   [2] bits 0-6: entity slot, relative to the stream-owned entity window at
 *                 index 0xD (the same +0xD bias StreamCmd_SetEntityTransform uses)
 *       bit 7   : "already on screen" flag, stored verbatim into onScreen; when it
 *                 is clear the entity is (re)introduced and unkF is primed to 0x1C
 *   [3..4] unaligned u16 X in pixels
 *   [5..6] unaligned u16 Y in pixels
 *
 * The pixel coordinates land in xPosScreen/yPosScreen and, shifted left by 4,
 * in the subpixel pair xPosBg2/yPosBg2 -- the same <<4 subpixel convention
 * StreamCmd_SetBGScroll uses. The two subpixel stores re-read the halfword they
 * just wrote instead of reusing the value in a register; that is what the
 * original does and writing it any other way loses the match.
 *
 * agbcc matching notes:
 *   - gUnk_03002920 must be indexed as the ARRAY, never through a local
 *     `struct Unk_03002920 *` copy. With a pointer local, gcc reassociates
 *     `(i + 0xD) * 0x1C` into `i * 0x1C + 0x16C`, hoists the 0x16C into a
 *     callee-saved register and burns r7; indexing the array directly keeps the
 *     `add #0xD` ahead of the multiply, which is what the ROM does.
 *   - the first half must NOT cache gStreamPtr in a local: letting each
 *     statement re-read it lets CSE keep one reload alive across the
 *     xPosScreen store and the `+5` argument, which is what puts the first
 *     ReadUnalignedU16 result in r3.
 */
void StreamCmd_ConfigureSprite(void) {
    u8 *p;
    u16 v;

    gUnk_03002920[(gStreamPtr[2] & 0x7F) + 0xD].onScreen = gStreamPtr[2] >> 7;
    gUnk_03002920[(gStreamPtr[2] & 0x7F) + 0xD].unkF = (gStreamPtr[2] >> 7) ? 0 : 0x1C;
    v = ReadUnalignedU16(gStreamPtr + 3);
    gUnk_03002920[(gStreamPtr[2] & 0x7F) + 0xD].xPosScreen = v;
    v = ReadUnalignedU16(gStreamPtr + 5);
    p = gStreamPtr;
    gUnk_03002920[(p[2] & 0x7F) + 0xD].yPosScreen = v;
    gUnk_03002920[(p[2] & 0x7F) + 0xD].xPosBg2 = gUnk_03002920[(p[2] & 0x7F) + 0xD].xPosScreen << 4;
    gUnk_03002920[(p[2] & 0x7F) + 0xD].yPosBg2 = gUnk_03002920[(p[2] & 0x7F) + 0xD].yPosScreen << 4;
    gStreamPtr = p + 7;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetupOAMSpriteGroup);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetEntityFlags);
/**
 * StreamCmd_SetEntityTransform: point an entity at an OBJ affine matrix and
 * load that matrix with a uniform scale.
 *
 * Stream layout (6 bytes):
 *   [2] entity slot, relative to the stream-owned entity window at index 0xD
 *   [3] bits 0-4: OBJ affine matrix number (also stored in the entity)
 *       bit 5   : OBJ affine enable
 *       bit 7   : OBJ affine double-size bounding box
 *   [4..5] unaligned s16 magnification (Q_8_8)
 *
 * pa = pd = 0x100 / mag with pb = pc = 0 is a pure uniform scale: no rotation,
 * no shear. The hardware matrix maps screen space back to texture space, so a
 * *larger* pa/pd yields a *smaller* sprite (see struct OamAffineMatrix, whose
 * field names are proven in docs/dynamic-analysis/).
 */
void StreamCmd_SetEntityTransform(void) {
    s16 mag;

    gUnk_03002920[gStreamPtr[2] + 0xD].affineHFlip_matrixNum = gStreamPtr[3] & 0x1F;
    gUnk_03002920[gStreamPtr[2] + 0xD].affineEnable = (gStreamPtr[3] >> 5) & 1;
    gUnk_03002920[gStreamPtr[2] + 0xD].affineDouble = gStreamPtr[3] >> 7;
    mag = ReadUnalignedS16(gStreamPtr + 4);
    gOamAffineMatrix[gStreamPtr[3] & 0x1F].pa = MultiplyQ8(0x100, ReciprocalQ8(mag));
    gOamAffineMatrix[gStreamPtr[3] & 0x1F].pb = 0;
    gOamAffineMatrix[gStreamPtr[3] & 0x1F].pc = 0;
    gOamAffineMatrix[gStreamPtr[3] & 0x1F].pd = MultiplyQ8(0x100, ReciprocalQ8(mag));
    gStreamPtr += 6;
}
/**
 * StreamCmd_SetBGPriority: sets the priority field of one BGxCNT register from
 * stream byte[2], then advances the stream by 3.
 *
 * Byte 2 packs both operands: bits 0-1 pick the layer (0-3 -> REG_BG0CNT..BG3CNT)
 * and bits 4-7 carry the priority value, which is OR-ed in after the register's
 * low two bits are cleared (& 0xFFFC). Only bits 4-5 of that nibble can be set in
 * practice -- bits 6-7 would land on BGxCNT's charblock field.
 *
 * All four arms are the same expression; the asymmetry in the ROM (case 0 extracts
 * the nibble with a bare `lsr #4`, the other three with `lsl #24 / lsr #28`) is
 * agbcc's, not the source's. What makes it appear is that gStreamPtr[2] is re-read
 * in every arm rather than cached in a local, which is the same house style as the
 * neighbouring stream commands. Cache the byte in a local instead and all four arms
 * collapse to `lsr #4` and cross-jump into one block, which costs the match.
 *
 * The discriminator inside agbcc was NOT identified. An earlier version of this
 * comment blamed a reg-to-reg copy from global CSE evaluated while
 * nonzero_sign_valid was 0. Both halves are wrong: agbcc's own RTL shows all four
 * arms reading one pseudo set exactly once, directly from the MEM, so there is no
 * copy; and -fno-gcse leaves the asymmetry untouched (it only drops the
 * callee-saved copy of gStreamPtr's address, 172 -> 168 bytes), so gcse is not the
 * discriminator either. Do not repeat the explanation; the
 * behaviour is reproducible, the cause is open.
 */
void StreamCmd_SetBGPriority(void) {
    switch (gStreamPtr[2] & 3) {
        case 0:
            REG_BG0CNT = (REG_BG0CNT & 0xFFFC) | ((gStreamPtr[2] >> 4) & 0xF);
            break;
        case 1:
            REG_BG1CNT = (REG_BG1CNT & 0xFFFC) | ((gStreamPtr[2] >> 4) & 0xF);
            break;
        case 2:
            REG_BG2CNT = (REG_BG2CNT & 0xFFFC) | ((gStreamPtr[2] >> 4) & 0xF);
            break;
        case 3:
            REG_BG3CNT = (REG_BG3CNT & 0xFFFC) | ((gStreamPtr[2] >> 4) & 0xF);
            break;
    }
    gStreamPtr += 3;
}

/**
 * StreamCmd_FillBGTilemap: stream command that clears one BG layer to a single
 * tile, then paints that layer's whole tilemap with that tile.
 *
 * Stream layout (7 bytes):
 *   [2]    BG layer index (0..3)
 *   [3..6] unaligned u32 fill pattern for the tile's pixels
 *
 * Two DMA fills:
 *   1. 32 bytes (one 4bpp tile) of the unaligned pattern into the layer's
 *      charblock at +0x40, i.e. tile index 2.
 *   2. 0x800 bytes (0x400 entries) of 0xF002 — tile 2, palette 15 — over the
 *      layer's tilemap. Layers 0 and 1 are painted into the IWRAM scratch
 *      buffers gBgTilemapBufs[i]; layers 2 and 3 straight into their VRAM
 *      screenbase, gBgInfo[i].pTilemap. Any other index does nothing beyond
 *      the tile fill.
 *
 * Advances the stream pointer by 7.
 */
void StreamCmd_FillBGTilemap(void) {
    /* `entry` must be a u16 local, not 0xF002 written inline at both call sites:
     * inlining it costs the match. */
    u16 entry;

    DmaFill32(3, ReadUnalignedU32(gStreamPtr + 3), (u8 *)gBgInfo[gStreamPtr[2]].pTiles + 0x40, 32);

    entry = 0xF002;
    switch (gStreamPtr[2]) {
        case 0:
        case 1:
            DmaFill16(3, entry, &gBgTilemapBufs[gStreamPtr[2]], 0x800);
            break;
        case 2:
        case 3:
            DmaFill16(3, entry, gBgInfo[gStreamPtr[2]].pTilemap, 0x800);
            break;
    }

    gStreamPtr += 7;
}
/**
 * StreamCmd_EnableMosaic: enables mosaic on BG2/BG3 and sets mosaic level.
 *
 * Sets bit 6 (mosaic) on REG_BG2CNT and REG_BG3CNT, reads a 4-bit
 * mosaic value from stream byte[2], stores to gMosaicSize (0x030007D8).
 * Advances stream pointer by 3.
 */
void StreamCmd_EnableMosaic(void) {
    vu16 *bg2cnt = &REG_BG2CNT;
    *bg2cnt |= 0x40;
    bg2cnt++;
    *bg2cnt |= 0x40;

    gMosaicSize = gStreamPtr[2] & 0x0F;
    gStreamPtr += 3;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetSpriteAttrs);
/**
 * StreamCmd_SetRenderMode: set render mode to 2.
 * Clears low 2 bits of gGfxBuffer[0], sets bit 1. Advances stream by 2.
 */
void StreamCmd_SetRenderMode(void) {
    s8 *p = (s8 *)gGfxBufferPtr;
    *p = (*p & ~3) | 2;
    gStreamPtr += 2;
}
void SetupLevelLayerConfig(u32 sceneIdx, u32 layerIdx);
void LoadBGTilemapData(u32 sceneIdx, u32 layerIdx);
/**
 * DispatchLevelLayerSetup: set up every BG layer of one scene.
 *
 * Reads the scene index from the command stream (byte 2), advances the cursor
 * by 3, then walks that scene's layer list in gBgLayerLookup -- 4 bytes per
 * scene, two 2-byte layer slots, 0xFF terminating -- configuring, loading tiles
 * for and loading the tilemap of each layer in turn. Finally loads the scene's
 * BG palette from the first slot's entry index.
 *
 * The two-subscript form `gBgLayerLookup[sceneIdx][i]` is load-bearing, as is the
 * fact that the base is a real extern rather than a cast address constant. Both
 * failures have the same shape: agbcc strength-reduces the table address into a
 * pointer induction variable (`adds rN, #2` per iteration) and the function
 * shrinks 100 -> 92 bytes, where the original recomputes the whole address every
 * iteration. Writing the index flat as `sceneIdx * 4 + i * 2` does it; so does a
 * CONST_INT base.
 */
void DispatchLevelLayerSetup(void) {
    s32 sceneIdx;
    s32 i;

    sceneIdx = gStreamPtr[2];
    gStreamPtr += 3;
    i = 0;
    if (gBgLayerLookup[sceneIdx][0][0] != 0xFF) {
        do {
            SetupLevelLayerConfig(sceneIdx, i);
            LoadBGTileData(sceneIdx, i);
            LoadBGTilemapData(sceneIdx, i);
            i++;
        } while (i <= 1 && gBgLayerLookup[sceneIdx][i][0] != 0xFF);
    }
    FinalizeLevelLayerSetup(gBgLayerLookup[sceneIdx][0][0]);
}
/**
 * StreamCmd_SetBGScroll: set BG layer scroll from stream data.
 *
 * Stream format: byte[2]=layer index, bytes[3-4]=scrollX, bytes[5-6]=scrollY.
 * Reads each value via ReadUnalignedU16, shifts left 4 for subpixel precision,
 * and stores to gBGLayerState[layer].scrollX/Y. Advances the stream by 7.
 *
 * gStreamPtr is re-read after each ReadUnalignedU16 call (the call clobbers the
 * cached pointer); reusing the second read's `p` for both the scrollX index and
 * the scrollY argument, plus declaring `tbl` before `p`, reproduces the original
 * register schedule with no pins.
 */
void StreamCmd_SetBGScroll(void) {
    u16 scrollX = ReadUnalignedU16(gStreamPtr + 3);
    struct BGLayerState *tbl = gBGLayerState;
    u8 *p = gStreamPtr;
    u16 scrollY;
    u8 *p2;
    tbl[p[2]].scrollX = scrollX << 4;
    scrollY = ReadUnalignedU16(p + 5);
    p2 = gStreamPtr;
    tbl[p2[2]].scrollY = scrollY << 4;
    gStreamPtr = p2 + 7;
}
/*
 * Reads a palette color entry from a data stream and writes it to BG palette RAM.
 * The stream at *0x03004D84 is a packed format: byte[2] is the palette index,
 * bytes[3..4] are a little-endian RGB555 color value. Advances the stream pointer by 5.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value (writes directly to GBA palette RAM at 0x05000000)
 */
void WritePaletteColor(void) {
    u8 **gp = &gStreamPtr;
    u16 color = ReadUnalignedU16(*gp + 3);
    u8 *ptr = *gp;
    *(u16 *)(BG_PAL_RAM + ptr[2] * 2) = color;
    *gp = ptr + 5;
}
/*
 * Reads a 16-bit value from the data stream and writes it to two destinations:
 * the palette/color register at 0x03005420 and the mirror at 0x030034AC.
 * Advances the data stream pointer by 4.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void WriteStreamValue_Dual(void) {
    u16 *dest1 = &gBg2XMag;
    u8 **gp = &gStreamPtr;

    int val = ReadUnalignedU16(*gp + 2);
    gBg2YMag = val;
    *dest1 = val;

    *gp += 4;
}
/**
 * UpdateCursorBlink: updates cursor blink state based on frame counter.
 *
 * Copies 16 bytes of ROM cursor data to stack, then checks a flag
 * at gSoundInfo+0x17 bit 1. If set, writes the alternating blink
 * state (gControlBlock[4] >> 5 & 1) to gEntityArray + 0x17C.
 * If clear, writes 0 to disable the blink.
 */
void MemCopy(void *, const void *, u32);
void UpdateCursorBlink(void) {
    u8 buf[16];
    MemCopy(buf, (void *)ROM_SOUND_INIT_DATA, 0x10);

    if (*((u8 *)(*(u32 *)&gSoundInfo) + 0x17) & 2) {
        /* cAddr/entityBase/offset intermediates are load-bearing: they stop
           agbcc from constant-folding the macro addresses, preserving the
           base + [r0, #4] load and the movs/lsls/adds for the 0x17C offset.
           Folding them inline (e.g. gControlBlock + 4) breaks the match. */
        u32 cAddr;
        u32 entityBase;
        u32 val;
        u32 offset;
        entityBase = (u32)gEntityArray;
        cAddr = (u32)gControlBlock;
        val = (*(u32 *)(cAddr + 4) >> 5) & 1;
        offset = 0x17C;
        *(u8 *)(entityBase + offset) = val;
    } else {
        u32 entityBase = (u32)gEntityArray;
        u32 offset = 0x17C;
        *(u8 *)(entityBase + offset) = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessAnimationSteps);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateLinearInterpolation);
/**
 * CalcSinCosVelocity: compute one frame of a GfxStreamEntry's sine oscillation.
 *
 * Writes the X and Y velocity for this tick into out[0]/out[1]:
 * amplitude * gSineTable[(timer * angularStep) & 0xFF] >> 8, with the X amplitude
 * at +0x08 and the Y amplitude at +0x0A. Returns 1 once the entry's timer has run
 * out (timer <= 0), which tells ProcessMotionStep to stop the entry.
 */
u32 CalcSinCosVelocity(struct GfxStreamEntry *entry, s16 *out) {
    out[0] = ((s16)entry->unk_08 * SIN(((s16)entry->timer * entry->unk_1E) & 0xFF)) >> 8;
    out[1] = ((s16)entry->unk_0A * SIN(((s16)entry->timer * entry->unk_1E) & 0xFF)) >> 8;

    if ((s16)entry->timer <= 0)
        return 1;
    return 0;
}
/* Stub_0804CAC4: an empty function — 2 bytes of `bx lr` plus 2 bytes of alignment padding — sitting
 * between CalcSinCosVelocity and ProcessMotionStep. Nothing in the ROM references it, which is why
 * it has no symbol and why luvdis folded its bytes into the tail of CalcSinCosVelocity.s. It is a
 * separate function, not codegen belonging to the one above: its `bx lr` follows the interworking
 * epilogue (`pop {r4, r5, r6}; pop {r1}; bx r1`), which agbcc emits exactly once per function, and
 * `bx r1` followed by `bx lr` occurs nowhere else in the project's disassembly. Without it the ROM
 * is 4 bytes short from here on. */
void Stub_0804CAC4(void) { }
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessMotionStep);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessMotionStepExtended);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessStaticBGScroll);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessFrameAnimation);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessSpriteOscillation);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessStarfieldEffect);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitStarfield);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitLinearMotion);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitLinearMotionExt);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitRotationMotion);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitMotionWithPalette);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitAngleMotion);
/*
 * Shape shared by the StreamCmd_Init* handlers below (agbcc 2.95 matching notes).
 * StreamCmd_InitFrameAnimation and StreamCmd_InitHBlankWait predate it and still
 * carry the raw offA/offB form; they want the same treatment.
 *
 * The originals re-read gStreamPtr and gBuffer_52A4 once per short run of stores
 * instead of caching them, so each run gets its own scope with a fresh
 * `cmd`/`entries` pair. Hoisting a single pair to the top of the function makes
 * agbcc park them in callee-saved registers and changes the prologue.
 *
 * Inside a run, `idx` exists only to order the first `ldrb [cmd, #2]` ahead of the
 * gBuffer_52A4 load; the later stores index with `cmd[2]` directly, which re-reads
 * the byte exactly like the original (the intervening stores kill the cached load
 * anyway).
 *
 * One exception: a bitfield store whose value comes straight from the command
 * stream (`targetIndex = cmd[3]`) has to have its entry address computed in a
 * separate statement, scaled index first. Written as one expression, agbcc
 * schedules the `cmd[3]` load before the address arithmetic instead of after it.
 */
/**
 * StreamCmd_InitOscillation: initialize a sprite-oscillation entry from stream data.
 *
 * Writes the oscillation parameters from the command stream into the
 * GfxStreamEntry indexed by stream byte[2], then installs ProcessMotionStep as
 * the entry's per-tick callback and advances the stream by 9 bytes. Each tick
 * ProcessMotionStep adds trig[(entry[0x1E] * entry->timer) & 0xFF] * amplitude >> 8
 * to the scroll of the object selected by targetIndex, and decrements the timer.
 *
 *   byte[3] low nibble -> targetIndex: which object oscillates (BG layer index)
 *   byte[4] -> +0x08, byte[5] -> +0x0A: X and Y amplitude of the oscillation
 *   bytes[6-7] (unaligned s16) -> timer: how many ticks it runs, and the phase
 *                                 argument (phase = byte[8] * timer)
 *   byte[8] -> +0x1E: angular step per tick, i.e. the oscillation frequency
 *
 * It also clears the target-mode selector (0x07F8 of the halfword at +0x00) to 0,
 * selecting the BG-layer target mode, and sets the entry type nibble to 1.
 *
 * Field meanings verified at runtime against the ROM's own ProcessMotionStep —
 * see docs/dynamic-analysis/scripts/prove-gfxstream-motion-fields.mjs.
 */
void StreamCmd_InitOscillation(void) {
    s16 duration;

    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;
        struct GfxStreamEntry *entry;
        u8 target;

        entry = (struct GfxStreamEntry *)(idx * sizeof(struct GfxStreamEntry) + (u32)entries);
        target = cmd[3];
        entry->targetIndex = target;
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_08 = cmd[4];
        entries[cmd[2]].unk_0A = cmd[5];
        duration = ReadUnalignedS16(cmd + 6);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].timer = duration;
        entries[cmd[2]].unk_1E = cmd[8];
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].param = 0;
        entries[cmd[2]].callback = (u32)ProcessMotionStep;
        entries[cmd[2]].type = 1;
    }
    gStreamPtr += 9;
}
/**
 * StreamCmd_InitOscillationExt: start a sine oscillation on a gfx-stream target.
 *
 * Fills the GfxStreamEntry selected by stream byte[2] with the parameters of a
 * sine wave and installs ProcessMotionStep as its per-frame callback, so that
 * every frame the entry adds `amplitude * gSineTable[(timer * angularStep) & 0xFF] >> 8`
 * to its target's X and Y, counts `timer` down by one, and deactivates itself
 * when the countdown is spent.
 *
 * Stream layout (9 bytes):
 *   [2] entry index          [3] target object index (7 bits, biased +13 by the handler)
 *   [4] X amplitude          [5] Y amplitude
 *   [6..7] duration in frames (unaligned s16, also the oscillation phase)
 *   [8] angular step per frame
 *
 * The "Ext" variant additionally forces the entry's target selector (word 0,
 * bits 3..10) to 2, which makes ProcessMotionStep drive a gUnk_03002920 object
 * rather than a BG scroll pair.
 *
 * Every meaning above is verified at runtime against the real ROM by
 * docs/dynamic-analysis/scripts/prove-oscillation-fields.mjs.
 */
void StreamCmd_InitOscillationExt(void) {
    s16 duration;

    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].objIndex = cmd[3];
        entries[cmd[2]].unk_08 = cmd[4];
        entries[cmd[2]].unk_0A = cmd[5];
        duration = ReadUnalignedS16(cmd + 6);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].timer = duration;
        entries[cmd[2]].unk_1E = cmd[8];
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].param = 2;
        entries[cmd[2]].callback = (u32)ProcessMotionStep;
        entries[cmd[2]].type = 1;
    }
    gStreamPtr += 9;
}
/**
 * StreamCmd_InitStaticScroll: initialize a static BG-scroll entry from stream data.
 *
 * Reads the entry index from stream byte[2], stores the per-frame X/Y scroll steps
 * from bytes[4]/[5] into the GfxStreamEntry, selects which BG layer to scroll from
 * byte[3] (the 4-bit targetIndex into gBGLayerState), clears the packed 8-bit param
 * field so ProcessStaticBGScroll's gate is open, installs ProcessStaticBGScroll as the
 * per-frame callback, sets the entry type to 1 (active), then advances the stream by
 * 6 bytes.
 */
void StreamCmd_InitStaticScroll(void) {
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;
        struct GfxStreamEntry *entry;
        u8 target;

        entries[idx].unk_08 = cmd[4];
        entries[cmd[2]].unk_0A = cmd[5];

        entry = (struct GfxStreamEntry *)(cmd[2] * sizeof(struct GfxStreamEntry) + (u32)entries);
        target = cmd[3];
        entry->targetIndex = target;
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].param = 0;
        entries[cmd[2]].callback = (u32)ProcessStaticBGScroll;
        entries[cmd[2]].type = 1;
    }
    gStreamPtr += 6;
}
/**
 * StreamCmd_InitFrameAnimation: initialize a frame-animation entry from stream data.
 *
 * Reads packed parameters from the command stream (bytes 3..6) into the
 * GfxStreamEntry indexed by stream byte[2], installs ProcessFrameAnimation as
 * the per-frame callback, sets the entry type nibble to 1, then advances the
 * stream by 7 bytes.
 */
void StreamCmd_InitFrameAnimation(void) {
    u8 **streamPP = &gStreamPtr;
    u8 **basePP;
    u8 *sp;
    u8 idx;
    u8 *base;
    u32 offA;
    u32 offB;
    u8 *sp2;
    u8 *base2;
    u32 offC;
    u32 offD;
    u32 param6;
    u32 offE;
    u32 offF;
    u8 *entryF;
    u8 flags;
    s32 mask;

    sp = *streamPP;
    idx = sp[2];
    basePP = &gBuffer_52A4;
    base = *basePP;
    offA = (u32)(idx * 9) * 4;
    offA += (u32)base;
    *(u16 *)(offA + 0x04) = sp[3];

    idx = sp[2];
    offB = (u32)(idx * 9) * 4;
    offB += (u32)base;
    *(u8 *)(offB + 0x1E) = sp[4];
    *(u16 *)(offB + 0x0C) = sp[4];

    sp2 = *streamPP;
    idx = sp2[2];
    base2 = *basePP;
    offC = (u32)(idx * 9) * 4;
    offC += (u32)base2;
    *(u8 *)(offC + 0x1F) = sp2[5];

    sp = *streamPP;
    idx = sp[2];
    base = *basePP;
    offD = (u32)(idx * 9) * 4;
    offD += (u32)base;
    param6 = sp[6];
    *(u16 *)(offD + 0x08) = param6;
    *(u16 *)(offD + 0x14) = param6;

    idx = sp[2];
    offE = (u32)(idx * 9) * 4;
    offE += (u32)base;
    *(u32 *)(offE + 0x20) = (u32)ProcessFrameAnimation;

    idx = sp[2];
    offF = (u32)(idx * 9) * 4;
    offF += (u32)base;
    entryF = (u8 *)offF;
    flags = entryF[0];
    mask = -8;
    mask &= flags;
    entryF[0] = mask | 1;

    *streamPP += 7;
}
/**
 * ProcessHBlankWait: process HBlank wait timer for a stream entry.
 *
 * Enables HBlank interrupt (bit 1 in REG_IE) and VCount interrupt
 * (bit 4 in REG_DISPSTAT). Checks the entry's timer at offset 0x14;
 * if expired (negative after decrement), disables both interrupts
 * and returns 0. If bit 7 of entry[3] is set or timer still positive,
 * returns 1.
 *
 * @param idx  Stream entry index
 * @return     1 if still waiting, 0 if timer expired
 */
u32 ProcessHBlankWait(u32 idx) {
    volatile u16 *ie;
    volatile u16 *dispstat;
    u8 *buf;
    u32 off;
    u8 *entry;
    u32 timer;

    ie = &REG_IE;
    *ie |= 2;
    dispstat = &REG_DISPSTAT;
    *dispstat |= 0x10;

    buf = gBuffer_52A4;
    off = idx * 9;
    off <<= 2;
    off += (u32)buf;
    entry = (u8 *)off;

    if (entry[3] >> 7) {
        return 1;
    }
    timer = *(u16 *)(entry + 0x14) - 1;
    *(u16 *)(entry + 0x14) = timer;
    if ((s32)(timer << 16) < 0) {
        *ie &= 0xFFFD;
        *dispstat &= 0xFFEF;
        return 0;
    }
    return 1;
}
/**
 * StreamCmd_InitHBlankWait: initialize an HBlank wait entry from stream data.
 */
void StreamCmd_InitHBlankWait(void) {
    s16 timerVal;
    u8 **streamPP = &gStreamPtr;
    u8 **basePP;
    u8 *sp1;
    u8 idx1;
    u8 *base1;
    u32 offA;
    u32 offB;
    u8 *entryB;
    s16 val;
    u32 signBit;
    u8 *sp2;
    u8 idx2;
    u8 *base2;
    u32 offC;
    u32 offD;
    u8 *entryD;
    u8 flags;
    s32 mask;

    timerVal = ReadUnalignedS16(*streamPP + 3);

    sp1 = *streamPP;
    idx1 = sp1[2];
    basePP = &gBuffer_52A4;
    base1 = *basePP;

    offA = (u32)(idx1 * 9) * 4;
    offA += (u32)base1;
    *(s16 *)(offA + 0x14) = timerVal;

    idx1 = sp1[2];
    offB = (u32)(idx1 * 9) * 4;
    offB += (u32)base1;
    entryB = (u8 *)offB;
    val = *(s16 *)(entryB + 0x14);
    signBit = ((u32)val >> 31) << 7;
    entryB[3] = (entryB[3] & 0x7F) | signBit;

    sp2 = *streamPP;
    idx2 = sp2[2];
    base2 = *basePP;

    offC = (u32)(idx2 * 9) * 4;
    offC += (u32)base2;
    *(u32 *)(offC + 0x20) = (u32)ProcessHBlankWait;

    idx2 = sp2[2];
    offD = (u32)(idx2 * 9) * 4;
    offD += (u32)base2;
    entryD = (u8 *)offD;
    flags = entryD[0];
    mask = -8;
    mask &= flags;
    entryD[0] = mask | 1;

    *streamPP += 5;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitSpriteWave);
/**
 * StreamCmd_InitButtonWait: initialize a button-wait entry from stream data.
 *
 * Same shape as StreamCmd_InitHBlankWait: reads the s16 timeout from stream
 * bytes[3-4] into the GfxStreamEntry indexed by stream byte[2] (field `timer`),
 * then latches the timeout's sign bit into entry byte 0x1E — so a NEGATIVE
 * timeout means "ignore the timer, wait for the button however long it takes".
 * Installs ProcessButtonWait as the per-frame callback, sets the entry type
 * nibble to 1 (which is what makes ProcessAnimationSteps dispatch the entry),
 * switches the level-state render mode to 2, and advances the stream by 5.
 *
 * ProcessButtonWait ends the wait on a fresh A press (gNewKeys bit 0). It has no
 * thumb_func_start of its own — luvdis merged it into the tail of
 * asm/nonmatchings/gfx/ProcessSpriteOscillation.s at 0x0804D074 — so it is linked
 * through the ldscript.in.txt symbol instead of as a C function.
 * Both the name and the field meanings are backed by runtime evidence:
 * docs/dynamic-analysis/scripts/prove-button-wait.mjs
 */
void StreamCmd_InitButtonWait(void) {
    s16 timeout = ReadUnalignedS16(gStreamPtr + 3);

    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].timer = timeout;
        entries[cmd[2]].unk_1E = entries[cmd[2]].timer >> 15;
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].callback = (u32)ProcessButtonWait;
        entries[cmd[2]].type = 1;
    }
    {
        s8 *renderMode = (s8 *)gGfxBufferPtr;
        *renderMode = (*renderMode & ~3) | 2;
    }
    gStreamPtr += 5;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_StopMotion);
/**
 * ProcessScreenFade: step the active screen fade one frame toward its target.
 *
 * Only acts on odd global frames. Writes the fade's blend config into REG_BLDCNT
 * from a ROM lookup table indexed by the control struct's palette byte, then
 * increments or decrements gUnk_03005498 depending on the direction bit
 * (bit 6 of byte 1, extracted via `<<25 >>31`). When the counter reaches the
 * target level (byte 6) it clamps to that level, clears gPauseFlag, and returns 0
 * (fade complete); otherwise returns 1 (fade still in progress).
 */
s32 ProcessScreenFade(void) {
    u8 *p;
    u32 sign;
    u8 c;

    gUnk_030034E4 = 1;
    if (gUnk_03004C20.globalFrameCounter & 1) {
        REG_BLDCNT = gBlendModeTable[((u8 *)gGfxBufferPtr)[5]];
        p = (u8 *)gGfxBufferPtr;
        sign = ((u32)p[1] << 25) >> 31;
        if (sign != 0) {
            c = gUnk_03005498;
            gUnk_03005498 = c - 1;
            if ((u8)(c - 1) <= p[6]) {
                gUnk_030034E4 = 0;
                gUnk_03005498 = p[6];
                return 0;
            }
            return 1;
        } else {
            c = gUnk_03005498;
            gUnk_03005498 = c + 1;
            if ((u8)(c + 1) >= p[6]) {
                gUnk_030034E4 = sign;
                gUnk_03005498 = p[6];
                return 0;
            }
            return 1;
        }
    }
    return 1;
}
INCLUDE_ASM("asm/nonmatchings/gfx", UpdatePaletteFadeStep);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessSceneTransitionOut);
/**
 * StreamCmd_BeginSceneExit: stream command that ends the current scene.
 *
 * Clears the render-mode bits (low 2) of gGfxBuffer[0], advances the stream by
 * 2, then replaces the scene-exit control (gGfxBuffer[2] bits 1-2) with
 * GFX_SCENE_EXITING — so the next gfx-stream tick stops advancing the stream and
 * runs ProcessSceneTransitionOut() instead. This is the same pair of writes the
 * tick performs itself when START is pressed on a GFX_SCENE_SKIPPABLE scene.
 *
 * Both statements re-read gGfxBufferPtr because the gStreamPtr store between
 * them may alias it.
 */
void StreamCmd_BeginSceneExit(void) {
    *(s8 *)gGfxBufferPtr &= ~3;
    gStreamPtr += 2;
    ((s8 *)gGfxBufferPtr)[2] = (((s8 *)gGfxBufferPtr)[2] & ~(GFX_SCENE_EXITING | GFX_SCENE_SKIPPABLE)) | GFX_SCENE_EXITING;
}
/**
 * StreamCmd_SetRenderModeTiled: set render mode to 2.
 * Clears low 2 bits of gGfxBuffer[0], sets bit 1. Advances stream by 2.
 */
void StreamCmd_SetRenderModeTiled(void) {
    s8 *p = (s8 *)gGfxBufferPtr;
    *p = (*p & ~3) | 2;
    gStreamPtr += 2;
}
void StreamCmd_ClearRenderMode(void) {
    s8 *p;
    u8 **streamPtr;

    p = (s8 *)gGfxBufferPtr;
    *p &= -4;
    streamPtr = &gStreamPtr;
    *streamPtr += 2;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetTimerAndMode);
/**
 * StreamCmd_ToggleDisplayFlag: toggles one of two graphics-control flags,
 * selected by the command's argument byte, then advances the stream by 3.
 *
 * arg == 0 flips GfxControlFlags.sceneExit's HIGH bit -- byte bit 2, i.e.
 * GFX_SCENE_SKIPPABLE, which gates whether a START press ends the scene. It is
 * NOT GFX_SCENE_EXITING (byte bit 1), which is the bit that stops the stream
 * advancing and runs ProcessSceneTransitionOut; sub_0804EB64 tests the two
 * separately. `sceneExit ^= 2` reads as EXITING only if you mistake a byte mask
 * for a value in a 2-bit field at bit offset 1.
 *
 * Both arms are agbcc's canonical bitfield read-modify-write (extract with
 * lsl/lsr, xor, reinsert under a negated mask), so both must be spelled as
 * struct bitfields rather than hand-written shifts and masks. See the note on
 * `sceneExit` in include/gfx.h for why its container type is u32 while the
 * 0x1C group's is u8.
 */
void StreamCmd_ToggleDisplayFlag(void) {
    if (gStreamPtr[2] == 0) {
        /* the cast is repeated rather than held in a local: a `struct GfxControlFlags *`
         * local costs the match */
        ((struct GfxControlFlags *)gGfxBufferPtr)->sceneExit ^= GFX_SCENE_SKIPPABLE >> 1;
    } else {
        ((struct GfxControlFlags *)gGfxBufferPtr)->forceWindowsOpen ^= 1;
    }
    gStreamPtr += 3;
}
/**
 * StreamCmd_ToggleFadeDirection: flips GfxControlFlags.blendRampDown (byte 0x1C,
 * bit 5) of the graphics control block, then advances the stream pointer by 2.
 *
 * The flag reverses the scene-transition cross-fade: ProcessSceneTransitionOut
 * ramps gBlendValue down instead of up, and on underflow switches BG2 off and
 * clears the flag again. Proved at runtime in
 * docs/dynamic-analysis/scripts/prove-gfx-flag-1C-bit5.mjs — the name
 * "ToggleLayerFlag" predates that evidence and is a misnomer.
 *
 * The `lsl #26 / lsr #31` bit extraction plus the `~0x20` read-modify-write is
 * agbcc's canonical 1-bit bitfield toggle, so spelling the flag as a bitfield
 * (rather than hand-written shifts and masks) reproduces the register schedule.
 */
void StreamCmd_ToggleFadeDirection(void) {
    struct GfxControlFlags *ctl = (struct GfxControlFlags *)gGfxBufferPtr;
    ctl->blendRampDown ^= 1;
    gStreamPtr += 2;
}
/**
 * StreamCmd_SetBlendMode: set the hardware blend control (BLDCNT) from stream data.
 *
 * Copies the blend-mode index (stream byte[2]) into gGfxBuffer[5], looks it up in
 * the ROM blend-mode table at 0x08057B4C, and writes the resulting value to
 * REG_BLDCNT. Also stores the raw blend value (stream byte[3]) to gBlendValue.
 * Advances the stream by 4.
 */
void StreamCmd_SetBlendMode(void) {
    ((u8 *)gGfxBufferPtr)[5] = gStreamPtr[2];
    REG_BLDCNT = gBlendModeTable[((u8 *)gGfxBufferPtr)[5]];
    gUnk_03005498 = gStreamPtr[3];
    gStreamPtr += 4;
}
/**
 * StreamCmd_SetWindowCorner: writes one window-clip corner of the current
 * level's window bounds from the command stream.
 *
 * Stream layout: byte[2] = selector, byte[3] = horizontal value, byte[4] =
 * vertical value; the command is 5 bytes long. Selector bit 1 chooses which
 * edge group is written (clear -> rightBottom, set -> leftTop) and bit 0
 * chooses window 0 or window 1. Both values are stored as (byte << 4), i.e. the
 * 12.4 fixed-point form the HBlank handler packs into REG_WIN0H/WIN1H and
 * REG_WIN0V/WIN1V; UpdateAffineRegisters() then pushes the whole block out.
 *
 * Note that byte[2] is loaded twice: the store to the first halfword may alias
 * the command stream, so agbcc must re-read it for the second index. Keeping
 * both edges as 2x2 arrays (see struct LevelWindowBounds) is what makes the
 * member offset stay out of the store's immediate field, which is required for
 * this function to match.
 */
void StreamCmd_SetWindowCorner(void) {
    u8 *p = gStreamPtr;

    if (p[2] & 2) {
        struct LevelWindowBounds *win = gLevelStatePtr; /* declared per arm on purpose: hoisting it above the if costs the match */

        win->leftTop[p[2] & 1][0] = p[3] << 4;
        win->leftTop[p[2] & 1][1] = p[4] << 4;
    } else {
        struct LevelWindowBounds *win = gLevelStatePtr;

        win->rightBottom[p[2] & 1][0] = p[3] << 4;
        win->rightBottom[p[2] & 1][1] = p[4] << 4;
    }
    UpdateAffineRegisters();
    gStreamPtr += 5;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetBGScreenSize);
/**
 * StreamCmd_SetWindowRegs: writes WIN0H/WIN0V from stream bytes[2-5].
 * Reads two pairs of bytes from the command stream at gStreamPtr,
 * packs each pair into a 16-bit value (low | high<<8), and writes
 * them to REG_WININ (0x04000048) and REG_WINOUT (0x0400004A).
 * Advances stream by 6.
 */
void StreamCmd_SetWindowRegs(void) {
    vu16 *reg = &REG_WININ;
    u8 **streamPtrAddr = &gStreamPtr;
    u8 *stream = *streamPtrAddr;

    *reg = stream[2] | (stream[3] << 8);
    reg++;
    *reg = stream[4] | (stream[5] << 8);
    *streamPtrAddr = stream + 6;
}
/**
 * StreamCmd_EnableScrollMode: sets the scroll-enable bits in the gfx buffer
 * control byte at gGfxBufferPtr. Masks off the top bits and sets bit 6
 * (0x40), then sets the render mode to 2 (mask ~3 | 2). Advances stream by 2.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void StreamCmd_EnableScrollMode(void) {
    *(s8 *)gGfxBufferPtr = (*(s8 *)gGfxBufferPtr & 0x3F) | 0x40;
    *(s8 *)gGfxBufferPtr = (*(s8 *)gGfxBufferPtr & ~3) | 2;
    gStreamPtr += 2;
}
/**
 * StreamCmd_StopMusic: stream command to halt all music playback.
 * Calls m4aMPlayAllStop, advances stream by 2.
 */
void StreamCmd_StopMusic(void) {
    m4aMPlayAllStop();
    gStreamPtr += 2;
}
/*
 * Reads a command byte from the data stream and processes it via m4aSongNumStop.
 * Byte[2] is the command argument. Advances the stream pointer by 3.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void ProcessStreamCommand_50094(void) {
    m4aSongNumStop(gStreamPtr[2]);
    gStreamPtr += 3;
}
/*
 * Dispatches a sound/music stream command based on byte[2] of the data stream.
 * If byte[2] <= 0x22, passes it directly; otherwise re-reads and passes it.
 * Both paths call m4aSongNumStart. Advances the stream pointer by 3.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void DispatchMusicStreamCommand(void) {
    u8 *ptr = gStreamPtr;

    if (ptr[2] <= 0x22) {
        m4aSongNumStart(ptr[2]);
    } else {
        m4aSongNumStart(ptr[2]);
    }

    gStreamPtr += 3;
}
/**
 * StreamCmd_StopSound: stream command to stop sound effects.
 * Calls m4aMPlayAllContinue, advances stream by 2.
 */
void StreamCmd_StopSound(void) {
    m4aMPlayAllContinue();
    gStreamPtr += 2;
}
void StreamCmd_Nop3(void) {
    gStreamPtr += 3;
}
/**
 * StreamCmd_StopMusicAndDisableIRQ: stops all music and disables interrupts.
 * Calls m4aMPlayAllStop + m4aSoundVSyncOff, advances by 2.
 */
void StreamCmd_StopMusicAndDisableIRQ(void) {
    m4aMPlayAllStop();
    m4aSoundVSyncOff();
    gStreamPtr += 2;
}
/**
 * StreamCmd_DisableVBlank: disables VBlank interrupt and calls
 * m4aSoundVSyncOff. Advances stream by 2.
 */
void StreamCmd_DisableVBlank(void) {
    REG_IE &= ~IE_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();
    gStreamPtr += 2;
}
/*
 * Enables VBlank interrupt and VBlank IRQ status, then calls
 * m4aSoundVSyncOn to set up the handler. Advances the data stream by 2.
 *   no parameters
 *   no return value
 */
void EnableVBlankHandler(void) {
    REG_IE |= IE_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_IRQ_ENABLE;
    m4aSoundVSyncOn();
    gStreamPtr += 2;
}
/*
 * Enables VBlank interrupt and IRQ, sets up handler via m4aSoundVSyncOn,
 * then dispatches a music stream command via m4aSongNumStart using byte[2].
 * Advances the data stream pointer by 3.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void EnableVBlankAndDispatchMusic(void) {
    u8 **gp = &gStreamPtr;
    u8 *ptr = *gp;

    if (ptr[2] <= 0x22) {
        REG_IE |= 1;
        REG_DISPSTAT |= 8;
        m4aSoundVSyncOn();
        m4aSongNumStart((*gp)[2]);
    } else {
        REG_IE |= 1;
        REG_DISPSTAT |= 8;
        m4aSoundVSyncOn();
        m4aSongNumStart((*gp)[2]);
    }

    gStreamPtr += 3;
}
/**
 * StreamCmd_DisableVBlankAndStopMusic: disables the VBlank interrupt, detaches
 * the m4a VSync hook, stops every music player, and advances the stream by 2.
 *
 * Identical to StreamCmd_DisableVBlank above but with m4aMPlayAllStop() added
 * after m4aSoundVSyncOff(). Both registers are vu16, so the &= narrows to 16
 * bits and the pool word is 0x0000FFFE / 0x0000FFF7 either way — the named
 * constants and the literal masks compile to the same bytes.
 */
void StreamCmd_DisableVBlankAndStopMusic(void) {
    REG_IE &= ~IE_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();
    m4aMPlayAllStop();
    gStreamPtr += 2;
}
/*
 * Enables VBlank interrupt and VBlank IRQ status, then calls two
 * interrupt setup handlers (m4aSoundVSyncOn, m4aMPlayAllContinue).
 * Advances the data stream pointer by 2.
 *   no parameters
 *   no return value
 */
void EnableVBlankAndHandlers(void) {
    REG_IE |= IE_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_IRQ_ENABLE;
    m4aSoundVSyncOn();
    m4aMPlayAllContinue();
    gStreamPtr += 2;
}
struct MP2KPlayerState;
extern struct MP2KPlayerState gMPlayInfo_0, gMPlayInfo_1, gMPlayInfo_2, gMPlayInfo_3;
void m4aMPlayVolumeControl(struct MP2KPlayerState *, u16, u16);
/*
 * Reads a 16-bit master-volume value from the data stream and applies it to
 * all four music player slots (via m4aMPlayVolumeControl with full track mask
 * 0xFF), also mirroring it into the gfx buffer word[12] and gSceneFadeCounter.
 * Advances the data stream pointer by 4.
 *   no parameters (reads from global data stream pointer at 0x03004D84)
 *   no return value
 */
void StreamCmd_SetMusicParams(void) {
    int val = ReadUnalignedU16(gStreamPtr + 2);
    ((u16 *)gGfxBufferPtr)[12] = val;
    gSceneFadeCounter = val;
    m4aMPlayVolumeControl(&gMPlayInfo_0, 0xFF, gSceneFadeCounter);
    m4aMPlayVolumeControl(&gMPlayInfo_1, 0xFF, gSceneFadeCounter);
    m4aMPlayVolumeControl(&gMPlayInfo_2, 0xFF, gSceneFadeCounter);
    m4aMPlayVolumeControl(&gMPlayInfo_3, 0xFF, gSceneFadeCounter);
    gStreamPtr += 4;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_ConfigureBlend);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_RunScript);
