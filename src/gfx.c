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
void ProcessSpriteOscillation(void);
extern s16 ReciprocalQ8(s16 a);
extern s16 MultiplyQ8(s16 a, s16 b);
/* DivideQ8 is defined in src/math.c with an s16 return type, but it sign-extends
 * its result before returning (the trailing `lsls #16 / asrs #16` in its ROM code),
 * so an s32-returning declaration observes exactly the same values. It has to be
 * declared that way here: with an s16 return type agbcc re-narrows the returned
 * value in the CALLER, and StreamCmd_InitBg2Zoom feeds the result to two u16
 * stores, so that conversion has two uses, combine cannot fold it into a store, and
 * the function carries two instructions the ROM does not have. The parameters must
 * still be prototyped as s16 — that is what produces the narrowing of the arguments
 * at the call site. */
extern s32 DivideQ8(s16 num1, s16 num2);
/* ReadUnalignedS16 (defined below in this file) was widened the same way and for the
 * same reason. Its DEFINITION returns s32 and that IS load-bearing: narrow it back to
 * `s16 ReadUnalignedS16(u8 *ptr)` and `make compare` FAILS. The consumer is
 * StreamCmd_InitBg2Zoom, and only that one — build both spellings and diff
 * `arm-none-eabi-objdump -d build/src/gfx.o`: the entire delta is inside
 * StreamCmd_InitBg2Zoom, which is the only caller that assigns the result to an s32
 * local (every other consumer takes it into an s16, where the return width is
 * invisible). With an s16 return agbcc emits the re-narrowing
 * `lsls #16 / asrs #16` in the caller and the rest of the body reassociates around it,
 * so the function stops matching. Unlike DivideQ8 this pair cannot drift silently: a
 * conflicting declaration makes agbcc exit 1 and `make` stop. */
void m4aSoundVSyncOff(void);
void m4aMPlayAllStop(void);
void m4aSoundVSyncOn(void);
void m4aSongNumStart(u16 n);
void UpdateSceneTransition(void);
void UpdateBGTileAnimation(void);
extern void LoadBGPalette();
extern void VBlankCallback_TitleScreen();
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
/**
 * UpdateSceneTransition: per-frame driver of a fixed 0x140-frame fade sequence
 * that ends by handing the screen to the title-screen callbacks.
 *
 * It is a generic driver, not one destination's transition: four sites install
 * it in a callback slot -- FadeOutController (the boot path; AgbMain installs
 * [ReadKeyInput, FadeOutController, VBlankCallback_Gameplay]),
 * ProcessSceneTransitionOut, RunSceneScript and TransitionToSceneSelect. That
 * is every literal-pool reference to it in the ROM; the scan is in the proof
 * script. Only the boot instance was OBSERVED running: there the sequence
 * presents the Namco logo for ~5.3 s and hands over to the KLONOA title screen.
 * What the other three installers leave on screen is NOT verified.
 *
 * It runs off gUnk_03004C20.sceneFrameCounter (gControlBlock[0], 0x03004C20),
 * which the VBlank callback increments once per frame. The installers do NOT
 * reset it to 0: FadeOutController and TransitionToSceneSelect both store -1,
 * and the next VBlank increment makes that 0 -- which is also how this function
 * ends the sequence.
 *
 *   counter == 0            : run one UpdateBGTileAnimation() step.
 *   0x11 <= counter <= 0x2F : fade IN -- gUnk_03005498 = (0x30 - counter) >> 1,
 *                             i.e. 15 down to 0 (0 = no darkening).
 *   counter == 0x20         : re-arm the VBlank interrupt (REG_IE bit 0 =
 *                             INTR_FLAG_VBLANK, REG_DISPSTAT bit 3 =
 *                             DISPSTAT_VBLANK_INTR), restart the m4a VSync hook
 *                             and kick song 0x21 -- the music starts 32 frames
 *                             in, while the picture is still fading up. All
 *                             three were observed firing at counter 0x20 and
 *                             nowhere else.
 *   0x101 <= c <= 0x13F     : fade OUT -- gUnk_03005498 = (counter - 0x100) >> 2,
 *                             i.e. 0 up to 15 over 63 frames.
 *   counter > 0x13F         : done -- stop the counter (-1), install
 *                             LoadBGPalette and VBlankCallback_TitleScreen into
 *                             callback slots 1 and 2, clear gUnk_03004D9C, park
 *                             the blend level at 0x10 (fully black) and set
 *                             REG_BLDCNT = BLDCNT_TGT1_ALL |
 *                             BLDCNT_EFFECT_DARKEN (0x3F | 0xC0 = 0xFF, the
 *                             literal this used to be spelled as).
 *
 * gUnk_03005498 is the BLDY darken level, and is the same cell as code_1.c's
 * gBlendValue and game.h's gFrameCounter -- three names for 0x03005498, of
 * which only the blend reading survives measurement. The VBlank callback copies
 * it into REG_BLDY every frame; forcing it to 15 through the hold window puts
 * 15 in REG_BLDY and blacks the picture out (mean luminance 6.5 -> 0.4) while
 * the untouched control stays bright, and forcing it to 0 through the fade-out
 * window keeps the picture bright where the control fades to black. Evidence:
 * docs/dynamic-analysis/scripts/prove-scene-transition-fade.mjs.
 *
 * In THIS function the two spellings ARE interchangeable: replacing all three
 * gUnk_03005498 in the body with an `extern u8 gBlendValue` leaves the whole ROM
 * byte-exact (`make compare` still prints `klonoa-eod.gba: OK`) -- measured, not
 * assumed. The macro spelling is kept only because it is what the function was
 * written with. That is not a general licence, though: re-run `make compare`
 * before changing the spelling of a global anywhere, because sometimes a macro
 * over a literal address and a declared symbol are NOT interchangeable to agbcc.
 * The counter-examples are the gLevelStatePtr / gGfxBufferPtr and
 * gGfxStreamBuffer / gGfxStreamAllocs notes in include/gfx.h, where picking the
 * wrong one of the pair costs the match.
 *
 * The two callback stores go through gCallbackStateArray, which is the SAME
 * address as gCallbackQueue (both 0x03003510), so [1] and [2] are
 * gCallbackQueue.current[1] / current[2] -- the LIVE list, not the next[] half
 * (+0x28) that the neighbouring transitions in src/code_1.c write.
 *
 * The counter is *not* cached in a local: the three trailing tests each spell
 * `sceneCtrl[0]`, which agbcc's CSE collapses to one load plus a register copy
 * (`ldr r3` / `adds r2, r3, #0`). Hoisting it into a `u32 t` local deletes that
 * second copy (two `adds rX, rY, #0` become one) and moves the 0xFFFFFF00 pool
 * constant out of r5 into r2, which is the whole difference.
 *
 * Two things the decompiling commit (6ecdbdb) said about this lever do NOT
 * reproduce when the function is built in its real translation unit, and the
 * likely reason is that the commit measured a STANDALONE candidate: the prologue
 * does not collapse to `push {r4, lr}` -- both spellings emit `push {r4, r5, lr}`
 * (0xB530), which is three registers, not the "five-register push" claimed -- and
 * the constant does not land in r3. Only the register the constant occupies
 * differs, r5 versus r2, and `ldr rX, =0xFFFFFF00` is emitted either way. See the
 * translation-unit coupling note above EntityPickupCollect in src/code_1.c for why
 * a standalone measurement of an agbcc function can disagree with the same
 * function inside its .c.
 *
 * (The second read is spelled `vu32`; that is NOT load-bearing -- replacing it
 * with a plain `sceneCtrl[0]` leaves the whole ROM byte-exact. An earlier version of
 * this comment claimed it prevented a CSE on the non-call path; no such CSE occurs.)
 */
