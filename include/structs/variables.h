#ifndef GUARD_STRUCTS_VARIABLES_H
#define GUARD_STRUCTS_VARIABLES_H

#include "global.h"

/* BG state table (graphics layer scroll/source pointers).
 * Address: 0x03003430, size = 0x60.
 * Struct layout mirrored from kleod. */
struct Unk_03003430 {
    /* 0x00 */ void *pVramBg0Tiles;
    /* 0x04 */ void *pVramBg0Tilemap;
    /* 0x08 */ u16 bg0HOfs;
    /* 0x0A */ u16 bg0VOfs;
    /* 0x0C */ u8 padC[0x10 - 0xC];
    /* 0x10 */ u16 bg0MapWidth; /* Per-level BG0 tilemap width in tiles */
    /* 0x12 */ u16 bg0MapHeight; /* Per-level BG0 tilemap height in tiles */
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 pad19[0x1C - 0x19];

    /* 0x1C */ void *pVramBg1Tiles;
    /* 0x20 */ void *pVramBg1Tilemap;
    /* 0x24 */ u16 bg1HOfs;
    /* 0x26 */ u16 bg1VOfs;
    /* 0x28 */ u8 pad28[0x2C - 0x28];
    /* 0x2C */ u16 bg1MapWidth; /* Per-level BG1 tilemap width in tiles */
    /* 0x2E */ u16 bg1MapHeight; /* Per-level BG1 tilemap height in tiles */
    /* 0x30 */ u16 unk30;
    /* 0x32 */ u16 unk32;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 pad35[0x38 - 0x35];

    /* 0x38 */ void *pVramBg2Tiles;
    /* 0x3C */ void *pVramBg2Tilemap;
    /* 0x40 */ u16 bg2HOfs;
    /* 0x42 */ u16 bg2VOfs;
    /* 0x44 */ u16 bg2StreamColumn; /* BG2 horizontal stream bookkeeping */
    /* 0x46 */ u16 bg2StreamRow; /* BG2 vertical stream bookkeeping */
    /* 0x48 */ u16 bg2MapWidth; /* Per-level BG2 collision/tile map width in tiles */
    /* 0x4A */ u16 bg2MapHeight; /* Per-level BG2 collision/tile map height in rows */
    /* 0x4C */ u16 unk4C;
    /* 0x4E */ u16 unk4E;
    /* 0x50 */ u8 unk50;
    /* 0x51 */ u8 pad51[0x54 - 0x51];

    /* 0x54 */ void *pVramBg3Tiles;
    /* 0x58 */ void *pVramBg3Tilemap;
    /* 0x5C */ u16 bg3HOfs;
    /* 0x5E */ u16 bg3VOfs;
};
extern struct Unk_03003430 gUnk_03003430;

/* Scene/level state (counters, world/level/room). Address: 0x03004C20. */
struct Unk_03004C20 {
    /* 0x00 */ u32 sceneFrameCounter; /* per-scene; reset to 0 on scene entry */
    /* 0x04 */ u32 globalFrameCounter; /* free-running since boot; never resets */
    /* 0x08 */ u16 unk8;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 level;
    /* 0x0D */ u8 world;
    /* 0x0E */ u8 room;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 pad11[0x12 - 0x11];
    /* 0x12 */ u8 demoNumber;
    /* 0x13 */ u8 demoInputIndex;
    /* 0x14 */ u8 demoNextInputTimer;
};
extern struct Unk_03004C20 gUnk_03004C20;

/* Iris/screen-wipe window state at 0x03004D90.
 * unk4/unk6 hold the pending REG_WIN1H / REG_WIN1V ranges, unk8 the wipe mode
 * (1 = opening, 2 = closing, 0 = idle). Used by InitFadeTransition,
 * MainGameFrameLoop and UpdateScreenWipe. */
struct Unk_03004D90 {
    /* 0x00 */ u8 pad0[0x4];
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u8 unk8;
};
extern struct Unk_03004D90 gUnk_03004D90;

/* Per-frame globals used by VBlank callbacks. */
#define gUnk_03003420 (*(u8 *)0x03003420)
#define gUnk_03005498 (*(u8 *)0x03005498)

/* MOSAIC size (BG/OBJ), written into REG_MOSAIC via MOSAIC_SET. Address 0x030007D8. */
extern u8 gMosaicSize;

/* Screen-shake / wobble animation state (AnimatePaletteEffects). */
extern u8 gUnk_03000000;
extern u8 gUnk_03000001;
extern u8 gUnk_03000002;
extern u8 gUnk_03000003;

/* Per-scanline BG2 affine scroll tables (0xA0 scanlines). */
extern u16 gUnk_03004C40[];
extern u16 gUnk_030052C0[];

/* OAM entity / object "kind" id, stored in gUnk_03002920[i].kind (offset 0x11). */
#define ENTITY_KIND_RED_KEY           0x01
#define ENTITY_KIND_BLUE_KEY          0x02
#define ENTITY_KIND_STAR              0x03
#define ENTITY_KIND_DOOR              0x05
#define ENTITY_KIND_HEART             0x07
#define ENTITY_KIND_DREAM_STONE       0x2C
#define ENTITY_KIND_LARGE_DREAM_STONE 0x2D
#define ENTITY_KIND_ONE_UP            0x2E
#define ENTITY_KIND_GOOMI             0x2F
#define ENTITY_KIND_MOBILE_GOOMI_V    0x31 /* "Mobile Goomi (vertical)" */
#define ENTITY_KIND_MOBILE_GOOMI_D    0x32 /* "Mobile Goomi (diagonal)" */
#define ENTITY_KIND_KLONOA            0x6E /* the player */
#define ENTITY_KIND_BOX               0x6F
#define ENTITY_KIND_MOO               0x76
#define ENTITY_KIND_FLYING_MOO_H      0x77 /* "Flying Moo (horizontal)" */
#define ENTITY_KIND_FLYING_MOO_V      0x78 /* "Flying Moo (vertical)" */

/* Entity/OAM struct (~36 bytes per element).
 * Address: 0x03002920.  Mirrored from kleod's variables.h. */
struct Unk_03002920 {
    /* 0x00 */ u16 xPosBg2;
    /* 0x02 */ u16 yPosBg2;
    /* 0x04 */ u16 xPosScreen;
    /* 0x06 */ u16 yPosScreen;
    /* 0x08 */ u8 unk8;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B_0 */ s32 xOffset : 4; /* signed pixel offset added to the sprite's OAM x (on top of xPosScreen) */
    /* 0x0B_4 */ s32 yOffset : 4; /* signed pixel offset added to the sprite's OAM y (on top of yPosScreen) */
    /* 0x0C_0 */ u32 priority : 2;
    /* 0x0C_2 */ u32 flip : 2; /* OAM flip flags: bit0 = horizontal, bit1 = vertical */
    /* 0x0C_4 */ u32 unkC_4 : 4;
    /* 0x0D_0 */ u32 objMode : 2;
    /* 0x0D_2 */ u32 affineHFlip_matrixNum : 4;
    /* 0x0D_6 */ u32 unkD_6 : 2;
    /* 0x0E_0 */ u32 affineEnable : 1;
    /* 0x0E_1 */ u32 affineDouble : 1;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 onScreen; /* 1 if the entity's screen position is within the visible area (+margin), else 0 */
    /* 0x11 */ u8 kind; /* entity/object type id; see ENTITY_KIND_* */
    /* 0x12 */ u8 pad12[0x1C - 0x12];
}; /* size = 0x1C */
extern struct Unk_03002920 gUnk_03002920[];

