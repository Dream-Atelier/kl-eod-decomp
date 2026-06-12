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

#endif /* GUARD_STRUCTS_VARIABLES_H */
