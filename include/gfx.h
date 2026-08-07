#ifndef GUARD_GFX_H
#define GUARD_GFX_H

#include "global.h"

/* ══════════════════════════════════════════════════════════════════════
 *  Struct Definitions
 * ══════════════════════════════════════════════════════════════════════ */

/* BGLayerState: per-layer BG configuration (28 bytes, >=4 entries at gBGLayerState;
 * gBgInfo[4] aliases the same memory and prove-gfxstream-motion-fields drives layer 3).
 * Controls VRAM destinations, scroll positions, tilemap dimensions, and DMA params. */
struct BGLayerState {
    u32 tileVramDest; /* +0x00: VRAM charblock address for tile DMA */
    u32 tmapVramDest; /* +0x04: VRAM screenbase address for tilemap DMA */
    u16 scrollX; /* +0x08: horizontal scroll position (subpixel, >>4 for pixels) */
    u16 scrollY; /* +0x0A: vertical scroll position (subpixel, >>4 for pixels) */
    u16 unk_0C; /* +0x0C: unknown */
    u16 unk_0E; /* +0x0E: unknown */
    u16 mapWidth; /* +0x10: tilemap width in tiles */
    u16 mapHeight; /* +0x12: tilemap height in tiles */
    u16 flags; /* +0x14: layer flags */
    u16 dmaTileCount; /* +0x16: number of tile rows for DMA */
    u8 dmaRowSize; /* +0x18: bytes per DMA row */
    u8 pad_19; /* +0x19: padding */
    u16 pad_1A; /* +0x1A: padding */
}; /* total: 0x1C = 28 bytes */

/* GfxStreamEntry: per-entry state for the graphics command stream processor
 * (36 bytes, 32 entries at gBuffer_52A4).
 * Controls animations, motion paths, and timed events driven by the stream. */