/* kleod-canonical typed views of the same data, used by logic ported from
 * kleod so field/index expressions match kleod exactly. These alias the same
 * addresses as gUnk_03002920 (0x03002920) and gUnk_03003430 (0x03003430);
 * existing code continues to use the local names/structs above. */
union __attribute__((packed)) EntityInfo_8 {
    struct __attribute__((packed)) {
        u8 unk8;
        u8 unk9;
    } split;
    u16 all;
};
struct EntityInfo {
    /* 0x00 */ u16 xPosBg2;
    /* 0x02 */ u16 yPosBg2;
    /* 0x04 */ u16 xPosScreen;
    /* 0x06 */ u16 yPosScreen;
    /* 0x08 */ union EntityInfo_8 unk8;
    /* 0x0A */ u8 unkA;
    /* 0x0B_0 */ s32 unkB_0 : 4;
    /* 0x0B_4 */ s32 unkB_4 : 4;
    /* 0x0C_0 */ u32 priority : 2;
    /* 0x0C_2 */ u32 unkC_2 : 2;
    /* 0x0C_4 */ u32 unkC_4 : 4;
    /* 0x0D_0 */ u32 objMode : 2;
    /* 0x0D_2 */ u32 affineHFlip_matrixNum : 4;
    /* 0x0D_6 */ u32 unkD_6 : 2;
    /* 0x0E_0 */ u32 affineEnable : 1;
    /* 0x0E_1 */ u32 affineDouble : 1;
    /* 0x0F */ u8 unkF;
    /* 0x10 */ u8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 pad13[0x14 - 0x13];
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u8 unk16;
    /* 0x17 */ u8 unk17;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 pad19[0x1C - 0x19];
}; /* size = 0x1C */
extern struct EntityInfo gEntityInfo[];

/* BG layer info, kleod-canonical array form (0x1C bytes per layer, 4 layers
 * at 0x03003430). Aliases gUnk_03003430. */
struct BgInfo {
    /* 0x00 */ void *pTiles;
    /* 0x04 */ void *pTilemap;
    /* 0x08 */ u16 hOfs;
    /* 0x0A */ u16 vOfs;
    /* 0x0C */ u16 tileCol;
    /* 0x0E */ u16 tileRow;
    /* 0x10 */ u16 hLength;
    /* 0x12 */ u16 vLength;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 pad19[0x1C - 0x19];
}; /* size = 0x1C */
extern struct BgInfo gBgInfo[4];

/* Per-entity movement parameter row (ROM table gUnk_081168E8). */
struct Unk_0803D4AC {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    s8 unk3;
    s8 unk4;
    u8 unk5;
    u8 unk6;
};
/* Decompressed BG tile/tilemap buffer pointers (0x03004790). */
struct BgDataPtrs {
    /* 0x00 */ void *pBufBg0Tiles;
    /* 0x04 */ u16 *pBufBg0Tilemap;
    /* 0x08 */ void *pBufBg1Tiles;
    /* 0x0C */ u16 *pBufBg1Tilemap;
    /* 0x10 */ void *pBufBg2Tiles;
    /* 0x14 */ u8 *pBufBg2Tilemap;
    /* 0x18 */ void *pBufBg3Tiles;
    /* 0x1C */ u16 *pBufBg3Tilemap;
}; /* size = 0x20 */
/* World-map completion/animation state (0x03004C08). */
struct Unk_03004C08 {
    u8 unk0_0 : 4;
    u8 unk0_4 : 4;
    s8 unk1;
    u8 unk2;
    u8 pad3[0x4 - 0x3];
};

/* Active entity count (number of slots in gUnk_03002920 to iterate). Entities
 * below 0xD are fixed slots, so InitGfxStreamState and ResetGfxStreamEntries
 * rewind it to 0xD when they tear the graphics stream down. */
extern u8 gUnk_03005428;

/* Arming flag for the player avatar of the hidden boot-menu minigame (0x0300549C).
 * ConfigureInterruptsForGameplay stores IsDpadUpHeld() here on the single frame the
 * minigame loop starts; UpdateBootMinigame's Select handler only spawns the avatar
 * when it is 1.  Proven at runtime by
 * docs/dynamic-analysis/scripts/prove-minigame-player-armed.mjs. */
extern u8 gMinigamePlayerArmed;

/* gNewKeys / gHeldKeys live in input.h, next to the rest of the input state. */

/* Game-state struct at 0x03003410 (mirrored from kleod). */
struct Unk_03003410 {
    u32 unk0;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8;
    u8 unk9;
    u8 unkA;
    u8 unkB;
    u8 unkC;
};
extern struct Unk_03003410 gUnk_03003410;

/* Frame-callback dispatch table at 0x03003510 (mirrored from kleod). */
typedef void (*IntrFunc)(void);
/* GBA interrupt vector table at 0x030047C0 (mirrored from kleod). */
struct IntrTable {
    /* 0x00 */ IntrFunc vBlank;
    /* 0x04 */ IntrFunc hBlank;
    /* 0x08 */ IntrFunc vCount;
    /* 0x0C */ IntrFunc timer0;
    /* 0x10 */ IntrFunc timer1;
    /* 0x14 */ IntrFunc timer2;
    /* 0x18 */ IntrFunc timer3;
    /* 0x1C */ IntrFunc serial;
    /* 0x20 */ IntrFunc dma0;
    /* 0x24 */ IntrFunc dma1;
    /* 0x28 */ IntrFunc dma2;
    /* 0x2C */ IntrFunc dma3;
    /* 0x30 */ IntrFunc keypad;
    /* 0x34 */ IntrFunc gamePak;
}; /* size = 0x38 */
extern struct IntrTable gIntrTable;
/* BG0..BG3 tilemap scratch buffers (kleod-canonical, address 0x03000900). */
extern u16 gBgTilemapBufs[4][0x400];
/* World-load progress / save-slot byte array (kleod-canonical, address 0x03004670). */
struct Unk_03004670 {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 unk7;
    u8 unk8[6][8];
    s32 unk38;
    u8 pad3C[0x40 - 0x3C];
};
extern struct Unk_03004670 *gUnk_03004670;
/* Scratch decompression buffer pointer (kleod-canonical, address 0x03005290). */
extern void *gUnk_03005290;
/* Sprite/palette state byte (kleod-canonical, address 0x030052A0). */
extern u8 gUnk_030052A0;