void UpdateSceneTransition(void) {
    u32 *sceneCtrl = (u32 *)gControlBlock;

    if (sceneCtrl[0] == 0)
        UpdateBGTileAnimation();

    if (*(vu32 *)sceneCtrl == 0x20) {
        REG_IE |= INTR_FLAG_VBLANK;
        REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
        m4aSoundVSyncOn();
        m4aSongNumStart(0x21);
    }

    if ((sceneCtrl[0] >= 0x11) && (sceneCtrl[0] <= 0x2F))
        gUnk_03005498 = (0x30 - sceneCtrl[0]) >> 1;

    if ((sceneCtrl[0] >= 0x101) && (sceneCtrl[0] <= 0x13F))
        gUnk_03005498 = (sceneCtrl[0] - 0x100) >> 2;

    if (sceneCtrl[0] > 0x13F) {
        sceneCtrl[0] = (u32)-1;
        gCallbackStateArray[1] = (u32)LoadBGPalette;
        gCallbackStateArray[2] = (u32)VBlankCallback_TitleScreen;
        gUnk_03004D9C = 0;
        gUnk_03005498 = 0x10;
        REG_BLDCNT = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_DARKEN;
    }
}
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
 * ReadUnalignedS16: reads two bytes from a potentially unaligned address, assembles
 * them in little-endian order, sign-extends the result as a 16-bit quantity and
 * returns it WIDENED TO s32.
 *
 * The s32 return type is deliberate and load-bearing -- do not "correct" it to s16 to
 * match the name. Narrowing it fails `make compare`; see the note beside the DivideQ8
 * declaration at the top of this file for the measurement and the single consumer
 * (StreamCmd_InitBg2Zoom) that depends on it.
 */