struct GfxStreamEntry {
    /* +0x00: packed 32-bit header. The motion handlers read bytes 0..3 as one
     * word, and two of the packed fields below straddle a byte boundary, so the
     * whole header is one u32 bitfield; agbcc still narrows each access to the
     * byte or halfword the original uses. Bit numbers are within the
     * little-endian word at +0x00. Proven at runtime by
     * docs/dynamic-analysis/scripts/prove-oscillation-fields.mjs (E5, E6). */
    u32 type : 3; /* bits 0-2: entry type; 1 = active, dispatch `callback` */
    /* bits 3-10: per-handler parameter — the word-level "target selector" above.
     * For the static-scroll handler it gates the effect: ProcessStaticBGScroll
     * returns early unless this is 0. Handlers read it differently, so the name
     * stays generic. */
    u32 param : 8;
    /* bits 11-14: index of the object this entry drives. Every consumer that
     * reads byte 1 (ProcessMotionStep, ProcessMotionStepExtended,
     * ProcessStaticBGScroll) extracts exactly these bits
     * and uses them only as an index: in the default target mode it selects
     * gBGLayerState[targetIndex] (gUnk_03003430, stride 0x1C) whose scrollX/scrollY
     * it advances; in the other mode bits 1 and 0 pick a window-clip edge pair in
     * gGfxBufferPtr (+0x08/+0x0C/+0x10/+0x14), which UpdateAffineRegisters pushes
     * to REG_WIN0H/WIN1H/WIN0V/WIN1V — despite its name it writes no affine
     * register at all. Observed values 0, 1, 2. Proven at runtime — see
     * prove-gfxstream-motion-fields.mjs and prove-gfxstreamentry-header.mjs
     * (sweeping it with the scroll step held constant moves a DIFFERENT layer by
     * the SAME distance). Named for the index, not "bgLayer", because the
     * window-bounds and affine readers do not treat it as a BG layer. */
    u32 targetIndex : 4;
    /* bits 15-21: which gUnk_03002920 object this entry drives, biased by +13.
     * StreamCmd_InitOscillationExt loads it straight from the command stream. */
    u32 objIndex : 7;
    /* bits 22-31: the rest of the header (the old unk_02 / unk_03 bytes). Bit 31,
     * i.e. bit 7 of the byte at +0x03, is the timer-sign latch that
     * StreamCmd_InitHBlankWait writes and ProcessHBlankWait reads. */
    u32 headerHigh : 10;
    u16 unk_04; /* +0x04: tile/frame base index */
    u16 unk_06; /* +0x06: tile count / allocation size */
    /* +0x08 and +0x0A are mode-dependent, so they keep `unk` names on purpose:
     *   ProcessMotionStep     — +0x08 is the X oscillation amplitude and +0x0A
     *                           the Y amplitude; each is multiplied by
     *                           gSineTable[angle] and shifted right by 8 (E2, E3).
     *   ProcessFrameAnimation — +0x08 is instead the reload value copied into
     *                           `timer` when the countdown expires. */
    u16 unk_08; /* +0x08: counter / position A */
    u16 unk_0A; /* +0x0A: counter / position B */
    u16 unk_0C; /* +0x0C: frame index */
    u16 unk_0E; /* +0x0E: unknown */
    u16 unk_10; /* +0x10: unknown */
    u16 unk_12; /* +0x12: unknown */
    /* +0x14: per-entry frame counter, read as s16. ProcessMotionStep,
     * ProcessFrameAnimation, ProcessHBlankWait and ProcessButtonWait all decrement
     * it once per dispatch and finish on the tick it goes negative.
     * ProcessSpriteOscillation is the exception: it compares it against unk_0E as a
     * limit and never decrements it. ProcessMotionStep also uses it as the
     * oscillation phase: angle = (timer * unk_1E) & 0xFF. StreamCmd_InitHBlankWait
     * latches its sign into bit 7 of unk_03. Runtime: prove-oscillation-fields.mjs
     * (E4) and prove-button-wait.mjs (3a: a wait ticks 4->3->2->1->0->-1). */
    u16 timer;
    u16 unk_16; /* +0x16: unknown */
    u16 unk_18; /* +0x18: unknown */
    u16 unk_1A; /* +0x1A: unknown */
    u16 unk_1C; /* +0x1C: unknown */
    /* +0x1E: per-entry-type payload — deliberately NOT renamed, the meaning is not
     * shared across types. ProcessMotionStep reads it as the per-frame angular step
     * (E1) and ProcessFrameAnimation as an animation frame base (StreamCmd_InitFrameAnimation
     * stores a frame index here, the same stream byte it writes to unk_0C). For a
     * button-wait entry it is instead a boolean "ignore the timer, wait for the button
     * however long it takes" flag, which StreamCmd_InitButtonWait sets from the sign of
     * the s16 timeout (a negative timeout => wait indefinitely). Proven by
     * prove-button-wait.mjs 3c/3d: with the flag set an already-expired timer keeps
     * ticking negative and the entry survives; with it clear and everything else
     * identical the entry deactivates on the tick the timer goes negative. */
    u8 unk_1E;
    u8 unk_1F; /* +0x1F: frame/animation param */
    u32 callback; /* +0x20: function pointer for per-frame update */
}; /* total: 0x24 = 36 bytes */

/* ProcessButtonWait (0x0804D074 thumb, so 0x0804D075 as a callback value) — the
 * per-frame handler StreamCmd_InitButtonWait installs into GfxStreamEntry.callback.
 *
 * Returns 1 to keep waiting, 0 to finish; ProcessAnimationSteps masks the result
 * with 7 and stores it into status's low 3 bits, so returning 0 deactivates the
 * entry. It ends the wait when gNewKeys bit 0 (a fresh A press) is set, otherwise
 * it decrements `timer` and ends when that goes negative — unless unk_1E is set,
 * in which case the timeout is ignored and only A ends it.
 *
 * The address is an ldscript symbol rather than a C function because luvdis merged
 * this function into the tail of asm/nonmatchings/gfx/ProcessSpriteOscillation.s,
 * so it has no thumb_func_start of its own to link against.
 * Evidence: docs/dynamic-analysis/scripts/prove-button-wait.mjs */
u32 ProcessButtonWait(u32 idx);

/* ── Graphics Stream ── */

/* Cursor into the graphics/music command stream (0x03004D84).
 * Nearly all gfx stream command functions read/advance this pointer.
 * Declared as a real extern object (address supplied by ldscript) rather than
 * a literal-address macro — agbcc's alias analysis only distinguishes it from
 * the typed global arrays when it is a declared object, which several stream
 * commands (e.g. StreamCmd_SetEntityTransform) need in order to match. */
extern u8 *gStreamPtr;

/* Pointer to the graphics buffer control struct.
 * Dereferenced for palette state, flags, and buffer management. */
#define gGfxBufferPtr       (*(u32 *)0x030034A0)