/* HandlePauseMenuInput / UpdateUIState entity-link state. */
extern u8 gUnk_030047B8;
extern u8 gUnk_0300528C;
extern u8 gUnk_03005470;
extern u8 gUnk_030034BC;

struct CallbackQueue {
    /* 0x00 */ void (*current[10])(void); // current callbacks
    /* 0x28 */ void (*next[10])(void); // next callbacks
    /* 0x50 */ void (*previous[10])(void); // previous callbacks
    /* 0x78 */ u8 currentCount; // current callback count
    /* 0x79 */ u8 nextCount; // next callback count
    /* 0x7A */ u8 previousCount; // previous callback count
    /* 0x7B */ u8 pad7B[0x7C - 0x7B];
}; /* size = 0x7C */
extern struct CallbackQueue gCallbackQueue;

/* BG-tile/tilemap workbuffer pointers (mirrored from kleod). */
struct Unk_03004790 {
    /* 0x00 */ void *pBufBg0Tiles;
    /* 0x04 */ u16 *pBufBg0Tilemap;
    /* 0x08 */ void *pBufBg1Tiles;
    /* 0x0C */ u16 *pBufBg1Tilemap;
    /* 0x10 */ void *pBufBg2Tiles;
    /* 0x14 */ u8 *pBufBg2Tilemap;
};
extern struct Unk_03004790 gUnk_03004790;

/* BG2 column tile scratch buffer. */
extern u8 gUnk_03004DB0[];

/* Camera scroll bounds for the current room, in world pixels (left/top/right/bottom). */
struct ScrollBounds {
    u16 scrollLeft; /* left bound: camera bg2HOfs clamped to >= scrollLeft */
    u16 scrollTop; /* top bound: camera bg2VOfs clamped to >= scrollTop */
    u16 scrollRight; /* right bound: camera bg2HOfs clamped to <= scrollRight - 0xF0 (screen width) */
    u16 scrollBottom; /* bottom bound: camera bg2VOfs clamped to <= scrollBottom - 0xA0 (screen height) */
};
extern struct ScrollBounds gScrollBounds;

/* World-load lockout flag. */
extern u8 gUnk_03004660;

/* Camera mode-switch state at 0x030007E0 (mirrored from kleod). */
struct Unk_030007E0 {
    /* 0x0 */ s16 unk0;
    /* 0x2 */ s16 unk2;
    /* 0x4 */ s16 unk4;
    /* 0x6 */ s16 unk6;
    /* 0x8 */ s16 unk8;
    /* 0xA */ s16 unkA;
    /* 0xC_0 */ u8 unkC_0 : 4;
    /* 0xC_4 */ u8 unkC_4 : 4;
    /* 0xD */ u8 padD[0x10 - 0xD];
};
extern struct Unk_030007E0 gUnk_030007E0;

/* Player/scroll wobble & fall state at 0x03005400 (0x18 bytes, from kleod). */
struct Unk_03005400 {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08_0 */ u8 unk8_0 : 1;
    /* 0x08_1 */ u8 unk8_1 : 1;
    /* 0x08_2 */ u8 unk8_2 : 1;
    /* 0x08_3 */ u8 unk8_3 : 1;
    /* 0x08_4 */ u8 unk8_4 : 1;
    /* 0x08_5 */ u8 unk8_5 : 1;
    /* 0x08_6 */ u8 unk8_6 : 1;
    /* 0x08_7 */ u8 unk8_7 : 1;
    /* 0x09 */ u8 unk9;
    /* 0x0A */ u8 unkA;
    /* 0x0B */ u8 unkB;
    /* 0x0C */ u8 unkC;
    /* 0x0D */ u8 unkD;
    /* 0x0E_0 */ u8 unkE_0 : 1;
    /* 0x0E_1 */ u8 unkE_1 : 1;
    /* 0x0E_2 */ u8 unkE_2 : 1;
    /* 0x0E_3 */ u8 unkE_3 : 1;
    /* 0x0E_4 */ u8 unkE_4 : 1;
    /* 0x0E_5 */ u8 unkE_5 : 2;
    /* 0x0E_7 */ u8 unkE_7 : 1;
    /* 0x0F */ s8 unkF;
    /* 0x10 */ s8 unk10;
    /* 0x11 */ u8 unk11;
    /* 0x12 */ u8 unk12;
    /* 0x13 */ u8 unk13;
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 unk15;
    /* 0x16 */ s8 unk16;
    /* 0x17 */ u8 pad17[0x18 - 0x17];
}; /* size = 0x18 */
extern struct Unk_03005400 gUnk_03005400;

/* Affine reference points at 0x03005440. */
struct Unk_03005440 {
    /* 0x00 */ u16 unk0;
    /* 0x02 */ u16 unk2;
    /* 0x04 */ u16 unk4;
    /* 0x06 */ u16 unk6;
    /* 0x08 */ u8 pad8[0xC - 0x8];
    /* 0x0C */ u16 unkC;
    /* 0x0E */ u16 unkE;
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ u8 pad14[0x18 - 0x14];
    /* 0x18 */ u16 unk18;
    /* 0x1A */ u16 unk1A;
    /* 0x1C */ u16 unk1C;
    /* 0x1E */ u16 unk1E;
};
extern struct Unk_03005440 gUnk_03005440;

/* Camera scroll auto-advance offsets. */
struct Unk_030034E8 {
    s32 unk0;
    s32 unk4;
};
extern struct Unk_030034E8 gUnk_030034E8;
extern u8 gUnk_030051B8;
extern s32 gUnk_03005480;
extern s32 gUnk_030007C0;
extern u16 gPrevBg2VOfs; /* Previous frame's bg2VOfs */