s32 ReadUnalignedS16(u8 *ptr) {
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
/**
 * SetupLevelLayerConfig: configure one BG layer of a scene.
 *
 * `sceneIdx`/`layerIdx` select a slot in gBgLayerLookup, whose two bytes are the
 * sub-table row and the BG layer number (2 or 3). Every per-layer table is then
 * read at [row][layer - 2]: charblock and screenblock become the layer's VRAM
 * tile and tilemap destinations, the three u16 tables give its tilemap width,
 * height and DMA tile count, and the colour-depth byte decides whether the
 * screen runs in text or affine mode. Scroll and flags are reset, the video mode
 * is written into REG_DISPCNT's mode field, CalcBGScrollMapSize turns the tilemap
 * size into a BGCNT screen-size field, and REG_BG2CNT or REG_BG3CNT is composed
 * from all of it. Called per layer by DispatchLevelLayerSetup, ahead of
 * LoadBGTileData and LoadBGTilemapData.
 *
 * MATCHING. Three spellings decide this one. Each number below is a single-lever
 * ablation off the matching source (score against the ROM-disassembled target;
 * asmlift e0c1dce raw output scores 157 with the symbol map, 166 without):
 *
 *  - The seven sub-tables are declared `extern const uN [][2]` (see gfx.h and
 *    ldscript.in.txt) and indexed [row][layer - 2] EVERYWHERE, including inside
 *    the two BGxCNT arms where `layer` is a known constant. Spelling those arms
 *    `[row][0]` / `[row][1]` costs 59: it leaves one table base CSE'd into a
 *    callee-saved register across the CalcBGScrollMapSize call and the whole
 *    register file shifts (167 instructions become 170). It does NOT add a
 *    long-lived value, though an earlier version of this note said so: build both
 *    and the prologue and the epilogue are the same instructions -- the same
 *    `push {r4, r5, r6, r7, lr}` plus `mov r7, sl / r6, r9 / r5, r8`, and the same
 *    `pop {r3, r4, r5} / mov r8, r3 / mov r9, r4 / mov sl, r5 / pop {r4, r5, r6,
 *    r7} / pop {r0} / bx r0` -- so the set of callee-saved registers is identical.
 *    Casting all seven tables to address constants instead costs 94.
 *
 *  - The affine flag is assigned straight from the comparison rather than through
 *    an `if` and a local. The assignment expands its destination address first,
 *    so gLevelStatePtr is loaded before the comparison, which is what the ROM
 *    does; the `if` form loads it after and costs 51.
 *
 *  - `& 0xFFFF` on the screenbase group is load-bearing, not cosmetic. agbcc's
 *    fold reassociates `X | (S | CONST)` into `(X | CONST) | S`, which ORs
 *    0x2040 into the accumulator before the screenblock byte; the mask puts a
 *    BIT_AND node between the two BIT_IORs so the group survives as a subtree.
 *    Dropping it costs 39. A `(u16)` cast does NOT substitute -- it folds away
 *    before that pass, and its output is BYTE-IDENTICAL to dropping the mask
 *    entirely, which is a stronger statement than "does not substitute".
 *
 *    What the mask contributes is evaluation ORDER, not an instruction. An
 *    earlier version of this note ended by calling the truncation "the extra
 *    register copy the ROM has"; there is no truncation. Both spellings emit
 *    exactly 167 instructions and exactly two `adds rX, rY, #0` copies, and
 *    neither contains a `lsl/lsr #16` or a masking `and` anywhere in the group
 *    (the only `and` against a mask is the single `REG_DISPCNT & 0xFFF8`
 *    read-modify-write; the other `ands` in the function is the `bgAffine`
 *    bitfield store). Diff the two disassemblies and the entire delta is
 *    that `movs rX,#0x81 / lsls rX,#6 / adds rY,rX,#0` -- the materialisation
 *    of 0x2040, which is where the copy comes from -- moves from after the
 *    screenblock byte to before it.
 */
void SetupLevelLayerConfig(u32 sceneIdx, u32 layerIdx) {
    u32 layer = gBgLayerLookup[sceneIdx][layerIdx][1];
    u32 row = gBgLayerLookup[sceneIdx][layerIdx][0];

    gBgInfo[layer].pTiles = (void *)(VRAM + (gLayerCharBlock[row][layer - 2] << 14));
    gBgInfo[layer].pTilemap = (void *)(VRAM + (gLayerScreenBlock[row][layer - 2] << 11));
    gBgInfo[layer].hOfs = 0;
    gBgInfo[layer].vOfs = 0;
    gBgInfo[layer].hLength = gLayerMapWidth[row][layer - 2];
    gBgInfo[layer].vLength = gLayerMapHeight[row][layer - 2];
    gBgInfo[layer].unk16 = gLayerTileCount[row][layer - 2];
    gBgInfo[layer].unk18 = gLayerTileByteSize[row][layer - 2];
    gBgInfo[layer].unk14 = 0;

    ((struct GfxControlFlags *)gLevelStatePtr)->bgAffine = gLayerColorMode[row][layer - 2] == BGCNT_256COLOR;
    REG_DISPCNT = (REG_DISPCNT & 0xFFF8) | ((struct GfxControlFlags *)gLevelStatePtr)->bgAffine;
    ((struct GfxControlFlags *)gLevelStatePtr)->bgMapSize[layer - 2]
        = CalcBGScrollMapSize(((struct GfxControlFlags *)gLevelStatePtr)->bgAffine, gBgInfo[layer].hLength, gBgInfo[layer].vLength);

    switch (layer) {
        case 2:
            REG_BG2CNT = BGCNT_PRIORITY(2) | gLayerColorMode[row][layer - 2]
                | (((struct GfxControlFlags *)gLevelStatePtr)->bgMapSize[0] << 14)
                | ((BGCNT_SCREENBASE(gLayerScreenBlock[row][layer - 2]) | BGCNT_WRAP | BGCNT_MOSAIC) & 0xFFFF)
                | BGCNT_CHARBASE(gLayerCharBlock[row][layer - 2]);
            break;
        case 3:
            REG_BG3CNT = BGCNT_PRIORITY(2) | gLayerColorMode[row][layer - 2]
                | (((struct GfxControlFlags *)gLevelStatePtr)->bgMapSize[1] << 14)
                | ((BGCNT_SCREENBASE(gLayerScreenBlock[row][layer - 2]) | BGCNT_WRAP | BGCNT_MOSAIC) & 0xFFFF)
                | BGCNT_CHARBASE(gLayerCharBlock[row][layer - 2]);
            break;
    }
}
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
 * Finds the first free GfxStreamAlloc slot by walking the table from slot 0 until a
 * slot with tileCount == 0, accumulating every live slot's tileCount into the OBJ
 * tile index the new slot gets. That accumulator -- not the walk -- starts at
 * gPaletteCursorInit converted from an OBJ VRAM address to a tile id
 * ((cursor - 0x06010000) / 32), the same tiles-not-bytes convention
 * DmaSpriteToObjVram reverses.
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
extern void SetupOAMSprite(s32 arg0, u8 arg1, u16 arg2, u16 arg3, u8 arg4, u8 arg5, u8 arg6, u8 arg7, u8 arg8);
/**
 * StreamCmd_SetupSpriteGroup: spawn a whole canned group of OBJ sprites.
 *
 * Stream layout (4 bytes). Bytes [0]/[1] are the dispatcher's, not this
 * function's -- StreamCmd_RunScript (0x0804EA94) reads them:
 *   [0]    0xFF        marks a command; any other value is music data
 *   [1]    0x24        bit 5 selects the handler table at 0x08117854 and the low
 *                      nibble picks entry 4 in it, which is this function
 *   [2..3] unaligned u16 group id
 *
 * gUnk_08189F04 is a table of sprite groups, each 16 entries of 0xC bytes
 * (0xC0 per group), terminated by an entry whose unk0 halfword is 0xFFFF. It
 * runs to the next symbol, gUnk_0818B704: 0x1800 bytes = 32 group slots, of
 * which only ids 0..19 carry data (4..13 live rows each). Ids 20..31 are all
 * zeros -- every one of the 0x900 bytes from 0x0818AE04 to 0x0818B704 reads back
 * 0x00 out of baserom.gba -- so they hold no terminator either, and this loop has
 * no bound, so such an id runs on past its group: measured at runtime, 290
 * iterations, the u8 cursor wraps (13 -> 47) and descriptor slots are written far
 * beyond the table.
 *
 * Every live entry becomes one SetupOAMSprite call at gUnk_03005428++ -- the same
 * running slot cursor RenderCharacterTiles uses (src/code_0.c, which resets it to
 * 0xD and then spawns with `SetupOAMSprite(gUnk_03005428++, ...)`), so a group
 * appends to whatever the room already spawned rather than replacing it. Each row
 * lands in the descriptor as unk0 -> xPosScreen, unk2 -> yPosScreen, unk4 -> unk8,
 * unk6 -> unkF, unk7 -> unkA, unk8 -> kind. Every kind in the table is 0x4A..0x50,
 * and SetupOAMSprite splits on `(u8)(kind - 1) <= 0x48` at 0x08003E10: kinds
 * 1..0x49 take the arm that treats the pair as BG2/world coordinates (stored to
 * xPosBg2/yPosBg2, with xPosScreen/yPosScreen derived by subtracting the halfword
 * pair at gBgInfo+0x40/+0x42), while kind 0 and kind >= 0x4A fall through to the arm that
 * stores the pair straight into xPosScreen/yPosScreen. So every shipped group is
 * placed in screen space. What the groups DEPICT is not established, so nothing
 * here names them.
 *
 * It writes NEITHER hardware OAM (0x07000000) NOR the OAM shadow buffer gOamBuffer
 * (0x03004800): it fills sprite DESCRIPTORS in gUnk_03002920, and a later pass
 * turns descriptors into OAM entries. Proven at runtime by injecting the command
 * into the EWRAM stream and comparing against a do-nothing control and an
 * opcode-nibble control:
 * docs/dynamic-analysis/scripts/prove-streamcmd-spritegroup.mjs, which records
 * every write made from inside this call tree across all of IWRAM (gOamBuffer
 * is inside the watched range and never appears) and shows OAM being written
 * only by DMA3 out of the VBlank handler.
 *
 * agbcc matching notes:
 *   - gUnk_08189F04 must be the named extern ARRAY indexed as
 *     gUnk_08189F04[group][i] in every operand, never a `struct Unk_08189F04 *`
 *     local: the original recomputes `group * 0xC0` inside the loop and keeps
 *     only the scaled `i * 0xC` in a register, which is what indexing the array
 *     directly produces.
 *   - `group` is a u16, not a u32 -- the lsl/lsr #0x10 pair after the
 *     ReadUnalignedU16 call is the truncation, and it also gives the loop the
 *     `group * 2` it CSEs out of the guard.
 *   - the loop is a `for` with the test at the top; agbcc rotates it itself.
 *     Hand-rotating into `if (...) do { ... } while (...)` is the same control
 *     flow but lets agbcc strength-reduce the whole address into one induction
 *     pointer in r5, collapsing the four high-register live values the ROM keeps
 *     (r7 = group*2, r9 = group, r10 = base, r8 = i*0xC). The commit that matched
 *     this function (585501c) recorded the `for` form as scoring 1, "the known
 *     alignment-halfword floor". There is no residual: that measurement predates
 *     the target-generation fixes made later on this same branch (6be0678,
 *     3e6d6c5), and with `make expected` producing gfx as MATCH the function is
 *     byte-exact against the ROM. The 40-point gap over the hand-rotated form
 *     stands; the leftover 1 does not.
 */
void StreamCmd_SetupSpriteGroup(void) {
    u16 group;
    s32 i;

    group = ReadUnalignedU16(gStreamPtr + 2);
    for (i = 0; gUnk_08189F04[group][i].unk0 != 0xFFFF; i++) {
        SetupOAMSprite(gUnk_03005428++, gUnk_08189F04[group][i].unk7, gUnk_08189F04[group][i].unk0, gUnk_08189F04[group][i].unk2,
                       gUnk_08189F04[group][i].unk4, gUnk_08189F04[group][i].unk9, gUnk_08189F04[group][i].unk5,
                       gUnk_08189F04[group][i].unk6, gUnk_08189F04[group][i].unk8);
    }
    gStreamPtr += 4;
}
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
 * WriteStreamValue_Dual: set a uniform BG2 zoom.
 *
 * Reads an unaligned u16 from stream bytes [2..3] and writes it to BOTH BG2
 * magnifications -- gBg2YMag (0x03005420) and gBg2XMag (0x030034AC) -- then advances
 * the stream by 4. Opcode `FF 34`.
 *
 * Neither destination is a palette or colour register, and neither is a mirror of the
 * other: they are the two axes of the BG2 affine scale, and they drive different
 * register pairs (gBg2XMag -> REG_BG2PA/PB, gBg2YMag -> REG_BG2PC/PD), which is
 * measured one axis at a time against a control in
 * docs/dynamic-analysis/scripts/prove-bg2-magnification.mjs. An earlier version of
 * this comment called 0x03005420 "the palette/color register" and 0x030034AC "the
 * mirror"; both halves were wrong.
 *
 * The magnifications are Q_8_8 with 0x100 = 1x, and larger means smaller on screen
 * (the hardware matrix maps screen space back to texture space). Five uses in the
 * shipped scripts, from scan-gfx-stream-commands.mjs: two of them arm the starting
 * zoom immediately before an `FF 46` StreamCmd_InitBg2Zoom tween (0x250 then eased
 * down by 224 over 60 frames; 0x200 then walked back towards 0x100 over 15), and the
 * other three set a static zoom with no tween after it -- 0x100 twice, i.e. reset to
 * 1x, and 0x130.
 *
 *   no parameters (reads from the global data stream pointer at 0x03004D84)
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
 * CalcSineVelocity: compute one frame of a GfxStreamEntry's sine oscillation.
 *
 * It was called CalcSinCosVelocity until round 4. There is no cosine: both halves
 * index the same table at 0x080D8E14 with the same argument and no quarter-turn
 * offset, differing only in which amplitude they scale by (+0x08 for X, +0x0A for Y).
 *
 * Writes the X and Y velocity for this tick into out[0]/out[1]:
 * amplitude * gSineTable[(timer * angularStep) & 0xFF] >> 8, with the X amplitude
 * at +0x08 and the Y amplitude at +0x0A. Returns 1 once the entry's timer has run
 * out (timer <= 0), which tells ProcessMotionStep to stop the entry.
 */
u32 CalcSineVelocity(struct GfxStreamEntry *entry, s16 *out) {
    out[0] = ((s16)entry->unk_08 * SIN(((s16)entry->timer * entry->unk_1E) & 0xFF)) >> 8;
    out[1] = ((s16)entry->unk_0A * SIN(((s16)entry->timer * entry->unk_1E) & 0xFF)) >> 8;

    if ((s16)entry->timer <= 0)
        return 1;
    return 0;
}
/* Stub_0804CAC4: an empty function — 2 bytes of `bx lr` plus 2 bytes of alignment padding — sitting
 * between CalcSineVelocity and ProcessMotionStep. Nothing in the ROM references it, which is why
 * it has no symbol and why luvdis folded its bytes into the tail of CalcSineVelocity.s. It is a
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
u32 ProcessMotionStepExtended(u32 idx);
extern s16 DivideQ4(s16 num1, s16 num2);
/**
 * StreamCmd_InitWindowCornerMotion: tween one corner of one hardware window.
 *
 * The animated twin of StreamCmd_SetWindowCorner, which sets the same corner
 * instantly and uses the identical selector encoding. This handler turns a pixel
 * displacement and a frame count into a per-frame velocity and installs
 * ProcessMotionStepExtended with param = 4 — the arm of that callback's jump table at
 * 0x0804CCB0 that advances the level's window-clip bounds through gLevelStatePtr.
 *
 * Stream layout (10 bytes; opcode `FF 44`, i.e. StreamCmd_RunScript's `c & 0x40`
 * table at 0x0811787C, slot 4):
 *   [2]      GfxStreamEntry index
 *   [3]      low nibble -> targetIndex, which corner is driven:
 *              bit 1 clear -> rightBottom, set -> leftTop
 *              bit 0       -> window 0 or window 1
 *            so 0 = win0 right/bottom, 1 = win1 right/bottom, 2 = win0 left/top,
 *            3 = win1 left/top. All four appear in the shipped scripts.
 *   [4..5]   unaligned s16 dX in PIXELS -> unk_04 = dX << 4 (the 12.4 fixed-point
 *            form the bounds are kept in, the same << 4 StreamCmd_SetWindowCorner uses)
 *   [6..7]   unaligned s16 dY in pixels -> unk_06 = dY << 4
 *   [8..9]   unaligned s16 duration in frames
 *
 * unk_08/unk_0A then get DivideQ4(unk_04, duration << 4) and DivideQ4(unk_06,
 * duration << 4) — the per-frame step in 1/16 px, so the edge travels dX/dY pixels
 * over `duration` frames (measured: dX = 40 px over 20 frames gives unk_04 = 640 and
 * a step of 32 = 2 px/frame, and the bound lands on exactly +640). The accumulators
 * at +0x0C/+0x0E are cleared, the entry type nibble is set to 1 (active), and the
 * stream advances by 10.
 *
 * dX drives the HORIZONTAL component of the chosen corner and dY the VERTICAL one, so
 * dX reaches REG_WIN0H or REG_WIN1H and dY reaches REG_WIN0V or REG_WIN1V
 * (include/io_reg.h: 0x04000040, 0x04000042, 0x04000044, 0x04000046). Proven at
 * runtime, one corner and one axis per trial, each against a dX = dY = 0 control that
 * moves no register at all. The selector POLARITY is proven by the same trials, not
 * assumed: targets 0 and 1 move rightBottom[win][axis] and targets 2 and 3 move
 * leftTop[win][axis], so bit 1 clear is rightBottom and bit 1 set is leftTop.
 *
 * The old name (StreamCmd_InitMotionWithPalette) was wrong: the command touches NO
 * palette RAM and no blend or mosaic register. Ten trials changed 0 bytes of palette
 * RAM and left REG_BLDCNT, REG_BLDALPHA, REG_BLDY, REG_MOSAIC, REG_WININ and
 * REG_WINOUT bit-identical.
 *
 * The scripts use it two ways. Sometimes in genuine pairs — one edge of a window and
 * the OPPOSITE edge of the same window, moving towards or away from each other, which
 * wipes that window shut or open: `target 3 dX +40` with `target 1 dX -40` over 40
 * frames (script[0] @0x0e46/@0x0e50) walks window 1's left edge right and its right
 * edge left, closing it by 40 px a side, and `target 2 dX -80` with `target 0 dX +80`
 * over 40 frames (script[1] @0x1f43/@0x1f4d) is the same shape on window 0, opening
 * it. Sometimes it is ONE corner driven out and then back, which is not a pair at all
 * even though the two commands look symmetric: `target 2 dY -96` then `target 2 dY
 * +96` over 48 frames each (script[1] @0x13d3/@0x13ea) moves window 0's top edge 96 px
 * and returns it, and one edge cannot letterbox. Both spellings are common; read the
 * target byte before assuming which one you are looking at. All 62 uses are listed by
 * docs/dynamic-analysis/scripts/scan-gfx-stream-commands.mjs.
 *
 * Note: the handler is also reachable as `FF 2E` through the 0x08117854 table, which
 * holds the same function pointer. No shipped script uses that spelling.
 *
 * Same shape as the other StreamCmd_Init* handlers: one `cmd`/`entries` pair per
 * short run of stores, because the original re-reads gStreamPtr and gBuffer_52A4
 * instead of caching them. The two paired stores (`unk_0C = unk_0E = 0`) are one
 * chained assignment: written as two statements agbcc recomputes the entry address,
 * which the ROM does not. The divisions read unk_04/unk_06 back out of the entry
 * rather than using the local, so the `<< 4` truncation to u16 is observable.
 *
 * Evidence: docs/dynamic-analysis/scripts/prove-window-corner-tween.mjs (the
 * UpdateAffineRegisters register-pointer walk labelled from include/io_reg.h by
 * address, then one causal trial per corner and axis with dX = dY = 0 controls) and
 * docs/dynamic-analysis/scripts/scan-gfx-stream-commands.mjs (all 62 shipped uses).
 */
void StreamCmd_InitWindowCornerMotion(void) {
    s16 dx;
    s16 dy;
    s16 duration;
    s16 step;
    s16 vx;
    s16 vy;

    dx = ReadUnalignedS16(gStreamPtr + 4);
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_04 = dx << 4;
        dy = ReadUnalignedS16(cmd + 6);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_06 = dy << 4;
        duration = ReadUnalignedS16(cmd + 8);
    }
    step = duration << 4;
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        vx = DivideQ4((s16)entries[idx].unk_04, step);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_08 = vx;
        vy = DivideQ4((s16)entries[cmd[2]].unk_06, step);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_0A = vy;
        entries[cmd[2]].unk_0C = entries[cmd[2]].unk_0E = 0;
        entries[cmd[2]].param = 4;
        entries[cmd[2]].callback = (u32)ProcessMotionStepExtended;
        entries[cmd[2]].type = 1;
    }
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
    gStreamPtr += 10;
}
/**
 * StreamCmd_InitBg2Zoom: start a uniform BG2 magnification tween.
 *
 * Installs ProcessMotionStepExtended on the GfxStreamEntry named by stream byte[2]
 * with param = 1 — the arm of that callback's jump table at 0x0804CCB0 that steps
 * gBg2XMag (0x030034AC) and gBg2YMag (0x03005420). Both axes are written from the
 * SAME delta and stepped by the SAME per-frame amount.
 *
 * That makes it a symmetric tween, not a guarantee of a square scale: the callback
 * ADDS the per-frame step to each magnification rather than assigning it, so a
 * non-square starting state stays non-square by exactly the same margin all the way
 * through. What the command cannot do is INTRODUCE anisotropy from a square start.
 * In practice the question does not arise, because both shipped uses are preceded by
 * a `FF 34` (WriteStreamValue_Dual) that writes one value to both magnifications and
 * so squares them first.
 *
 * Stream layout (7 bytes; opcode `FF 46`, i.e. StreamCmd_RunScript's `c & 0x40` table
 * at 0x0811787C, slot 6 — this handler appears in no other dispatch table):
 *   [2]      GfxStreamEntry index
 *   [3..4]   unaligned s16 total delta, stored in BOTH unk_04 (X target) and unk_06
 *            (Y target)
 *   [5]      duration in frames. The per-frame step is DivideQ8(unk_06, byte[5] << 8),
 *            i.e. (total << 8) / (frames << 8) = total/frames, stored in BOTH unk_08
 *            (X step) and unk_0A (Y step).
 *   [6]      DEAD. The handler fetches [5..6] as one unaligned s16 and shifts it left
 *            by 8 for DivideQ8's second parameter, which narrows back to s16 and drops
 *            the high byte. Confirmed at runtime: sweeping byte[6] over 00/01/7F/AB/FF
 *            leaves the installed step at -17 in all five runs.
 *
 * unk_0C/unk_0E (the accumulators UpdateLinearInterpolation compares against
 * unk_04/unk_06) are cleared, the entry type nibble is set to 1 (active), and the
 * stream advances by 7.
 *
 * gBg2XMag/gBg2YMag are magnifications with 0x100 = 1x. The VBlank handlers turn each
 * into its reciprocal and write the BG2 affine matrix: gBg2XMag reaches REG_BG2PA and
 * REG_BG2PB, gBg2YMag reaches REG_BG2PC and REG_BG2PD (include/io_reg.h,
 * 0x04000020..0x04000026). Verified at runtime one axis at a time against a no-write
 * control: doubling gBg2XMag halved REG_BG2PA (0x00FF -> 0x007F) and REG_BG2PB and
 * left REG_BG2PC/PD untouched; doubling gBg2YMag did the mirror image.
 *
 * The two uses in the shipped scripts are both zoom-outs: `FF 34` sets
 * gBg2XMag = gBg2YMag = 0x200, then `FF 46 02 00 FF 0F 00` walks both back to 0x101
 * over 15 frames (one short of 0x100, because the per-frame step truncates:
 * -17 x 15 = -255). The other sets 0x250 and eases down by 224 over 60 frames.
 *
 * The old name (StreamCmd_InitAngleMotion) was wrong: the command never touches
 * gBg2Alpha (0x03002910), and the value it moves is a linear magnification delta.
 *
 * Note the step is recomputed from the entry's own unk_06 rather than from the local
 * copy of the stream value — the handler stores the total, then reads it back as a
 * signed halfword (the `ldsh` in the ROM) to divide it.
 *
 * Evidence: docs/dynamic-analysis/scripts/prove-bg2-magnification.mjs (register
 * isolation with a control, the shipped commands executed on the real CPU, the
 * param=0 control, and the byte[6] sweep) and
 * docs/dynamic-analysis/scripts/scan-gfx-stream-commands.mjs (the two shipped uses).
 */
