#ifndef GUARD_ENTITY_H
#define GUARD_ENTITY_H

#include "global.h"

/* ── Entity / Object System ── */

/* Base of the main entity/sprite struct array (~36 bytes per element).
 * Most-referenced address in the entire codebase (1634 refs). */
extern u8 gEntityArray[];

/* OAM shadow buffer entry pointers (used by ClearVideoState and friends).
 * Linker symbols for specific offsets into gEntityArray. */
extern u32 gOamBuffer0[]; /* 0x03002920: entry 0 (full buffer) */
extern u32 gOamBuffer1[]; /* 0x0300293C: entry 1 (skip first) */
extern u32 gOamBuffer6[]; /* 0x03002A8C: entry 6 (skip first 6) */

/* OAM shadow buffer entry pointer (used for sprite attribute writes). */
#define gOamEntryPtr       (*(u32 *)0x03000820)

/* Entity index/status lookup table (entity behavior state tracking). */
#define gEntityStatusTable ((u8 *)0x0300363C)

/* Current entity context pointer (0x40 bytes, entity field access). */
#define gCurrentEntityCtx  (*(u32 *)0x03004670)

/* Entity source table pointer (0x18 bytes, sprite/position data). */
#define gEntitySourcePtr   (*(u32 *)0x03004658)

/* Level/entity data pointer (dereferenced for level layout data). */
#define gLevelDataPtr      (*(u32 *)0x03005288)

/* Global control/state block — flags and status at word/byte offsets. */
#define gControlBlock      ((u8 *)0x03004C20)

/* Control block secondary flags (game state transitions, bit fields). */
#define gControlFlags      ((u8 *)0x03004C08)

/* Game state struct array (byte-field access, ~200+ bytes per entry). */
#define gGameStateArray    ((u8 *)0x03005220)

/* Game state flags struct (byte fields at various offsets). */
extern u8 gGameFlagsPtr[];
#define gGameFlags        ((u8 *)0x03005400)

/* Frame/animation/particle state buffer (entity behavior state). */
#define gAnimStateBuffer  ((u8 *)0x03003590)

/* Graphics/decompression buffer control (0x2C bytes). */
#define gGfxDecompCtrl    (*(u32 *)0x030047FC)

/* UI/rendering state block (byte fields with 4-bit masks). */
#define gUIRenderState    ((u8 *)0x030034B0)

/* Decompress/DMA buffer control struct.
 * Holds pointers to allocated decompression buffers during scene setup.
 *   +0x00: buf0  (tile data A / tilemap layer 0)
 *   +0x04: buf1  (tile data B / tilemap layer 1)
 *   +0x08: buf2  (tile data C)
 *   +0x0C: buf3  (tile data D)
 *   +0x14: buf5  (tilemap layer 2) */
#define gDecompBufferCtrl ((u8 *)0x03004790)

/* Entity/object pointer (double indirection). */
#define gEntityPtr        (*(u32 *)0x03004654)

/* 0x03004680 is the OBJ affine-matrix shadow table — use the typed
 * `gOamAffineMatrix[]` (struct OamAffineMatrix, structs/variables.h) instead of
 * this untyped halfword view. Currently has no users. */
#define gOamSourceTable   ((u16 *)0x03004680)

/* Status byte lookup table. */
#define gStatusTable      ((u8 *)0x03000830)

/* Mixed-mode struct (byte + halfword fields). */
#define gMixedState       ((u8 *)0x03003510)

/* VBlank callback array: two function pointers at 0x030047C0.
 * Set by SetupGfxCallbacks for handler dispatch. */
extern u32 gVBlankCallbackArray[];

/* Callback state array: function pointers and state at 0x03003510.
 * Used by transition functions for scene setup callbacks. */
extern u32 gCallbackStateArray[];

/* ── Entity Subsystem (extended) ── */

/* Entity spawn state: byte fields at offsets 0-3.
 * +0x00: entity count A, +0x01: entity count B,
 * +0x02: palette anim slot, +0x03: palette anim param.
 * Used by SpawnEntityAtPosition, EntityBehaviorMasterUpdate, EntitySpriteFrameUpdate. */
#define gEntitySpawnState     ((u8 *)0x03003610)

/* Entity collision/physics lookup table: indexed by entity slot offset.
 * Used by EntityGravityAndFloorCheck, PlayerMainUpdate, SetupOAMSprite. */
#define gEntityCollisionTable ((u8 *)0x03000790)

/* Entity death/hit reaction state.
 * Used by EntityDeathAnimation, EntityHitReaction, SpawnEntityAtPosition. */
#define gEntityDeathState     ((u8 *)0x0300528C)

/* OAM sprite processing state byte: compared against 0xFE sentinel.
 * Used by ProcessOamSpriteLayout, HandlePauseMenuInput, InitLevelBG,
 * UpdateScrollPosition, UpdateStageSelectScreen. */
#define gOamProcessState      (*(u8 *)0x030052A0)

/* ── Entity Data Tables ── */

/* Entity data table: 8-byte entries indexed by entity type.
 * Used by GetEntityLookupData to read behavior parameters.
 * Offset +5: param A, offset +6: param B. */
extern const u8 gEntityDataTable[];
#define ROM_ENTITY_DATA_TABLE 0x081168E8

/* 0x080D8E14 is gSineTable (declared in data/trig.h), not an entity animation
 * table: docs/dynamic-analysis/scripts/prove-oscillation-fields.mjs (E7) dumps
 * it from ROM and the entries are exactly Q_8_8 sine —
 *   index    0    32    64    96   128   160   192   224
 *   value    0   181   256   181     0  -181  -256  -181
 * and CalcSinCosVelocity indexes it with ((timer * unk_1E) & 0xFF) to drive the
 * gfx-stream oscillation. The stale ROM_ENTITY_ANIM_TABLE alias for the same
 * address had no users and has been removed. */
extern const s16 gSineTable[];

/* Item drop velocity parameter table (2 bytes per entry, indexed by item type).
 * Provides horizontal velocity [+0x00] and vertical amplitude [+0x01].
 * State 3 reads at base offset, state 4 at base+0x0A. */
struct ItemDropParamTable {
    u8 unk_0[5][2];
    u8 unk_A[5][2];
};
extern const struct ItemDropParamTable gItemDropParamTable;

/* Entity sprite attribute table (129 refs).
 * Used by PlayerFollowEntityMovement and entity rendering.
 * Adjacent to ROM_OAM_TEMPLATE (0x080E2A7C), likely extended OAM data. */
#define ROM_ENTITY_SPRITE_TABLE 0x080E2B64

#endif /* GUARD_ENTITY_H */