/* Game-state struct at 0x03005220 (mirrored from kleod, full layout). */
struct Unk_03005220 {
    /* 0x00_0 */ u32 hearts : 2; /* player health 0..3 */
    /* 0x00_2 */ u32 stars : 3; /* bitmask: one bit per star piece collected this vision; HUD count = popcount */
    /* 0x00_5 */ u32 dreamStones : 7; /* dream stones collected this vision (the HUD "N/30") */
    /* 0x01_4 */ u32 keys : 3; /* bitfield of keys */
    /* 0x01_7 */ u32 unk1_7 : 8;
    /* 0x02_7 */ u32 unk2_7 : 6;
    /* 0x03_5 */ u32 unk3_5 : 1;
    /* 0x03_6 */ u32 unk3_6 : 1;
    /* 0x03_7 */ u32 unk3_7 : 1;
    /* 0x04 */ u32 unk4;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ u32 unkC;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u16 unk18;
    /* 0x1A */ u16 unk1A;
    /* 0x1C */ u16 unk1C;
    /* 0x1E */ u16 unk1E;
    /* 0x20 */ u16 unk20;
    /* 0x22 */ u16 unk22;
    /* 0x24 */ u16 unk24;
    /* 0x26 */ s16 unk26;
    /* 0x28 */ s16 unk28;
    /* 0x2A */ s16 unk2A;
    /* 0x2C */ s16 unk2C;
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ s8 unk2F;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 unk32;
    /* 0x33 */ u8 unk33;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 unk35;
    /* 0x36 */ u8 unk36;
    /* 0x37 */ u8 unk37;
    /* 0x38 */ u8 unk38;
    /* 0x39 */ u8 unk39;
    /* 0x3A */ u8 unk3A;
    /* 0x3B */ u8 unk3B;
    /* 0x3C */ u8 unk3C;
    /* 0x3D */ u8 unk3D;
    /* 0x3E */ u8 unk3E;
    /* 0x3F */ u8 unk3F;
    /* 0x40 */ u8 unk40;
    /* 0x41 */ u8 unk41;
    /* 0x42 */ u8 unk42;
    /* 0x43 */ u8 unk43;
    /* 0x44 */ u8 unk44;
    /* 0x45 */ u8 unk45;
    /* 0x46 */ u8 deathSequenceTimer; /* death->respawn latch/phase-clock: nonzero = sequence active (freezes camera, blocks
                                         pause & re-death). Armed to 0x46 on death (hearts==0), counted down to sequence the fade,
                                         cleared to 0 at respawn; the value is a fade-countdown ordinal (fade-out runs 0x41->0x01,
                                         = the 16-step screen blend) */
    /* 0x47 */ u8 unk47;
    /* 0x48 */ u8 unk48;
    /* 0x49 */ u8 unk49;
    /* 0x4A */ u8 unk4A;
    /* 0x4B */ u8 unk4B;
    /* 0x4C */ s8 lives; /* lives count (HUD "x N") */
    /* 0x4D */ u8 unk4D;
    /* 0x4E */ u8 unk4E;
    /* 0x4F */ u8 unk4F;
    /* 0x50 */ u8 unk50;
    /* 0x51 */ u8 unk51;
    /* 0x52 */ u8 unk52;
    /* 0x53 */ u8 unk53;
    /* 0x54 */ s8 unk54;
    /* 0x55 */ s8 unk55;
    /* 0x56 */ s8 unk56;
    /* 0x57 */ s8 unk57;
    /* 0x58 */ u8 unk58;
    /* 0x59 */ u8 unk59;
    /* 0x5A */ u8 unk5A;
    /* 0x5B */ u8 unk5B;
    /* 0x5C */ u8 unk5C;
    /* 0x5D */ u8 unk5D;
    /* 0x5E */ u8 unk5E;
    /* 0x5F */ u8 unk5F;
    /* 0x60 */ u16 unk60;
    /* 0x62 */ u8 pad62[0x64 - 0x62];
}; /* size = 0x64 */
extern struct Unk_03005220 gUnk_03005220;

/* Level-config view struct at 0x03005284, accessed via pointer.
 * Holds per-level entity/state replay values. */
struct Unk_030034B0 {
    /* 0x00_0 */ u8 unk0_0 : 1;
    /* 0x00_1 */ u8 unk0_1 : 3;
    /* 0x00_4 */ u8 visionStartPending : 1; /* latched high for the ~30-frame confirm-jingle
                                             * delay in RunVisionStartConfirmDelay and cleared when the scene
                                             * fade-out is queued. The latch itself is proven at
                                             * runtime (docs/dynamic-analysis/scripts/prove-vision-start-pending.mjs);
                                             * the "vision start" reading rests on that single
                                             * install site, so treat the name as provisional. */
    /* 0x00_5 */ u8 unk0_5 : 3;
    /* 0x01 */ u8 pad1[0x5 - 0x1];
    /* 0x05 */ u8 visionArrivalTimer; /* armed to 0x80 the frame the globe reaches the
                                       * selected node, then counts down 1/frame:
                                       * 0x40 = jingle + clear the node's progress byte,
                                       * 0x01 = hand over to FindNextUnlockedVision. */
    /* 0x06_0 */ u8 unk6_0 : 4;
    /* 0x06_4 */ u8 unk6_4 : 4;
    /* 0x07_0 */ u8 selectedVision : 4; /* world-map vision the globe is driven to,
                                         * 1-based; 0 = auto-rotation idle. */
    /* 0x07_4 */ u8 unk7_4 : 4;
};
struct Unk_03005294_03005418_0 {
    u32 src;
    u8 unk4;
    s32 unk5_0 : 4;
    s32 unk5_4 : 4;
};
struct Unk_03005294_03005418 {
    struct Unk_03005294_03005418_0 **unk0;
    void *dest;
    u16 size;
    u8 unkA;
    u8 padB[0xC - 0xB];
};

struct Unk_03005284 {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 unk3;
    /* 0x04 */ u8 unk4;
    /* 0x05 */ u8 unk5;
    /* 0x06 */ u8 unk6;
    /* 0x07 */ u8 unk7;
    /* 0x08_0 */ u8 unk8_0 : 2;
    /* 0x08_2 */ u8 unk8_2 : 3;
    /* 0x08_5 */ u8 unk8_5 : 7;
    /* 0x09_4 */ u8 unk9_4 : 3;
    /* 0x09_7 */ u8 unk9_7 : 8;
    /* 0x0A_7 */ u8 unkA_7 : 6;
    /* 0x0B_5 */ u8 unkB_5 : 1;
    /* 0x0B_6 */ u8 unkB_6 : 1;
    /* 0x0C */ u32 unkC;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u32 unk18;
    /* 0x1C */ u8 unk1C;
    /* 0x1D */ u8 unk1D;
    /* 0x1E */ u8 unk1E;
};
extern struct Unk_03005284 *gUnk_03005284;

/* ROM data tables (level config / room positions / boss positions). */
struct Unk_080D821C_4 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    u8 unk8;
};
struct Unk_080D821C {
    u16 unk0;
    u16 unk2;
    struct Unk_080D821C_4 *unk4;
    u8 unk8;
    u8 unk9;
    u8 padA[0xC - 0xA];
};
extern struct Unk_080D821C gUnk_080D821C[0xD];
extern struct Unk_080D821C *gUnk_03004D80;

struct Unk_080D6458 {
    u16 unk0;
    u16 unk2;
    u8 unk4_0 : 2;
    u8 pad5[0x8 - 0x5];
};
extern struct Unk_080D6458 gUnk_080D6458[6];

struct Unk_080D48C8 {
    u16 unk0;
    u16 unk2;
    u8 unk4_0 : 2;
    u8 unk4_2 : 6;
    u8 pad5[0x8 - 0x5];
};
extern struct Unk_080D48C8 gUnk_080D48C8[6][7][0x15];

struct Unk_080D89A8 {
    s32 unk0;
    s32 unk4;
};
extern struct Unk_080D89A8 gUnk_080D89A8[6][5];