/* Scene-exit control: bits 1..2 of the gfx buffer's byte 2.
 *
 * The gfx-stream tick (sub_0804EB64) branches on these two bits:
 *   GFX_SCENE_EXITING   -> stop advancing the stream; tear the windows down and
 *                          call ProcessSceneTransitionOut() every frame instead.
 *   GFX_SCENE_SKIPPABLE -> while set, pressing START (gNewKeys bit 3) makes the
 *                          tick clear the render mode and set GFX_SCENE_EXITING.
 *
 * Proven at runtime — see docs/dynamic-analysis/scripts/prove-gfxstream-exitrequest.mjs. */
#define GFX_SCENE_EXITING   0x02
#define GFX_SCENE_SKIPPABLE 0x04

/* Bit-flag view of the graphics buffer control struct pointed to by
 * gGfxBufferPtr. Byte 0x1C holds the scene-transition flags consumed by
 * sub_0804EB64 (the world-map screen's per-frame callback) and by the
 * ProcessSceneTransitionOut / UpdatePaletteFadeStep pair it drives. Nothing
 * outside that screen's callback chain reads this byte.
 *
 * Bits 5 and 6 are named from runtime evidence — see
 * docs/dynamic-analysis/scripts/prove-gfx-flag-1C-bit5.mjs (an A/B intervention
 * in the gba-kit headless emulator: one bit changed between otherwise identical
 * runs). Bits 0-1 are read by UpdatePaletteFadeStep and written by
 * StreamCmd_ConfigureBlend; that is static evidence, but the reader is short
 * enough to be exhaustive, so they are named. Bit 2 is written by
 * StreamCmd_ConfigureBlend and bit 4 by StreamCmd_ToggleVBlankHandler; bit 3
 * keeps a placeholder name. */
struct GfxControlFlags {
    u8 pad_00[2];
    u32 flag_02_0 : 1;
    /* 0x02 bits 1-2 — the two scene-exit bits GFX_SCENE_EXITING (0x02) and
     * GFX_SCENE_SKIPPABLE (0x04) documented above, spelled as one 2-bit field
     * because StreamCmd_ToggleDisplayFlag toggles the field as a unit with
     * `sceneExit ^= 2`, which reads and rewrites both bits as one unit. Note
     * that 2 is a value IN THE FIELD, so it toggles the field's high bit --
     * byte bit 2, GFX_SCENE_SKIPPABLE -- not GFX_SCENE_EXITING. A byte mask is
     * not a field value; the two only coincide for a field at bit offset 0.
     *
     * The `u32` container is load-bearing, not cosmetic. As `u8 : 2` agbcc
     * materialises the insert's negated mask with `mov #7 / neg`; as `u32 : 2`
     * it CSEs the mask against the register already holding the xor constant
     * and emits `mov #2 / sub #9` — which is what the ROM has. The two spell
     * the same 8-bit unit at byte 2 (both compile to ldrb/strb [x, #2]); only
     * the constant synthesis differs. The 0x1C group below is deliberately
     * left as `u8` for the same reason in reverse: it matches as u8. */
    u32 sceneExit : 2;
    u32 flag_02_3 : 5;
    u8 pad_03[0x17];
    /* 0x1A — target MUSIC volume, written by StreamCmd_ConfigureBlend from the
     * command's unaligned halfword argument. UpdatePaletteFadeStep steps the
     * running value at 0x18 toward it in units of 0x10 and passes it as the
     * `volume` argument of m4aMPlayVolumeControl; StreamCmd_SetMusicParams writes
     * the same cell and mirrors it into gSoundVolume. The command masks it with
     * 0x1FF, which fits a 0..0x100 m4a volume and is far wider than any blend
     * level. Signed: both readers load it with a register-offset `ldsh`. */
    s16 soundFadeTarget;
    /* 0x1C bits 0-1 — a two-bit field, not two flags: UpdatePaletteFadeStep
     * extracts it as a unit with `lsl #30 / lsr #30`, and StreamCmd_ConfigureBlend
     * assigns the whole field from the command byte (`and #3` is the field's own
     * truncation). It selects which m4a player the volume ramp drives: 0 ->
     * gMPlayInfo_0, 1 -> gMPlayInfo_1, 3 -> gSoundVolume and all four players.
     * 2 falls through and does nothing. Static evidence, but complete: the
     * function writes no I/O register at all and its whole literal pool is the
     * four gMPlayInfo_* and gSoundVolume. */
    u8 soundFadeMPlaySel : 2;
    u8 flag_1C_2 : 1;
    u8 flag_1C_3 : 1;
    u8 flag_1C_4 : 1;
    /* 0x1C bit 5 — direction of the scene-transition cross-fade run by
     * ProcessSceneTransitionOut. Clear: ramp gBlendValue (written to both REG_BLDALPHA's EVA and REG_BLDY; under
     * BLDCNT darken it is the BLDY level that shows) UP
     * to 16, then tear the scene down. Set: ramp it DOWN instead, and on
     * underflow clear REG_DISPCNT bit 10 (BG2 off) and clear this bit — it is
     * one-shot and self-clearing. */
    u8 blendRampDown : 1;
    /* 0x1C bit 6 — while the scene is transitioning out, sub_0804EB64 forces
     * REG_WININ = REG_WINOUT = 0x3F (every window region fully open) and re-pins
     * gBlendValue to 15 on every frame. */
    u8 forceWindowsOpen : 1;
    u8 flag_1C_7 : 1;
};

