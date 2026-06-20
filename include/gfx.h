#ifndef GUARD_GFX_H
#define GUARD_GFX_H

#include "global.h"

/* ══════════════════════════════════════════════════════════════════════
 *  Struct Definitions
 * ══════════════════════════════════════════════════════════════════════ */

/* BGLayerState: per-layer BG configuration (28 bytes, 3 entries at gBGLayerState).
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
    u8 status; /* +0x00: entry status (low 3 bits = type, bit 3+ = flags) */
    u8 unk_01; /* +0x01: sub-flags (bit 7 = direction, bits 6:3 = speed) */
    u8 unk_02; /* +0x02: secondary flags */
    u8 unk_03; /* +0x03: bit 7 = sign flag (set from timer sign) */
    u16 unk_04; /* +0x04: tile/frame base index */
    u16 unk_06; /* +0x06: tile count / allocation size */
    u16 unk_08; /* +0x08: counter / position A */
    u16 unk_0A; /* +0x0A: counter / position B */
    u16 unk_0C; /* +0x0C: frame index */
    u16 unk_0E; /* +0x0E: unknown */
    u16 unk_10; /* +0x10: unknown */
    u16 unk_12; /* +0x12: unknown */
    u16 unk_14; /* +0x14: timer value (s16, sign sets bit 7 of byte 3) */
    u16 unk_16; /* +0x16: unknown */
    u16 unk_18; /* +0x18: unknown */
    u16 unk_1A; /* +0x1A: unknown */
    u16 unk_1C; /* +0x1C: unknown */
    u8 unk_1E; /* +0x1E: frame/animation state */
    u8 unk_1F; /* +0x1F: frame/animation param */
    u32 callback; /* +0x20: function pointer for per-frame update */
}; /* total: 0x24 = 36 bytes */

/* ── Graphics Stream ── */

/* Pointer to the current position in the graphics/music data stream.
 * Double indirection: the u32 at this address holds a u8* into the stream.
 * Nearly all gfx stream command functions read/advance this pointer. */
#define gStreamPtr    (*(u8 **)0x03004D84)

/* Pointer to the graphics buffer control struct.
 * Dereferenced for palette state, flags, and buffer management. */
#define gGfxBufferPtr (*(u32 *)0x030034A0)

/* BG2 affine magnification (Q_8_8). Used as 1/scale in BG2PA/PD calculations. */
extern u16 gBg2XMag;
extern u16 gBg2YMag;

/* Decompressed data buffer pointer (allocated by LoadAndDecompress functions). */
#define gDecompBuffer            (*(void **)0x030007D0)

/* Graphics buffer freed by ShutdownGfxStream. */
#define gGfxStreamBuffer         (*(u32 *)0x030007C8)

/* Buffer freed by FreeBuffer_52A4. */
#define gBuffer_52A4             (*(u32 *)0x030052A4)

/* BLDY fade level counter. Incremented/decremented during transitions
 * and written to REG_BLDY (0x04000054) by the VBlank handler. */
#define gBldyFadeLevel           (*(u8 *)0x030007D8)

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
 * to an entry index into the BG layer struct and ROM data tables. */
#define ROM_BG_LOOKUP_TABLE      0x08057ACC

/* BG tile ROM pointer sub-table.
 * Indexed by entry from ROM_BG_LOOKUP_TABLE to select compressed tile data. */
#define ROM_BG_TILE_SUBTABLE     0x08189BCC

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
#define gBGLayerState      ((struct BGLayerState *)0x03003430)

/* VRAM write cursor: current palette RAM destination for DMA transfers.
 * Advanced by 0x20 after each 32-byte palette DMA during scene setup. */
#define gVramWriteCursor   (*(u32 *)0x030007DC)

/* Initial VRAM write cursor value, saved at stream init and restored on reset. */
#define gVramCursorInit    (*(u32 *)0x030034F4)

/* Palette VRAM write cursor: tracks current VRAM destination during
 * sequential tile/palette DMA transfers in scene setup. */
#define gPaletteVramCursor (*(u32 *)0x03005490)

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
 * 0x030034A0 (the same heap buffer aliased by gGfxBufferPtr). */
struct LevelWindowBounds {
    u8 pad_0[0x8];
    s16 win0Left; /* 0x08 */
    s16 win0Top; /* 0x0A */
    s16 win1Left; /* 0x0C */
    s16 win1Top; /* 0x0E */
    s16 win0Right; /* 0x10 */
    s16 win0Bottom; /* 0x12 */
    s16 win1Right; /* 0x14 */
    s16 win1Bottom; /* 0x16 */
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
 * Used by TransitionInitLevelMusic, TransitionFadeOut*, GameplayFrameInit. */
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