/* Camera-trigger record (sentinel-terminated array). */
struct Unk_0300542C {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u16 unk4;
    /* 0x6 */ u16 unk6;
    /* 0x8 */ s8 unk8;
    /* 0x9 */ s8 unk9;
    /* 0xA */ u8 padA[0xC - 0xA];
}; /* size = 0xC */
extern struct Unk_0300542C *gUnk_0300542C;
extern struct Unk_0300542C *gUnk_0818B704[6][7];

/* sub_08001F58 (ProcessOamSpriteLayout) extras. */
extern u16 gUnk_030008E8;
extern u16 gUnk_0300358C;

/* Loose globals used by sub_0800CA0C. */
extern u16 gUnk_03003508; /* halfword-stored per target asm */
extern u8 gUnk_03000810;
/* Saved scene callback: ShutdownGfxSubsystem stores gControlBlock[1] here
 * before tearing down; m4a sub_0804EB64 / SoundContextInit also reference
 * the same address. */
extern u32 gUnk_03000814;
extern u8 gUnk_030051C8;
extern u16 gUnk_030051E0;
extern u8 gUnk_030034C4;
extern u8 gUnk_030034E4; /* same address as gPauseFlag macro; kleod-style alias */
extern u8 *gUnk_03004654; /* small ROM/IWRAM byte pointer (indexed [1] in level lookup) */

/* Globals first referenced by sub_08002FD0 (InitLevelFromROMTable). */
extern u8 gUnk_03000800;
extern u16 gUnk_03005210;
/* Center point of the current room's scroll bounds (world px) */
struct ScrollCenter {
    s16 x;
    s16 y;
};
extern struct ScrollCenter gScrollCenter;

struct Unk_080D2E88 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
};
extern struct Unk_080D2E88 gUnk_080D2E88[6][7][0x14];
extern u16 gUnk_08051EFE[6][9][3];
/* ROM lookup table at gUnk_08051EFE + 0xEA = 0x08051FE8: per-(world, slot) row */
extern u8 gLevelRoomData[6][8][0x1C];
extern u8 gUnk_08052624[6][9];
extern u8 gUnk_03003650[][0x40];

/* Globals first referenced by sub_08001158 (InitLevelBG). */
extern u32 *gUnk_08189034[6][9][3]; /* tile-size pointer table */
extern u32 *gUnk_081892BC[6][9][3]; /* tilemap-size pointer table */
extern u32 *gUnk_0818955C[6]; /* cutscene-mode BG1 tile size */
extern u32 *gUnk_08189574[6]; /* cutscene-mode BG1 tilemap size */
extern u32 *gUnk_0818B7AC[12]; /* world-intro bg compressed blobs */
extern void *gUnk_08188F5C[6][9]; /* per-(world,level) BG palette blob */
extern u32 *gUnk_08189544[6]; /* cutscene-mode BG palette blob */
extern u8 gUnk_08051BD4[6][9][3]; /* BGxCNT flag bank (bpp bit) */
extern u16 gUnk_08051C76[6][9][3]; /* BG pixel width  per layer */
extern u16 gUnk_08051DBA[6][9][3]; /* BG pixel height per layer */
extern u8 gUnk_08052042[6][9][3]; /* BG tile length per layer */
extern u16 gUnk_0805265A[6]; /* cutscene BG1 width */
extern u16 gUnk_08052666[6]; /* cutscene BG1 height */
extern u16 gUnk_08052672[6]; /* cutscene BG1 tilemap width */
extern u8 gUnk_0805267E[6]; /* cutscene BG1 tilemap height */

/* OAM-entry union (matches kleod's variables.h).  Used by InitOamEntries,
 * RenderHUDTop, RenderDialogSprites, and other OAM writers. */
union Unk_03000820 {
    struct {
        /* 0x0_0 */ u32 y : 8;
        /* 0x1_0 */ u32 affineMode : 2;
        /* 0x1_2 */ u32 objMode : 2;
        /* 0x1_4 */ u32 mosaic : 1;
        /* 0x1_5 */ u32 bpp : 1;
        /* 0x1_6 */ u8 shape : 2;
        /* 0x2_0 */ u16 x : 9;
        /* 0x3_1 */ u8 matrixNum : 3;
        /* 0x3_4 */ u32 hFlip : 1;
        /* 0x3_5 */ u32 vFlip : 1;
        /* 0x3_6 */ u32 size : 2;
        /* 0x4_0 */ u16 tileNum : 10;
        /* 0x5_2 */ u8 priority : 2;
        /* 0x5_4 */ u16 paletteNum : 4;
        /* 0x6_0 */ u8 pad6[0x8 - 0x6];
    } split;
    struct {
        /* 0x0 */ u32 attr01;
        /* 0x4 */ u16 attr2;
        /* 0x6 */ u16 affineParam;
    } all;
}; /* size = 0x8 */
extern union Unk_03000820 gUnk_03004800[]; /* OAM shadow buffer */
extern const union Unk_03000820 gUnk_080E2A7C; /* ROM OAM template */
extern union Unk_03000820 *gUnk_03000820; /* current OAM-shadow write head */

/* Per-entity sprite-control sidecar at 0x03000830 (mirrored from kleod). */
struct Unk_03000830 {
    /* 0x0 */ u8 unk0;
    /* 0x1 */ u8 unk1;
    /* 0x2 */ u8 pad2[0x4 - 0x2];
}; /* size = 0x4 */
extern struct Unk_03000830 gUnk_03000830[];

struct EntityAnimationInfo {
    u8 state;
    u8 timer;
    volatile u8 frame;
    u8 pad3[1];
};

/* OBJ (sprite) affine-matrix shadow table at 0x03004680, one entry per hardware
 * OAM affine matrix. The OAM builder copies entry m into the GBA's matrix m,
 * whose PA/PB/PC/PD live interleaved in OAM at 0x07000006 + 32m, +0x0E, +0x16
 * and +0x1E (attribute3 of OAM entries 4m+0..4m+3). Entities select a matrix
 * through `Unk_03002920.affineHFlip_matrixNum`.
 *
 * Identity is {0x100, 0, 0, 0x100} (Q_8_8). pa/pd are *inverse* scales — the
 * hardware maps screen space back to texture space — so pa = pd = 0x200 renders
 * the sprite at half size. That is why StreamCmd_SetEntityTransform loads
 * pa = pd = 0x100 / mag.
 *
 * Field names proven at runtime, not guessed — see
 * docs/dynamic-analysis/scripts/prove-oam-affine-matrix.mjs: writing the four
 * distinct sentinels {0x1111, 0x2222, 0x3333, 0x4444} into one slot makes
 * exactly those values appear at that matrix's PA, PB, PC, PD in that order,
 * and perturbing only pa/only pd squashes a sprite horizontally/vertically. */
struct OamAffineMatrix {
    u16 pa; /* +0x00: x scale (cos component) -> OAM PA */
    u16 pb; /* +0x02: x shear (sin component) -> OAM PB */
    u16 pc; /* +0x04: y shear                 -> OAM PC */
    u16 pd; /* +0x06: y scale (cos component) -> OAM PD */
}; /* size = 0x8 */
extern struct OamAffineMatrix gOamAffineMatrix[];