void StreamCmd_InitBg2Zoom(void) {
    s32 total = ReadUnalignedS16(gStreamPtr + 3);
    s32 step;

    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_06 = total;
        entries[idx].unk_04 = total;
        step = DivideQ8(entries[cmd[2]].unk_06, ReadUnalignedS16(cmd + 5) << 8);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_0A = step;
        entries[idx].unk_08 = step;
        idx = cmd[2];
        entries[idx].unk_0E = 0;
        entries[idx].unk_0C = 0;
        entries[cmd[2]].param = 1;
        entries[cmd[2]].callback = (u32)ProcessMotionStepExtended;
        entries[cmd[2]].type = 1;
    }
    gStreamPtr += 7;
}
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
/**
 * StreamCmd_InitSpriteWave: start a slow VERTICAL oscillation of one stream-owned
 * sprite.
 *
 * Fills the GfxStreamEntry selected by stream byte[2] and installs
 * ProcessSpriteOscillation as its per-frame callback, then advances the stream by 8.
 * Opcode `FF 4C`, i.e. StreamCmd_RunScript's `c & 0x40` table at 0x0811787C, slot 12;
 * this handler appears in no other dispatch table.
 *
 * Stream layout (8 bytes):
 *   [2]    entry index into gGfxStreamEntries
 *   [3]    objIndex (7 bits). ProcessSpriteOscillation biases it by +13 and uses it to
 *          index gEntityArray (0x03002920, struct Unk_03002920, stride 0x1C). The bias
 *          is the stream-owned entity window: slots 0..12 belong to the engine
 *          (TransformSingleEntityToScreen switches sprite-template tables at
 *          arg0 > 0xC), and every gfx-stream command that names a sprite adds 13 —
 *          StreamCmd_SetEntityTransform spells the same thing as
 *          `gUnk_03002920[gStreamPtr[2] + 0xD]`.
 *   [4] hi nibble -> unk_1F: which 16-byte row of the table at 0x081177F4 to play.
 *   [4] lo nibble -> unk_1A: right shift applied to the frame counter before it
 *          indexes the row, i.e. the period (bigger = slower).
 *   [5]    -> unk_1C: amplitude each signed sample is multiplied by.
 *   [6..7] -> timer (unaligned s16): how many frames the wave runs; -1 = forever.
 *
 * It also zeroes unk_0E (the frame counter ProcessSpriteOscillation increments and
 * compares against timer), sets the entry type nibble to 1 (active), and latches
 * `timer != -1` into unk_1E. The callback only retires once the counter reaches timer
 * AND unk_1E is set, so -1 really does mean "run forever" — verified: with -1 the
 * callback still returned 1 after 140 ticks; with 20 it returned 0 on tick 20.
 *
 * The 0x081177F4 table is 4 rows x 16 signed bytes of per-frame VELOCITY, not
 * position: each frame the callback does
 *     entity.yPosBg2   += unk_1C * (s8)row[(counter >> unk_1A) & 0xF];
 *     entity.yPosScreen = entity.yPosBg2 >> 4;
 * so the shape on screen is the row's running sum. Only Y moves; X is never touched.
 *   row 0  velocity  1, 1, 2,-2,-1,-1, 0, 0  (twice)  -> position 1,2,4,2,1,0,0,0
 *   row 1  velocity -1,-1,-2,-2,-1,-1,0,0,1,1,2,2,1,1,0,0 -> a one-shot dip to -8
 *   rows 2 and 3 are all zeroes, so selecting them is a no-op.
 * Reading row 0 as a jitter is a mistake: integrated, it is a smooth hump.
 *
 * All five uses in the shipped scripts select row 0, amplitude 2, duration -1, with a
 * shift of 3 (four of them) or 2 (one), on entity slots 14, 16 and 18. Measured on the
 * real callback: shift 3 gives a 4-pixel rise and fall with a 64-frame period
 * (~1.07 s), peaking at ticks 24 and 88 and flat at 48..64 and 112..128. That is a
 * float or bob, not a shake. Rows 1..3 are dead data.
 *
 * Evidence: docs/dynamic-analysis/scripts/prove-sprite-wave.mjs (the table and its
 * integral read out of the cartridge, the shipped command executed on the real CPU and
 * ticked 140 times, amplitude-0 and row-2/row-3 controls that produce no motion, and an
 * objIndex sweep showing only slot objIndex+13 ever moves) and
 * docs/dynamic-analysis/scripts/scan-gfx-stream-commands.mjs (all five shipped uses).
 */