/* 0x030034A0 already has two names — gLevelStatePtr (a declared object) and the
 * gGfxBufferPtr macro — and a third was added here for the flags view before it
 * was checked whether an existing one would do. It does: casting gLevelStatePtr
 * keeps StreamCmd_ConfigureBlend byte-exact, while the macro spelling does not.
 * What agbcc needs is *a* symbol_ref, not a new symbol, so no third name. */

/* BG2 affine magnification (Q_8_8). Used as 1/scale in BG2PA/PD calculations. */
extern u16 gBg2XMag;
extern u16 gBg2YMag;

/* Decompressed data buffer pointer (allocated by LoadAndDecompress functions). */
#define gDecompBuffer    (*(void **)0x030007D0)

/* Graphics buffer freed by ShutdownGfxStream. Holds a pointer to an array of
 * 32 GfxStreamAlloc entries (0x100 bytes, allocated by InitGfxStreamState). */
#define gGfxStreamBuffer (*(u32 *)0x030007C8)

/* GfxStreamAlloc: one slot of the graphics stream's OBJ-tile allocator
 * (8 bytes, 32 entries at *gGfxStreamBuffer). LoadGfxStreamEntry fills the first
 * free slot: it decompresses a tileset onto the heap and hands the entry the next
 * run of OBJ VRAM, walking `tileIndex` forward by each live slot's `tileCount`
 * starting from gPaletteCursorInit. ResetGfxStreamEntries frees them all again.
 *
 * `tileIndex`/`tileCount` are counted in 32-byte 4bpp tiles, NOT bytes — verified
 * at runtime, see docs/dynamic-analysis/scripts/prove-gfx-stream-alloc.mjs. */
struct GfxStreamAlloc {
    u32 pTiles; /* +0x00: decompressed OBJ tile data; points 4 bytes past the heap header, so freeing passes pTiles - 4 */
    u16 tileIndex; /* +0x04: first OBJ tile this entry owns, in 32-byte tiles from 0x06010000 (an OAM attr2 tile id) */
    u16 tileCount; /* +0x06: number of 32-byte tiles owned; 0 = slot unused, and it is the stride of tileIndex */
}; /* total: 8 bytes */

/* The same cell as gGfxStreamBuffer (0x030007C8), typed as what it actually holds.
 * Both spellings are kept because they are not interchangeable to agbcc: the macro
 * is a CONST_INT address and this is a symbol_ref, and LoadGfxStreamEntry only
 * matches through the symbol_ref -- see the note on that function in src/gfx.c. */
extern struct GfxStreamAlloc *gGfxStreamAllocs;

/* GfxStreamTileset: one row of the stream's OBJ-tileset table at 0x08057954
 * (46 rows, 8 bytes each), selected by the stream command's 7-bit tileset id.
 * LoadGfxStreamEntry decompresses `pTiles` onto the heap and copies `tileCount`
 * verbatim into the GfxStreamAlloc slot it fills, so a row is the ROM-side
 * template of a slot. The halfword at +0x04 is 0 in all 46 rows and no reader
 * for it has been found, so it keeps an `unk` name instead of being called a
 * tile index by analogy with GfxStreamAlloc. */
struct GfxStreamTileset {
    const void *pTiles; /* +0x00: compressed 4bpp OBJ tile data, with the 4-byte sub-header DecompressAlloc expects */
    u16 unk_04; /* +0x04: 0 in every row; no reader found */
    u16 tileCount; /* +0x06: 32-byte OBJ tiles the slot reserves — NOT the tileset's own size:
                    * rows whose payload is a multi-frame sheet decompress to several times this */
}; /* total: 8 bytes */