/* Sprite-part metadata referenced via gUnk_0300466C / gUnk_08078FC8 / gUnk_030051DC. */
struct Unk_0300466C_4 {
    u16 tileNum;
    u8 bpp_paletteNum;
    s8 unk3;
    u8 unk4;
    u8 shape_size;
};
struct Unk_0300466C {
    u8 unk0;
    u8 pad1[0x4 - 0x1];
    struct Unk_0300466C_4 *unk4;
};
extern struct Unk_0300466C gUnk_08078FC8[];
extern struct Unk_0300466C *gUnk_0300466C;

extern struct Unk_0300466C *gUnk_030051DC;
extern struct Unk_0300466C *gUnk_0818B8E0[6][9];

/* Per-tileset sprite tile-data source pointers (DMA src), indexed by tileset. */
extern u32 gUnk_0818B8A8[];

/* Text-scroll Bresenham accumulator (mirrors kleod's u16 typing). */
extern u16 gUnk_030034DC;

/* sub_0800BEF0 (UpdateTextScroll) parameter struct. */
struct Unk_0800BEF0 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    s8 unk8;
    s8 unk9;
};

/* sub_08003DC0 (SetupOAMSprite) — sprite-bookkeeping table at 0x080E2B64
 * indexed [world-1][level-1][slot] giving per-slot {unk0[5]=room rec, unk28, unk29}. */
struct Unk_080E2B64_0 {
    u16 unk0;
    u16 unk2;
    u8 unk4;
    u8 unk5;
    u8 unk6;
    u8 pad7[0x8 - 0x7];
};
struct Unk_080E2B64 {
    struct Unk_080E2B64_0 unk0[5];
    u8 unk28;
    u8 unk29;
    u8 pad2A[0x2C - 0x2A];
};
extern struct Unk_080E2B64 gUnk_080E2B64[6][8][0x64];

/* Sprite-related side tables used by sub_08003DC0. */
struct Unk_03000790 {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u16 unk4;
    /* 0x6 */ u16 unk6;
    /* 0x8 */ u16 unk8;
    /* 0xA */ u8 padA[0x10 - 0xA];
}; /* size = 0x10 */
extern struct Unk_03000790 gUnk_03000790[];

struct Unk_03003610 {
    u8 unk0;
    u8 unk1;
    u8 unk2;
    u8 unk3;
}; /* size = 0x4 */
extern struct Unk_03003610 gUnk_03003610[];

/* Loose globals first referenced by sub_08003DC0 (SetupOAMSprite). */
extern s32 gUnk_030007D4;
extern s32 gUnk_030007F0;
extern s32 gUnk_030007F4;
extern s32 gUnk_03000804;
extern u8 gUnk_03000818;
extern s32 gUnk_03000824;
extern s32 gUnk_0300082C;
extern u8 gUnk_030008EC;
extern s32 gUnk_030008F0;
extern s32 gUnk_030008F4;
extern s32 gUnk_030008FC;
extern s32 gUnk_03002904;
extern s32 gUnk_03002908;
extern s32 gUnk_0300290C;
extern s32 gUnk_030034A4;
extern s32 gUnk_030034C8;
extern s32 gUnk_030034CC;
extern s32 gUnk_030034D8;
extern s32 gUnk_03003500;
extern s32 gUnk_03003504;
extern s32 gUnk_03003630;
extern s32 gUnk_03003634;
extern s32 gUnk_03003638;
extern s32 gUnk_03003640;
extern s32 gUnk_03004650;
extern s32 gUnk_03004664;
extern s32 gUnk_03004674;
extern s32 gUnk_03004788;
extern s32 gUnk_030047B4;
extern s32 gUnk_030047BC;
extern s32 gUnk_030047F8;
extern u8 gUnk_03004C00;
extern s32 gUnk_03004C04;
extern u8 gUnk_03004C38;
extern u8 gUnk_030051B4;
extern s32 gUnk_030051C4;
extern s32 gUnk_030051D4;
extern s32 gUnk_030051D8;
extern u8 gUnk_03005288;
extern u8 gUnk_03005298;
extern s32 gUnk_0300529C;
extern u8 gUnk_030052A8;
extern s32 gUnk_030052B0;
extern s32 gUnk_030052B4;
extern s32 gUnk_0300541C;
extern s32 gUnk_03005424;
extern s32 gUnk_03005430;
extern u8 gUnk_0300547C;
extern s32 gUnk_03005484;
extern u8 gUnk_030034E0;
extern void *gUnk_03004C10;

/* sub_0800BFF4 family (kleod code_0800BFF4.c) state struct at 0x030051F0. */
struct Unk_030051F0 {
    s32 unk0;
    u16 unk4;
    u16 unk6;
    u16 unk8;
    u16 unkA;
    u16 unkC;
    u8 unkE;
};
extern struct Unk_030051F0 gUnk_030051F0;

/* sub_0800AC34 (UpdateUIState) — entity affine-matrix scratch table and friends. */
struct Unk_03003590 {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u8 unk4;
    /* 0x5_0 */ u8 unk5_0 : 1;
    /* 0x6 */ u8 pad6[0x8 - 0x6];
}; /* size = 0x8 */
extern struct Unk_03003590 gUnk_03003590[];
extern void (*gUnk_030034A8)(u8);
extern u8 gUnk_0300363C;
extern const u8 gUnk_080E2A84[0x6][0x8];