void StreamCmd_InitSpriteWave(void) {
    s16 duration;

    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;
        struct GfxStreamEntry *entry;
        u8 target;

        entry = (struct GfxStreamEntry *)(idx * sizeof(struct GfxStreamEntry) + (u32)entries);
        target = cmd[3];
        entry->objIndex = target;
        entries[cmd[2]].unk_1F = cmd[4] >> 4;
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_1A = cmd[4] & 0x0F;
        entries[cmd[2]].unk_1C = cmd[5];
        duration = ReadUnalignedS16(cmd + 6);
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].timer = duration;
        entries[cmd[2]].callback = (u32)ProcessSpriteOscillation;
        entries[cmd[2]].unk_0E = 0;
        entries[cmd[2]].type = 1;
    }
    {
        u8 *cmd = gStreamPtr;
        u8 idx = cmd[2];
        struct GfxStreamEntry *entries = gGfxStreamEntries;

        entries[idx].unk_1E = ~*(s16 *)&entries[idx].timer != 0;
    }
    gStreamPtr += 8;
}
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
/**
 * StreamCmd_ConfigureBlend: arms the MUSIC volume ramp run by UpdatePaletteFadeStep.
 *
 * Not a video blend, despite both function names: UpdatePaletteFadeStep writes no
 * I/O register at all — its whole literal pool is the four gMPlayInfo_* players and
 * gSoundVolume, and it calls m4aMPlayVolumeControl five times.
 *
 * Stream layout (5 bytes): byte[2] low 2 bits select which m4a player the ramp
 * drives (GfxControlFlags.soundFadeMPlaySel), and bytes[3..4] are an unaligned
 * halfword whose low 9 bits are the target volume (GfxControlFlags.soundFadeTarget)
 * and whose bit 15 is the ramp direction, copied into soundFadeRampUp before the
 * value is masked down. soundFadeActive is then set to start the ramp;
 * UpdatePaletteFadeStep clears both again when the running value at 0x18 reaches
 * the target.
 *
 * A cast of gLevelStatePtr rather than the gGfxBufferPtr macro: with the macro
 * spelling this function does not match and `make compare` fails. Why agbcc treats
 * the two differently is not established. Both spell 0x030034A0; see include/gfx.h.
 */
