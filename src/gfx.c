#include "global.h"
#include "gba.h"
#include "globals.h"
#include "structs/variables.h"
#include "include_asm.h"

INCLUDE_ASM("asm/nonmatchings/gfx", InitGfxState);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateBGScrollRegisters);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdateBGTileAnimation);
void UpdateBGScrollRegisters(void);
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
INCLUDE_ASM("asm/nonmatchings/gfx", CalcBGScrollMapSize);
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
 */
INCLUDE_ASM("asm/nonmatchings/gfx", LoadBGTileData);
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

INCLUDE_ASM("asm/nonmatchings/gfx", ClearScreenBufferB);

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
INCLUDE_ASM("asm/nonmatchings/gfx", ClearScreenBufferB_Alt);
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

    win->win0Left = 0;
    win->win0Right = 0xE80;
    win->win0Top = 0x700;
    win->win0Bottom = 0xA00;
    win->win1Left = 0x700;
    win->win1Bottom = 0xA00;
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
    *(u8 *)&gRenderFlags = 0x0D;
    gVramWriteCursor = gVramCursorInit;
    gPaletteVramCursor = gPaletteCursorInit;
}
/**
 * ResetGfxStreamEntries: frees all active stream entries and resets state.
 * Iterates 32 entries in gGfxStreamBuffer, frees those with non-zero flags,
 * clears OAM, resets write cursors.
 */
INCLUDE_ASM("asm/nonmatchings/gfx", ResetGfxStreamEntries);
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
INCLUDE_ASM("asm/nonmatchings/gfx", LoadGfxStreamEntry); /* ProcessStreamOpcode */
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
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_ConfigureSprite);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetupOAMSpriteGroup);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetEntityFlags);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetEntityTransform);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetBGPriority);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_FillBGTilemap);
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

    gBldyFadeLevel = gStreamPtr[2] & 0x0F;
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
INCLUDE_ASM("asm/nonmatchings/gfx", DispatchLevelLayerSetup);
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
INCLUDE_ASM("asm/nonmatchings/gfx", CalcSinCosVelocity);
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
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitOscillation);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitOscillationExt);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitStaticScroll);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitFrameAnimation);
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
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_InitButtonWait);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_StopMotion);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessScreenFade);
INCLUDE_ASM("asm/nonmatchings/gfx", UpdatePaletteFadeStep);
INCLUDE_ASM("asm/nonmatchings/gfx", ProcessSceneTransitionOut);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetBGModeTiled);
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
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_ToggleDisplayFlag);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_ToggleLayerFlag);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetBlendMode);
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_SetScrollPosition);
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
    REG_IE &= 0xFFFE; /* clear INT_VBLANK */
    REG_DISPSTAT &= 0xFFF7; /* clear VBLANK_IRQ */
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
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_DisableVBlankAndStopMusic);
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