/* ROM data tables referenced by decompiled code (kleod-imported). */
extern void *gUnk_030034F4;
extern u8 gUnk_03003790[][0x40];
extern void *gUnk_030052AC;
extern u8 gUnk_0805C6E8[0x200];
extern u8 gUnk_0805C8E8[0x80];
extern u8 gUnk_0805C968[0x80];
extern u8 gUnk_0805C9E8[0x800];
extern u8 gUnk_0805D1E8[0x800];
extern u8 gUnk_0805ECE8[0x200];
extern u8 gUnk_0805EEE8[0x200];
extern u8 gUnk_0805F0E8[0x200];
extern u8 gUnk_0805F2E8[0x80];
extern u8 gUnk_0805F368[0x20];
extern u8 gUnk_0805F388[0x80];
extern u8 gUnk_0805F408[0x80];
extern u8 gUnk_0805F488[0x80];
extern u8 gUnk_0805F508[0x200];
extern u8 gUnk_0805F708[0x80];
extern u8 gUnk_0805F788[0x80];
extern u8 gUnk_0805F808[0x200];
extern u8 gUnk_0805FA08[0x100];
extern u8 gUnk_0805FB08[0x100];
extern u8 gUnk_0805FC08[0x200];
extern u8 gUnk_0805FE08[0x800];
extern u8 gUnk_08060608[0x100];
extern u8 gUnk_08060708[0x100];
extern u8 gUnk_08060808[0x200];
extern u8 gUnk_08060A08[0x80];
extern u8 gUnk_08060A88[0x600];
extern u8 gUnk_08061088[0x800];
extern u8 gUnk_08061A28[0x200];
extern u8 gUnk_08061C28[0x100];
extern u8 gUnk_08061D28[0x20];
extern u8 gUnk_08061D48[0x20];
extern u8 gUnk_08061D68[0x20];
extern u8 gUnk_08061D88[0x20];
extern u8 gUnk_08061DA8[0x20];
extern u8 gUnk_08061DC8[0x200];
extern u8 gUnk_08061FC8[0x80];
extern u8 gUnk_08062048[0x100];
extern u8 gUnk_08062148[0x100];
extern u8 gUnk_08062248[0x100];
extern u8 gUnk_08062348[0x80];
extern u8 gUnk_080623C8[0x400];
extern u8 gUnk_080627C8[0x80];
extern u8 gUnk_08062848[0x80];
extern u8 gUnk_080628C8[0x200];
extern u8 gUnk_08062AC8[0x20];
extern u8 gUnk_08062AE8[0x200];
extern u8 gUnk_08062CE8[0x200];
extern u8 gUnk_08062EE8[0x200];
extern u8 gUnk_080630E8[0x80];
extern u8 gUnk_08063168[0x200];
extern u8 gUnk_08063368[0x80];
extern u8 gUnk_080633E8[0x200];
extern u8 gUnk_080635E8[0x80];
extern u8 gUnk_08063868[0x200];
extern u8 gUnk_08063A68[0x80];
extern u8 gUnk_08063AE8[0x100];
extern u8 gUnk_08063BE8[0x400];
extern u8 gUnk_08063FE8[0x80];
extern u8 gUnk_08064068[0x400];
extern u8 gUnk_08064468[0x400];
extern u8 gUnk_08064A68[0x200];
extern u8 gUnk_08064C68[0x200];
extern u8 gUnk_08064E68[0x100];
extern u8 gUnk_08064F68[0x200];
extern u8 gUnk_08065168[0x200];
extern u8 gUnk_08065368[0x200];
extern u8 gUnk_08065568[0x200];
extern u8 gUnk_08065768[0x20];
extern u8 gUnk_08065788[0x20];
extern u8 gUnk_080657A8[0x20];
extern u8 gUnk_08077E28[0x20];
extern u8 gUnk_08077E48[0x20];
extern u8 gUnk_08078308[0x20];
extern u8 gUnk_08078328[0x20];
extern u8 gUnk_08078348[0x20];
extern u8 gUnk_08078368[0x20];
extern u8 gUnk_08078388[0x20];
extern u8 gUnk_080783A8[0x20];
extern u8 gUnk_080783C8[0x20];
extern u8 gUnk_080783E8[0x20];
extern u8 gUnk_08078408[0x20];
extern u8 gUnk_08078428[0x20];
extern u8 gUnk_08078448[0x20];
extern u8 gUnk_08078468[0x20];
extern u8 gUnk_08078488[0x20];
extern u8 gUnk_080784A8[0x20];
extern u8 gUnk_080784C8[0x20];
extern u8 gUnk_080784E8[0x20];
extern u8 gUnk_08078568[0x20];
extern u8 gUnk_08078588[0x20];
extern u8 gUnk_080785C8[0x20];
extern u8 gUnk_08078608[0x20];
extern u8 gUnk_080786A8[0x20];
extern u8 gUnk_08078748[0x20];
extern u8 gUnk_08078788[0x20];
extern u8 gUnk_080787A8[0x20];
extern u8 gUnk_080787C8[0x20];
extern u8 gUnk_080787E8[0x20];
extern u8 gUnk_08078848[0x20];
extern u8 gUnk_08078868[0x20];
extern u8 gUnk_08078888[0x20];
extern u8 gUnk_080788A8[0x20];
extern u8 gUnk_080788C8[0x20];
extern u8 gUnk_080788E8[0x20];
extern u8 gUnk_08078908[0x20];
extern u8 gUnk_08078948[0x20];
extern u8 gUnk_08078968[0x20];
extern u8 gUnk_08078988[0x20];
extern u8 gUnk_080789A8[0x20];
extern u8 gUnk_080789E8[0x20];
extern u8 gUnk_08078A28[0x20];
extern u8 gUnk_08078A48[0x20];
extern u8 gUnk_08078A68[0x20];
extern u8 gUnk_080A4888[0x800];
extern u8 gUnk_080A5888[0x800];
extern void *gUnk_0818B800[6][7];

extern u8 gUnk_030007CC;
extern u8 gUnk_030034C0;
extern u8 gUnk_03003D16[][8];
extern u8 gUnk_03003DD6[][8];
extern u8 gUnk_03003E96[][8];
extern u8 gUnk_03003F56[][8];
extern u8 *gUnk_03004658;
extern u8 gUnk_03004D9C;
extern u8 gUnk_08064868[0x200];
extern const u8 gUnk_08078508[0x20];
extern u8 gUnk_08078628[0x20];
extern u8 gUnk_08078728[0x20];
extern u8 gUnk_08078768[0x20];
extern u8 gUnk_080789C8[0x20];
extern u8 gUnk_080B9468[0x200];
extern u8 gUnk_080D8C30[6][0x40];
extern const u8 gUnk_081166F8[4][4];
extern const u8 gUnk_08116708[8][4];
extern const u16 gUnk_08116728[8][2];
extern const u8 gUnk_08116748[7][8];
extern const u8 gUnk_08116780[8][0x20];
extern const u8 gUnk_08116880[8];
extern const s8 gUnk_08116888[6][2];
extern const u8 gUnk_081168DC[6];
extern const u8 gUnk_081168E2[4];
extern const u8 gUnk_081169F9[3][3];
extern const u8 gUnk_08116A02[4][5];
extern u8 gUnk_08116A46[4][2];
extern u16 gUnk_08116A4E[4][4];
extern const u8 gUnk_08116A6E[4][6];
extern const u8 gUnk_08116A86[5][6];
extern const u8 gUnk_08116AA4[3];
extern const u8 gUnk_08116AA7[3];
extern const u8 gUnk_0811710A[6];
extern u16 gUnk_08117110[8];
extern const void *gUnk_0818B9F8[];
extern u32 gUnk_082EAF8C;
extern u32 gUnk_082EB488;
extern u32 gUnk_082EB5B8;
extern u32 gUnk_082EBB20;
extern u32 gUnk_082EBC68;
extern u32 gUnk_082EC1A4;
extern u32 gUnk_082EC2E4;
extern u32 gUnk_082EC7C8;
extern u32 gUnk_082EC8F4;
extern u32 gUnk_082ECD74;
extern u32 gUnk_083128F8;
extern u32 gUnk_08312A58;
extern u32 gUnk_08312B70;
extern u32 gUnk_08312BD8;
extern u32 gUnk_08313C34;
extern u32 gUnk_08313F24;
extern u32 gUnk_083141F0;
extern u32 gUnk_083142EC;
extern u32 gUnk_083155C4;
extern struct Unk_03004C08 gUnk_03004C08;
extern struct Unk_0803D4AC gUnk_081168E8[];
extern struct Unk_0803D4AC gUnk_03003620;
extern struct Unk_030034B0 gUnk_030034B0;