extern const struct GfxStreamTileset gStreamTilesetTable[];

/* OBJ palette table at 0x08189DCC: 46 pointers to 32-byte (16-colour) OBJ
 * palettes, indexed by the same tileset id as gStreamTilesetTable. When the
 * stream command's high bit is set, LoadGfxStreamEntry DMAs one of these to
 * gVramWriteCursor and advances that cursor by 0x20. */
extern void *const gStreamPaletteTable[];

/* Buffer freed by FreeBuffer_52A4. */
#define gBuffer_52A4             (*(u32 *)0x030052A4)

/* The 32-entry GfxStreamEntry table gBuffer_52A4 points at. Written as a macro
 * over the global rather than cached in a local on purpose: the stream commands
 * only match when every statement re-reads gBuffer_52A4 and lets agbcc's CSE
 * decide which of those reloads survive. */
#define gGfxStreamEntries        ((struct GfxStreamEntry *)gBuffer_52A4)

/* 0x030007D8 is NOT a BLDY fade level -- it is the 4-bit MOSAIC size, declared as
 * `extern u8 gMosaicSize` in structs/variables.h. Proven at runtime by
 * docs/dynamic-analysis/scripts/prove-mosaic-vs-bldy.mjs: forcing 0x030007D8 to 15
 * makes VBlankCallback_Gameplay write REG_MOSAIC=0xFFFF (screen pixelates) and
 * leaves REG_BLDY untouched. The real BLDY fade level is gBlendValue (0x03005498). */

/* ── ROM Data Tables (Sprite / Display) ── */

/* Input/state dispatch table used by ProcessInputAndTimers.
 * Array of function pointers / state transition entries. */
#define ROM_STATE_DISPATCH_TABLE 0x080D9150

/* Sprite tileset sub-table used by LoadSpriteFrame.
 * Indexed by frame number within a tileset. */
#define ROM_SPRITE_SUBTABLE      0x0818B8A8

/* Sprite frame/animation data table.
 * Array of {u32 count, u32 dataPtr} pairs for sprite animations.
 * Referenced by RenderMenuUI, RenderDialogBox, RenderHUD*, etc. */
#define ROM_SPRITE_FRAME_TABLE   0x08078FC8

/* Display configuration / sprite mapping table.
 * Used by SetupDisplayConfig to select rendering modes. */
#define ROM_DISPLAY_CONFIG_TABLE 0x080D821C

/* OAM template data (initial OAM attribute values).
 * Used by InitOamEntries (sub_0800A468). */
#define ROM_OAM_TEMPLATE         0x080E2A7C

/* Graphics asset / tileset tables used by InitGraphicsSystem. */
#define ROM_GFX_ASSET_TABLE      0x0818B7AC
#define ROM_TILESET_TABLE        0x0818B8E0

/* ── Background Layer Tables ── */

/* BG tile data pointer table: 162 u32 ROM pointers to compressed tile charblock data.
 * Indexed as: (vision-1)*27 + world*3 + layer, where vision=1-6, world=0-8, layer=0-2.
 * Each entry points to a compressed asset with a 4-byte sub-header (BGCNT template)
 * followed by 4bpp tile character data. */
#define ROM_BG_TILE_TABLE        0x08189034

/* BG tilemap pointer table: 162 u32 ROM pointers to compressed screenblock data.
 * Same indexing as ROM_BG_TILE_TABLE.
 * Each entry points to compressed tilemap data (u16 per cell:
 * bits 0-9=tileID, bit10=hflip, bit11=vflip, bits12-15=palBank). */
#define ROM_BG_TILEMAP_TABLE     0x081892BC

/* BG palette pointer table: 54 u32 ROM pointers to compressed palette data.
 * Indexed as: (vision-1)*9 + world.
 * Each entry points to 512 bytes of GBA RGB555 palette (16 banks x 16 colors). */
#define ROM_BG_PALETTE_TABLE     0x08188F5C

