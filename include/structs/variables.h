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
#define gUnk_03004C20 (*(struct Unk_03004C20 *)0x03004C20)

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
struct Unk_03003510 {
    /* 0x00 */ IntrFunc unk0[3];
    /* 0x0C */ s32 unkC;
    /* 0x10 */ IntrFunc unk10;
    /* 0x14 */ u8 pad14[0x28 - 0x14];
    /* 0x28 */ IntrFunc unk28[3];
    /* 0x34 */ void *unk34;
    /* 0x38 */ void *unk38;
    /* 0x3C */ u32 unk3C;
    /* 0x40 */ IntrFunc unk40;
    /* 0x44 */ u32 unk44;
    /* 0x48 */ u8 pad48[0x50 - 0x48];
    /* 0x50 */ IntrFunc unk50[1];
    /* 0x54 */ u8 pad54[0x78 - 0x54];
    /* 0x78 */ u8 unk78;
    /* 0x79 */ u8 unk79;
    /* 0x7A */ u8 unk7A;
    /* 0x7B */ u8 pad7B[0x7C - 0x7B];
};
extern struct Unk_03003510 gUnk_03003510;

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

/* Game-state struct at 0x03005220 (partial — fields used by current ports). */
struct Unk_03005220 {
    /* 0x00 */ u8 pad0[0x2E - 0x00];
    /* 0x2E */ u8 unk2E;
    /* 0x2F */ s8 unk2F;
    /* 0x30 */ u8 unk30;
    /* 0x31 */ u8 unk31;
    /* 0x32 */ u8 pad32[0x46 - 0x32];
    /* 0x46 */ u8 unk46;
    /* 0x47 */ u8 pad47[0x64 - 0x47];
};
extern struct Unk_03005220 gUnk_03005220;

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

struct Unk_0818B8E0 {
    u8 pad0[0x4 - 0x0];
    u16 *unk4;
};
extern struct Unk_0818B8E0 *gUnk_030051DC;

/* Text-scroll Bresenham accumulator. */
extern s32 gUnk_030034DC;

/* sub_0800BEF0 (UpdateTextScroll) parameter struct. */
struct Unk_0800BEF0 {
    u16 unk0;
    u16 unk2;
    u16 unk4;
    u16 unk6;
    s8 unk8;
};

#endif /* GUARD_STRUCTS_VARIABLES_H */