/* World-map node tables (ROM).
 * gWorldMapNodes[world - 1][node][.] : 5-byte per-node records, 40 per world.
 *   byte 1 is the negated BG2 rotation angle at which that node faces the camera
 *   (proved dynamically: docs/dynamic-analysis/scripts/prove-worldmap-selected-vision.mjs).
 *   The remaining 4 bytes are not yet identified.
 * gWorldMapVisionNode[world - 1][vision - 1] : the node record a vision lives on. */
extern u8 gWorldMapNodes[6][40][5];
extern u8 gWorldMapVisionNode[6][8];
/* gVisionUnlockMask[.][stage] : bitmask of the visions a world offers at world-map
 * progress stage `stage` (gUnk_030034B0.unk7_4, 0..6); bit N set = vision N+1 exists.
 * 0x0811765C, six identical 7-byte rows { 02 04 18 60 60 80 80 } ending at 0x08117686
 * (the next 7-byte row is the zero padding that follows).
 * FindNextUnlockedVision indexes the row with gUnk_03004C20.world, NOT world - 1, so
 * world 6 reads the zero row just past the table; the outer bound is deliberately left
 * incomplete to record that the original code reads one row high. Because every row is
 * identical the off-by-one is invisible for worlds 1..5, which is presumably why it
 * survived. */
extern u8 gVisionUnlockMask[][7];
extern u8 gUnk_030007C4;
extern u16 gUnk_030052B8;
extern u16 gUnk_08057C70;
extern u16 gBlendModeTable[]; /* BLDCNT blend-config lookup table (0x08057B4C) */
extern u8 gUnk_0805D9E8[0x200];
extern u8 gUnk_0805DBE8[0x300];
extern u8 gUnk_0805DEE8[0x200];
extern u8 gUnk_0805E0E8[0x200];
extern u8 gUnk_0805E2E8[0x400];
extern u8 gUnk_0805E6E8[0x400];
extern u8 gUnk_0805EAE8[0x200];
extern u8 gUnk_08061888[0x100];
extern u8 gUnk_08061988[0x80];
extern u8 gUnk_08061A08[0x20];
extern u8 gUnk_08063668[0x200];
extern u8 gUnk_08077E68[0x20];
extern u8 gUnk_08077E88[0x20];
extern u8 gUnk_08077EA8[0x20];
extern u8 gUnk_08077EC8[0x20];
extern u8 gUnk_08077EE8[0x20];
extern u8 gUnk_08077F08[0x20];
extern u8 gUnk_08077F28[0x20];
extern u8 gUnk_08077F48[0x20];
extern u8 gUnk_08077F68[0x20];
extern u8 gUnk_08077F88[0x20];
extern u8 gUnk_08077FA8[0x20];
extern u8 gUnk_08077FC8[0x20];
extern u8 gUnk_08077FE8[0x20];
extern u8 gUnk_08078008[0x20];
extern u8 gUnk_08078028[0x20];
extern u8 gUnk_08078048[0x20];
extern u8 gUnk_08078068[0x20];
extern u8 gUnk_08078088[0x20];
extern u8 gUnk_080780A8[0x20];
extern u8 gUnk_080780C8[0x20];
extern u8 gUnk_080780E8[0x20];
extern u8 gUnk_08078108[0x20];
extern u8 gUnk_08078128[0x20];
extern u8 gUnk_08078148[0x20];
extern u8 gUnk_08078168[0x20];
extern u8 gUnk_08078188[0x20];
extern u8 gUnk_080781A8[0x20];
extern u8 gUnk_080781C8[0x20];
extern u8 gUnk_080781E8[0x20];
extern u8 gUnk_08078208[0x20];
extern u8 gUnk_08078228[0x20];
extern u8 gUnk_08078248[0x20];
extern u8 gUnk_08078268[0x20];
extern u8 gUnk_08078288[0x20];
extern u8 gUnk_080782A8[0x20];
extern u8 gUnk_080782C8[0x20];
extern u8 gUnk_080782E8[0x20];
extern u8 gUnk_08078528[0x20];
extern u8 gUnk_08078548[0x20];
extern u8 gUnk_080785A8[0x20];
extern u8 gUnk_080785E8[0x20];
extern u8 gUnk_08078648[0x20];
extern u8 gUnk_08078668[0x20];
extern u8 gUnk_08078688[0x20];
extern u8 gUnk_080786C8[0x20];
extern u8 gUnk_080786E8[0x20];
extern u8 gUnk_08078708[0x20];
extern u8 gUnk_080A5088[0x800];
/* Per-level animated-tile pointer table (kleod-canonical), indexed
 * [world-1][level]; the ->unk3C etc members point at frame data streamed
 * into OBJ VRAM by the level loaders. */
struct Unk_08189A24 {
    u8 pad0[0x3C - 0x0];
    void ***unk3C;
    u8 pad40[0x60 - 0x40];
    void ***unk60;
    u8 pad64[0x6C - 0x64];
    void ***unk6C;
    u8 pad70[0x78 - 0x70];
    void ***unk78;
    u8 pad7C[0x90 - 0x7C];
    void ***unk90;
};
extern struct Unk_08189A24 *gUnk_08189A24[6][9];
extern struct Unk_03005294_03005418 *gUnk_03005418;
extern struct Unk_03005294_03005418 *gUnk_03005294;
extern u16 gUnk_080D927C[];
extern u16 gUnk_080D947C[];
/* Stream-command OAM sprite groups (0x08189F04), indexed [group][entry].
 * A group is 16 entries of 12 bytes (0xC0 apart); the entry whose unk0 reads
 * 0xFFFF terminates the group. Each live entry is one SetupOAMSprite call, so
 * the member roles line up with that function's parameters:
 *   unk0/unk2 -> x/y, unk4 -> arg4, unk9 -> arg5, unk5 -> arg6, unk6 -> arg7,
 *   unk7 -> arg1 (the same role gUnk_080E2B64::unk28 fills),
 *   unk8 -> arg8 (the sprite kind, the same role unk29 fills). */
struct Unk_08189F04 {
    /* 0x0 */ u16 unk0;
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u8 unk4;
    /* 0x5 */ u8 unk5;
    /* 0x6 */ u8 unk6;
    /* 0x7 */ u8 unk7;
    /* 0x8 */ u8 unk8;
    /* 0x9 */ u8 unk9;
    /* 0xA */ u8 padA[0xC - 0xA];
}; /* size = 0xC */
extern struct Unk_08189F04 gUnk_08189F04[][16];
#endif /* GUARD_STRUCTS_VARIABLES_H */
