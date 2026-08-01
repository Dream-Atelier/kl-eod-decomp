#ifndef GUARD_UI_H
#define GUARD_UI_H

#include "global.h"

/* ── UI / Text Rendering ── */

/* Sprite attribute table for HUD/dialog rendering.
 * Array of 8-byte OAM-like entries at 0x03004800. */
#define gOamBuffer       ((u8 *)0x03004800)

/* Current sprite/OAM slot index for HUD. */
#define gSpriteSlotIndex (*(u16 *)0x0300466C)

/* Sprite draw count / limit. */
#define gSpriteDrawCount (*(u16 *)0x030051DC)

/* Sprite data table pointer (u32 stored at 0x030051DC by SetSpriteTableFromIndex). */
extern u32 gSpriteRenderPtr;

/* ROM sprite data lookup table (array of u32 pointers). */
extern const u32 gSpriteDataTable[];

/* Display configuration flags (byte). */
#define gDisplayMode     ((u8 *)0x03000810)

/* Scroll/position state for text rendering. */
#define gTextScrollState (*(u32 *)0x030034DC)

/* UI sub-state struct (for dialog, menus, transitions). */
#define gUIState         ((u8 *)0x03004DA0)

/* Secondary display state. */
#define gDisplayState2   ((u8 *)0x03003410)

/* Viewport/layer state. */
#define gViewportState   ((u8 *)0x03005284)

/* ── Gameplay / UI State (extended) ── */

/* Gameplay mode/sub-state: byte accessed with DMA multiplier (×0x800).
 * Used by ProcessInputAndUpdateEntities, UpdateUIState, HandlePauseMenuInput,
 * AnimatePaletteEffects. Controls OAM DMA source selection. */
#define gGameplayMode    (*(u8 *)0x030034BC)

/* Gameplay mode secondary state.
 * Used alongside gGameplayMode by ProcessInputAndUpdateEntities, InitGameplayState. */
#define gGameplayModeAlt ((u8 *)0x030034C0)

#endif /* GUARD_UI_H */