/* BG tile/tilemap configuration lookup table.
 * Used by LoadBGTileData and LoadBGTilemapData to map (sceneIdx, layerIdx)
 * to an entry index into the BG layer struct and ROM data tables.
 *
 * Layout, from the three functions that index it: 4 bytes per scene, split
 * into two 2-byte layer slots, so the byte at [sceneIdx * 4 + layerIdx * 2]
 * is the layer's entry index and the byte after it is its BG-layer id.
 * 0xFF in the entry-index byte terminates a scene's layer list.
 *
 * Declared as a real extern object (address from ldscript) as well as a
 * literal-address macro: DispatchLevelLayerSetup only matches with the
 * symbol_ref spelling -- see the note in ldscript.in.txt.  It is deliberately
 * a flat u8 array rather than a struct: agbcc's ARM default
 * STRUCTURE_SIZE_BOUNDARY is 32 bits, so `struct { u8 a; u8 b; }` is padded to
 * 4 bytes and a struct spelling cannot express the 2-byte slot at all. */

/* The BG layer lookup table at 0x08057ACC.
 *
 *   gBgLayerLookup[levelIdx][sublevel][0] = row index into the ROM sub-tables
 *   gBgLayerLookup[levelIdx][sublevel][1] = BG layer (2 or 3), also the gBgInfo index
 *
 * The stride pair is proven by LoadBGTileData and LoadBGTilemapData, which both
 * compute `levelIdx*4 + sublevel*2` and then read byte [+1] before byte [+0].
 * Both bytes must be reached through ONE array object of this exact shape --
 * that is what makes agbcc fold the `+1` into the symbol (`adds r0, r4, #1`)
 * instead of using `ldrb r0, [r0, #1]`. */
extern const u8 gBgLayerLookup[][2][2];

/* BG tile ROM pointer sub-table.
 * Indexed by the entry byte from gBgLayerLookup to select compressed tile data. */

/* The compressed-tile sub-table at 0x08189BCC: one row of two compressed
 * tile-data pointers per lookup row, selected by BG layer.
 *   gBgTileSubtable[row][layer - 2]
 * (layer is 2 or 3, so the second index is 0 or 1 -- hence the `- 2` bias that
 * shows up as `subs r2, r3, #0x2` here and `subs r0, #0x02` in LoadBGTilemapData.) */
extern const u32 gBgTileSubtable[][2];

/* BG tilemap ROM pointer sub-table.
 * Used by LoadBGTilemapData for per-entry tilemap data selection. */
#define ROM_BG_TILEMAP_SUBTABLE  0x08189CCC

/* BG dimension lookup tables: map (vision, world) to tilemap width/height.
 * Each is a 54-entry table of u16 values (6 visions x 9 worlds). */
#define ROM_BG_WIDTH_TABLE       0x08051C76
#define ROM_BG_HEIGHT_TABLE      0x08051DBA
#define ROM_BG_TILECOUNT_TABLE   0x08051EFE
#define ROM_BG_STRIDE_TABLE      0x08052042

/* BG control register flags lookup table.
 * Used by InitLevelBG for REG_BG0CNT/BG1CNT/BG3CNT setup. */
#define ROM_BG_CONTROL_FLAGS     0x08051BD4

/* Extra BG tables for sublevel==0 (world map / special screens). */
#define ROM_BG_EXTRA_TILES_A     0x0818955C
#define ROM_BG_EXTRA_TILEMAPS_A  0x08189574
#define ROM_BG_OBJ_TILESET_TABLE 0x08189544

/* Per-level collision/layout map table. Decompressed into gCollisionMapPtr. */
#define ROM_COLLISION_MAP_TABLE  0x0818B7AC

/* Per-level parameter table, stored at 0x03005294. */
#define ROM_LEVEL_PARAM_TABLE    0x08189A24

/* Layer configuration sub-tables used by SetupLevelLayerConfig.
 * Define per-layer charblock/screenblock/scroll/dimension properties. */
#define ROM_LAYER_SCROLL_FLAGS   0x080576D4
#define ROM_LAYER_WIDTH_TABLE    0x08057714
#define ROM_LAYER_HEIGHT_TABLE   0x08057794
#define ROM_LAYER_VSCROLL_TABLE  0x08057814
#define ROM_LAYER_TILE_BPP       0x08057894
#define ROM_LAYER_CHARBLOCK_IDX  0x080578D4
#define ROM_LAYER_SCREENBLOCK    0x08057914

/* ── Scene-Specific Shared Tilesets ── */

/* These compressed tilesets are loaded into VRAM charblocks during scene init
 * (SetupSceneGfx / sub_0804886C). They provide shared tiles (HUD, items, etc.)
 * that are referenced by per-level BG tilemaps via absolute tile IDs. */
