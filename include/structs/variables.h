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
    /* 0x10 */ u16 unk10;
    /* 0x12 */ u16 unk12;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u16 unk16;
    /* 0x18 */ u8 unk18;
    /* 0x19 */ u8 pad19[0x1C - 0x19];

    /* 0x1C */ void *pVramBg1Tiles;
    /* 0x20 */ void *pVramBg1Tilemap;
    /* 0x24 */ u16 bg1HOfs;
    /* 0x26 */ u16 bg1VOfs;
    /* 0x28 */ u8 pad28[0x2C - 0x28];
    /* 0x2C */ u16 unk2C;
    /* 0x2E */ u16 unk2E;
    /* 0x30 */ u16 unk30;
    /* 0x32 */ u16 unk32;
    /* 0x34 */ u8 unk34;
    /* 0x35 */ u8 pad35[0x38 - 0x35];

    /* 0x38 */ void *pVramBg2Tiles;
    /* 0x3C */ void *pVramBg2Tilemap;
    /* 0x40 */ u16 bg2HOfs;
    /* 0x42 */ u16 bg2VOfs;
    /* 0x44 */ u16 unk44;
    /* 0x46 */ u16 unk46;
    /* 0x48 */ u16 unk48;
    /* 0x4A */ u16 unk4A;
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
    /* 0x00 */ s32 unk0; /* per-scene frame counter */
    /* 0x04 */ s32 unk4; /* global frame counter */
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

/* Per-frame globals used by VBlank callbacks. */
#define gUnk_03003420 (*(u8 *)0x03003420)
#define gUnk_03005498 (*(u8 *)0x03005498)
#define gUnk_030007D8 (*(u8 *)0x030007D8)

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
    /* 0x0B */ u8 unkB;
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
    /* 0x12 */ u8 pad12[0x1C - 0x12];
}; /* size = 0x1C */
extern struct Unk_03002920 gUnk_03002920[];

/* Active entity count (number of slots in gUnk_03002920 to iterate). */
extern u8 gUnk_03005428;

/* Per-frame edge-detected key state. */
extern u16 gNewKeys;
extern u16 gHeldKeys;

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
    u8 pad0[0x8 - 0x0];
    u8 unk8[2][8];
};
extern volatile struct Unk_03004670 *gUnk_03004670;
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
    /* 0x00 */ void (*current[10])(void);  // current callbacks
    /* 0x28 */ void (*next[10])(void);     // next callbacks
    /* 0x50 */ void (*previous[10])(void); // previous callbacks
    /* 0x78 */ u8 currentCount;            // current callback count
    /* 0x79 */ u8 nextCount;               // next callback count
    /* 0x7A */ u8 previousCount;           // previous callback count
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

/* Room/scroll bounds. */
struct Unk_03005468 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
};
extern struct Unk_03005468 gUnk_03005468;

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

/* World-4 wobble state at 0x03005400 (partial). */
struct Unk_03005400 {
    /* 0x0 */ u8 pad0[0x2 - 0x0];
    /* 0x2 */ u16 unk2;
    /* 0x4 */ u8 pad4[0x6 - 0x4];
    /* 0x6 */ u16 unk6;
    /* 0x8 */ u8 pad8[0xA - 0x8];
    /* 0xA */ u8 unkA;
    /* 0xB */ u8 unkB;
    /* 0xC */ u8 unkC;
    /* 0xD */ u8 unkD;
    /* 0xE_0 */ u8 unkE_0 : 1;
    /* 0xE_1 */ u8 unkE_1 : 1;
    /* 0xE_2 */ u8 unkE_2 : 1;
    /* 0xE_3 */ u8 unkE_3 : 4;
    /* 0xE_7 */ u8 unkE_7 : 1;
    /* 0x0F */ u8 padF[0x14 - 0xF];
    /* 0x14 */ u8 unk14;
    /* 0x15 */ u8 pad15[0x16 - 0x15];
    /* 0x16 */ s8 unk16;
    /* 0x17 */ u8 pad17[0x18 - 0x17];
};
extern struct Unk_03005400 gUnk_03005400;