void StreamCmd_ConfigureBlend(void) {
    ((struct GfxControlFlags *)gLevelStatePtr)->soundFadeSel = gStreamPtr[2];
    ((struct GfxControlFlags *)gLevelStatePtr)->soundFadeTarget = ReadUnalignedU16(gStreamPtr + 3);
    /* (s16) so agbcc emits the LDRSH the ROM has; the field itself is unsigned. */
    if ((s16)((struct GfxControlFlags *)gLevelStatePtr)->soundFadeTarget & 0x8000) {
        ((struct GfxControlFlags *)gLevelStatePtr)->soundFadeRampUp = 1;
    }
    ((struct GfxControlFlags *)gLevelStatePtr)->soundFadeTarget &= 0x1FF;
    ((struct GfxControlFlags *)gLevelStatePtr)->soundFadeActive = 1;
    gStreamPtr += 5;
}
/**
 * StreamCmd_ToggleVBlankHandler: swaps the VBlank callback between the minimal
 * handler and the window-scroll handler, tracking which one is installed in
 * GfxControlFlags bit 0x1C.4, and advances the stream by 2.
 *
 * This function had no thumb_func_start of its own: luvdis ran it into the tail
 * of StreamCmd_ConfigureBlend (see the ROM address in the .s that used to be
 * INCLUDE_ASM'd here), so it is decompiled together with its parent.
 *
 * The flag records which handler is installed, so the arm that tests true is the
 * one that switches back to VBlankHandlerMinimal.
 *
 * Note that the pool words are 0x08000AB1 and 0x08000E69, i.e. the two handlers'
 * linked addresses with the Thumb bit set -- the `@ 08000AB2` / `@ 08000E6A`
 * comments in their .s files are two too high (`non_word_aligned_thumb_func_start`
 * emits no padding, and `nm klonoa-eod.elf` puts both symbols two bytes lower).
 */
void VBlankHandlerMinimal(void);
void StreamCmd_ToggleVBlankHandler(void) {
    if (((struct GfxControlFlags *)gLevelStatePtr)->flag_1C_4) {
        gVBlankCallback = (u32)VBlankHandlerMinimal;
        ((struct GfxControlFlags *)gLevelStatePtr)->flag_1C_4 = 0;
    } else {
        gVBlankCallback = (u32)VBlankHandler_WithWindowScroll;
        ((struct GfxControlFlags *)gLevelStatePtr)->flag_1C_4 = 1;
    }
    gStreamPtr += 2;
}
INCLUDE_ASM("asm/nonmatchings/gfx", StreamCmd_RunScript);