#define ROM_SCENE_TILESET_A      0x08366214 /* -> charblocks 0-3 via palettePtr */
#define ROM_SCENE_TILESET_B      0x08367468 /* -> small OBJ tiles */
#define ROM_SCENE_TILES_CB0      0x082F4D3C /* -> VRAM 0x06000000 (charblock 0) */
#define ROM_SCENE_TILES_CB1      0x082F518C /* -> VRAM 0x06004000 (charblock 1) */
#define ROM_SCENE_TILES_CB2      0x082F5D0C /* -> VRAM 0x06008000 (charblock 2) */
#define ROM_SCENE_TILES_CB3      0x082F7D64 /* -> VRAM 0x0600C000 (charblock 3) */
#define ROM_SCENE_TILEMAP_DATA   0x082F5920 /* -> IWRAM tilemap buffers */

/* Scene-specific palette data loaded during SetupSceneGfx. */
#define ROM_SCENE_PALETTE_A      0x08078F88
#define ROM_SCENE_PALETTE_B      0x08078FA8

/* OBJ (sprite) tileset for scene overlay. */
#define ROM_SCENE_OBJ_TILES      0x082F4934

/* Sprite layout table for HUD/scene overlay objects.
 * 12-byte entries terminated by 0xFFFF. */
#define ROM_SCENE_SPRITE_TABLE   0x08116590

/* Per-level BG palette table.
 * Indexed by level index; each entry is a ROM pointer to compressed
 * 0x1C0-byte palette data for BG layers. Used by FinalizeLevelLayerSetup. */
extern const u32 gLevelPaletteTable[];
#define ROM_LEVEL_PALETTE_TABLE 0x08189B4C

/* GFX data stream pointer table.
 * Indexed by stream ID; each entry is a ROM pointer to compressed
 * stream data. Used by LoadAndDecompressStream. */
extern void *const gStreamDataTable[];
#define ROM_STREAM_TABLE     0x08189AFC

/* World map BG tile data. */
#define ROM_WORLDMAP_TILES   0x082EA584
#define ROM_WORLDMAP_TILEMAP 0x082EA730
#define ROM_WORLDMAP_PALETTE 0x082EA7F0

/* ── BG Layer / Screen / Palette State ── */

/* BG2 affine rotation angle (Q_8_8 fixed-point, full circle = 256). */
extern u8 gBg2Alpha;

/* BG layer configuration array (3 entries, see struct BGLayerState).
 * Initialized by InitLevelBG with charblock assignments:
 *   Entry 0: tileVram=0x06000000, tmapVram=0x0600E000
 *   Entry 1: tileVram=0x06004000, tmapVram=0x0600E800
 *   Entry 2: tileVram=0x06008000, tmapVram=0x0600F000 */
#define gBGLayerState ((struct BGLayerState *)0x03003430)

/* OBJ palette RAM write cursor: current palette RAM destination for DMA
 * transfers. Advanced by 0x20 after each 32-byte palette DMA during scene
 * setup. (Historically named "VramWrite"; it actually tracks OBJ palette RAM.)
 * Extern volatile void* (address in ldscript) so the read-modify-write
 * sequences in the per-level VBlank DMA loaders reload it after each transfer
 * and agbcc eager-loads its address into a callee-saved register — matching. */
extern void *volatile gVramWriteCursor;

/* Initial VRAM write cursor value, saved at stream init and restored on reset. */
#define gVramCursorInit (*(u32 *)0x030034F4)

/* OBJ VRAM write cursor: tracks current OBJ VRAM destination during
 * sequential tile DMA transfers in scene setup. (Historically named
 * "PaletteVram"; it actually tracks OBJ VRAM.) Extern volatile void* for the
 * same reload-per-transfer / register-hoisting matching reason as
 * gVramWriteCursor. */
extern void *volatile gPaletteVramCursor;

/* Initial palette cursor value, saved at stream init and restored on reset. */
#define gPaletteCursorInit (*(u32 *)0x030052AC)

/* Pointer to decompressed collision/layout map data for current level. */
#define gCollisionMapPtr   (*(u32 *)0x03005290)

/* Level dimension bounds: +0x00=scrollX max, +0x02=scrollY max,
 * +0x04=width, +0x06=height. Set by InitLevelBG. */
#define gLevelBounds       ((u16 *)0x03005468)