/* Affine reference points at 0x03005440. */
struct Unk_03005440 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
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
extern u16 gUnk_03005474;

/* Game-state struct at 0x03005220 (mirrored from kleod, full layout). */
struct Unk_03005220 {
    /* 0x00_0 */ u32 unk0_0 : 2;
    /* 0x00_2 */ u32 unk0_2 : 3;
    /* 0x00_5 */ u32 unk0_5 : 7;
    /* 0x01_4 */ u32 unk1_4 : 3;
    /* 0x01_7 */ u32 unk1_7 : 8;
    /* 0x02_7 */ u32 unk2_7 : 6;
    /* 0x03_5 */ u32 unk3_5 : 1;
    /* 0x03_6 */ u32 unk3_6 : 1;
    /* 0x04 */ u32 unk4;
    /* 0x08 */ u32 unk8;
    /* 0x0C */ u32 unkC;
    /* 0x10 */ u32 unk10;
    /* 0x14 */ u16 unk14;
    /* 0x16 */ u8 pad16[0x1A - 0x16];
    /* 0x1A */ u16 unk1A;
    /* 0x1C */ u16 unk1C;
    /* 0x1E */ u8 pad1E[0x2E - 0x1E];
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ s8 unk2F;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 pad32[0x35 - 0x32];
    /* 0x35 */ u8 unk35;
    /* 0x36 */ u8 unk36;
    /* 0x37 */ u8 unk37;
    /* 0x38 */ u8 pad38[0x3F - 0x38];
    /* 0x3F */ u8 unk3F;
    /* 0x40 */ u8 pad40[0x46 - 0x40];
    /* 0x46 */ u8 unk46;
    /* 0x47 */ u8 pad47[0x4C - 0x47];
    /* 0x4C */ u8 unk4C;
    /* 0x4D */ u8 unk4D;
    /* 0x4E */ u8 unk4E;
    /* 0x4F */ u8 unk4F;
    /* 0x50 */ u8 pad50[0x58 - 0x50];
    /* 0x58 */ u8 unk58;
    /* 0x59 */ u8 pad59[0x5E - 0x59];
    /* 0x5E */ u8 unk5E;
    /* 0x5F */ u8 unk5F;
    /* 0x60 */ u16 unk60;
    /* 0x62 */ u8 pad62[0x64 - 0x62];
}; /* size = 0x64 */
extern struct Unk_03005220 gUnk_03005220;

/* Level-config view struct at 0x03005284, accessed via pointer.
 * Holds per-level entity/state replay values. */
struct Unk_03005284 {
    /* 0x00 */ u8 unk0;
    /* 0x01 */ u8 unk1;
    /* 0x02 */ u8 unk2;
    /* 0x03 */ u8 pad3[0x5 - 0x3];
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
};
extern struct Unk_03005284 *gUnk_03005284;

/* ROM data tables (level config / room positions / boss positions). */
struct Unk_080D821C {
    u8 pad0[0x8 - 0x0];
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
struct Unk_030051CC {
    s16 unk0;
    s16 unk2;
};
extern struct Unk_030051CC gUnk_030051CC;

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
extern void gUnk_03003650;

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
        /* 0x3_5 */ u8 vFlip : 1;
        /* 0x3_6 */ u8 size : 2;
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

/* Rotation/scale matrix source table for OAM (halfwords at 0x03004680). */
struct Unk_03004680 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
}; /* size = 0x8 */
extern struct Unk_03004680 gUnk_03004680[];

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

struct Unk_0818B8E0 {
    u8 pad0[0x4 - 0x0];
    u16 *unk4;
};
extern struct Unk_0818B8E0 *gUnk_030051DC;
extern struct Unk_0818B8E0 *gUnk_0818B8E0[6][9];

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
    u8 pad4[0xE - 0x4];
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

#endif /* GUARD_STRUCTS_VARIABLES_H */