/* Per-level window clip bounds, consumed by the HBlank handler that writes
 * REG_WIN0H/V and REG_WIN1H/V: each WINxH packs (left << 4) | (right >> 4),
 * each WINxV packs (top << 4) | (bottom >> 4). The pointer itself lives at
 * 0x030034A0 (the same heap buffer aliased by gGfxBufferPtr).
 *
 * The eight halfwords are two 2x2 arrays, not eight scalars: the first index is
 * the window number and the second is the axis (0 = horizontal, 1 = vertical).
 * StreamCmd_SetWindowCorner indexes BOTH dimensions at run time from stream
 * byte 2 (bit 1 picks leftTop vs rightBottom, bit 0 picks the window), and only
 * the array spelling reproduces agbcc's address arithmetic for it: with plain
 * scalars agbcc folds the member offset into the store's immediate, and the
 * function does not match. Offsets and meanings are unchanged from the earlier
 * scalar declaration (win0Left = leftTop[0][0], win0Top = leftTop[0][1], ...);
 * see include/io_reg.h for the real WIN0H/WIN1H/WIN0V/WIN1V map. */
struct LevelWindowBounds {
    u8 pad_0[0x8];
    /* 0x08: [window][0] = left, [window][1] = top */
    s16 leftTop[2][2];
    /* 0x10: [window][0] = right, [window][1] = bottom */
    s16 rightBottom[2][2];
};
extern struct LevelWindowBounds *gLevelStatePtr;

/* Tilemap work buffer (0x400 bytes): temporary staging for tilemap
 * row/column streaming during BG scrolling. */
#define gTilemapWorkBuffer ((u8 *)0x03004DB0)

/* Screenblock staging buffers in IWRAM: DMA'd to VRAM during VBlank.
 * Each is 0x800 bytes (1024 halfwords = one 32x32 screenblock). */
#define gScreenBufferA     ((u8 *)0x03000900)
#define gScreenBufferB     ((u8 *)0x03001100)
#define gScreenBufferC     ((u8 *)0x03001900)

/* VBlank interrupt callback function pointer.
 * Set to different handlers depending on scene:
 *   sub_080009D9 for normal levels
 *   sub_08000CE1 for mode-7
 *   sub_08000BD5 for title screen */
#define gVBlankCallback    (*(u32 *)0x030047C0)

/* ── BG2 Affine Transform Shadows ── */
/* These IWRAM values are written to hardware registers during VBlank:
 *   gBG2PA → REG_BG2PA (0x04000020) — horizontal scale / cos(angle)
 *   gBG2PB → REG_BG2PB (0x04000022) — horizontal shear / sin(angle)
 *   gBG2PC → REG_BG2PC (0x04000024) — vertical shear / -sin(angle)
 *   gBG2PD → REG_BG2PD (0x04000026) — vertical scale / cos(angle)
 *   gBG2X  → REG_BG2X  (0x04000028) — reference point X (32-bit fixed-point)
 *   gBG2Y  → REG_BG2Y  (0x0400002C) — reference point Y (32-bit fixed-point) */
extern s16 gBg2PA;
extern s16 gBg2PB;
extern s16 gBg2PC;
extern s16 gBg2PD;
extern s32 gBg2X;
extern s32 gBg2Y;

/* Per-frame cached sin/cos of gBg2Alpha (set by VBlankCallback_Dialog). */
#define gUnk_03004678 (*(s16 *)0x03004678)
#define gUnk_030051B0 (*(s16 *)0x030051B0)

/* Map-screen wobble Q_8_8 (set by VBlankCallback_MapScreen). */
extern s16 gUnk_030034F8;

/* ── Scene / Transition State ── */

/* Scene fade/blend counter: decremented by 0x10 each frame during transitions.
 * Used by TransitionInitLevelMusic, TransitionFadeOut*, sub_08024D84. */
#define gSceneFadeCounter (*(u16 *)0x03005210)

/* Scene/gfx state struct: used by InitGfxState, InitFadeTransition,
 * MainGameFrameLoop, PlayerMovementPhysics. Part of the graphics pipeline state. */
#define gGfxSceneState    ((u8 *)0x03004D90)

/* Scene script / title sequence state.
 * Used by RunSceneScript, RunTitleSequence. */
#define gSceneScriptState (*(u32 *)0x03005488)

/* Cutscene/credits animation state.
 * Used by VBlankCallback_Credits, VBlankCallback_Cutscene, TransitionFadeOutWithMusic. */
#define gCutsceneState    ((u8 *)0x030051C8)

/* ── Palette / Visual Effects ── */

/* Palette animation state buffer.
 * Used by AnimatePaletteEffects, InitGfxState, PlayerMovementPhysics. */
#define gPaletteAnimState ((u8 *)0x030051F0)

#endif /* GUARD_GFX_H */
