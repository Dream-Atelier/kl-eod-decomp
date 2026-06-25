#include "global.h"
#include "globals.h"
#include "include_asm.h"
#include "data/trig.h"
#include "structs/variables.h"

extern s16 MultiplyQ8(s16 a, s16 b);
extern s16 ReciprocalQ8(s16 a);
extern void TransitionGameplayInit(void);
extern void ComputeScrollLimits(void);
extern s32 Abs(s32 n);
extern void InitScrollState(void);
extern void InitOamEntries(void);
extern void InitLevelGameplay(u32 arg0); /* InitLevelGameplay */
extern void ReadKeyInput(void); /* ReadKeyInput == kleod InputHandler_Normal */
extern void InitGameplayFromWorldMap(void);
extern void VBlankCallback_Dialog(void); /* VBlankCallback_Dialog */
extern void UpdateWorldMapLogic(void); /* sub_08048028 (off-by-2 in our split) */
extern void TransitionInitLevelMusic(void); /* sub_080242C0 */
extern void ScrollBGColumnLoad(u8 arg0); /* ScrollBGColumnLoad */
/* InitLevelBG (InitLevelBG) extras */
extern void m4aSoundVSyncOff(void);
extern void m4aMPlayAllStop(void);
extern void *DecompressAlloc(void *src);
extern void Decompress(void *dest, void *src);
extern void DecompressDma(void *src, void *dest, u16 size);
extern void *thunk_HeapAlloc(u32 size, u32 flags);
extern void CheckTileCollisionRect(u8); /* CheckTileCollisionRect */
extern void UpdateHUDTimerAndLives(void); /* UpdateHUDTimerAndLives */
extern void VBlankHandler(void); /* sub_080009D8 — installed for non-cutscene levels */
extern void VBlankDmaTransfer(void); /* sub_08000CE0 — installed for cutscene levels */
extern void InitLevelFromROMTable(void); /* InitLevelFromROMTable */

INCLUDE_ASM("asm/nonmatchings/engine", VBlankHandler_ModeA);
INCLUDE_ASM("asm/nonmatchings/engine", VBlankHandler_ModeB);
INCLUDE_ASM("asm/nonmatchings/engine", VBlankDmaTransfer);
INCLUDE_ASM("asm/nonmatchings/engine", VBlankHandler_OamOnly);
INCLUDE_ASM("asm/nonmatchings/engine", VBlankHandler_OamOnlyAlt);
INCLUDE_ASM("asm/nonmatchings/engine", VBlankHandler_WithWindowScroll);
/**
 * UpdateFadeEffect: applies brightness fade using REG_BLDY.
 *
 * Reads REG_VCOUNT and entity brightness value, computes fade level
 * via sub_08051A0C, then writes to REG_BLDY if within valid range (<=16).
 */
void UpdateFadeEffect(void) {
    vu8 *vcount_reg = &REG_VCOUNT_L;
    u8 *entity = gEntityArray;
    u8 fade = sub_08051A0C(*vcount_reg, entity[0x08]);

    if (fade <= 16) {
        REG_BLDALPHA = ((u32)fade << 8) | fade;
    }
}
INCLUDE_ASM("asm/nonmatchings/engine", HBlankScrollUpdate);
INCLUDE_ASM("asm/nonmatchings/engine", UpdateAffineBGParams);
/**
 * UpdateWindowCircleEffect: compute circle window bounds for iris transition.
 *
 * Reads the current scanline from REG_VCOUNT, computes left/right bounds
 * of a circular window using BiosSquareRoot, and writes to REG_WIN0H.
 * Used for iris-in/out screen transitions.
 */
void UpdateWindowCircleEffect(void) {
    u16 vcount = REG_VCOUNT;
    u32 radius = gSceneScriptState;
    u32 half_r = radius >> 1;
    s32 y = vcount - half_r;
    s32 y_adj = y + 12;
    s32 x_span;
    u32 val;
    u32 sqr;
    u32 hw;
    x_span = 0xE4 - y;
    x_span -= radius;
    val = (u32)(x_span * y_adj) << 2;
    sqr = BiosSquareRoot(val);
    hw = (u8)sqr >> 1;
    if (hw <= 0x78) {
        REG_WIN1H = ((0x78 - hw) << 8) | (hw + 0x78);
    } else {
        REG_WIN1H = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/engine", UpdateBGScrollWithWave);
INCLUDE_ASM("asm/nonmatchings/engine", WaitVBlankAndClearMosaic);
INCLUDE_ASM("asm/nonmatchings/engine", AcknowledgeInterrupt);
void InitLevelBG(void) {
    s32 sp0;
    s32 temp_r1;
    u8 temp_r2;
    u32 temp_r3;
    u32 temp_r4;
    u32 var_r4;

    sp0 = 0;
    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();
    m4aMPlayAllStop();
    gUnk_03004660 = 0;

    if (gUnk_03004C20.level == 0) {
        sp0 = 1;
        for (var_r4 = 0; var_r4 < 8; var_r4++) {
            if (!(gUnk_03004670->unk8[gUnk_03004C20.world - 1][var_r4] & 0x80)) {
                sp0 = 0;
            }
        }
    }

    if (gUnk_03003410.unk9 == 0) {
        if ((gUnk_03004C20.level == 8) || (gUnk_03004C20.level == 0)) {
            gUnk_03005290 = DecompressAlloc(gUnk_0818B7AC[((gUnk_03004C20.level >> 3) * 6) + (gUnk_03004C20.world - 1)]) + 4;
        }

        gUnk_03004790.pBufBg2Tiles = thunk_HeapAlloc(*gUnk_08189034[gUnk_03004C20.world - 1][gUnk_03004C20.level][2] & 0x7FFFFFFF, 0);
        gUnk_03004790.pBufBg2Tilemap
            = thunk_HeapAlloc(*gUnk_081892BC[gUnk_03004C20.world - 1][gUnk_03004C20.level][2] & 0x7FFFFFFF, 0);
        if ((sp0 == 1) && (gUnk_03004C20.level == 0)) {
            gUnk_03004790.pBufBg1Tiles = thunk_HeapAlloc(*gUnk_0818955C[gUnk_03004C20.world - 1] & 0x7FFFFFFF, 0);
            gUnk_03004790.pBufBg1Tilemap = thunk_HeapAlloc(*gUnk_08189574[gUnk_03004C20.world - 1] & 0x7FFFFFFF, 0);
        } else {
            gUnk_03004790.pBufBg1Tiles
                = thunk_HeapAlloc(*gUnk_08189034[gUnk_03004C20.world - 1][gUnk_03004C20.level][1] & 0x7FFFFFFF, 0);
            gUnk_03004790.pBufBg1Tilemap
                = thunk_HeapAlloc(*gUnk_081892BC[gUnk_03004C20.world - 1][gUnk_03004C20.level][1] & 0x7FFFFFFF, 0);
        }
        gUnk_03004790.pBufBg0Tiles = thunk_HeapAlloc(*gUnk_08189034[gUnk_03004C20.world - 1][gUnk_03004C20.level][0] & 0x7FFFFFFF, 0);
        gUnk_03004790.pBufBg0Tilemap
            = thunk_HeapAlloc(*gUnk_081892BC[gUnk_03004C20.world - 1][gUnk_03004C20.level][0] & 0x7FFFFFFF, 0);

        Decompress(gUnk_03004790.pBufBg0Tiles, gUnk_08189034[gUnk_03004C20.world - 1][gUnk_03004C20.level][0]);
        Decompress(gUnk_03004790.pBufBg0Tilemap, gUnk_081892BC[gUnk_03004C20.world - 1][gUnk_03004C20.level][0]);
        if ((sp0 == 1) && (gUnk_03004C20.level == 0)) {
            Decompress(gUnk_03004790.pBufBg1Tiles, gUnk_0818955C[gUnk_03004C20.world - 1]);
            Decompress(gUnk_03004790.pBufBg1Tilemap, gUnk_08189574[gUnk_03004C20.world - 1]);
        } else {
            Decompress(gUnk_03004790.pBufBg1Tiles, gUnk_08189034[gUnk_03004C20.world - 1][gUnk_03004C20.level][1]);
            Decompress(gUnk_03004790.pBufBg1Tilemap, gUnk_081892BC[gUnk_03004C20.world - 1][gUnk_03004C20.level][1]);
        }
        Decompress(gUnk_03004790.pBufBg2Tiles, gUnk_08189034[gUnk_03004C20.world - 1][gUnk_03004C20.level][2]);
        Decompress(gUnk_03004790.pBufBg2Tilemap, gUnk_081892BC[gUnk_03004C20.world - 1][gUnk_03004C20.level][2]);

        gUnk_03004790.pBufBg0Tiles = (u8 *)gUnk_03004790.pBufBg0Tiles + 4;
        gUnk_03004790.pBufBg0Tilemap += 2;
        gUnk_03004790.pBufBg1Tiles = (u8 *)gUnk_03004790.pBufBg1Tiles + 4;
        gUnk_03004790.pBufBg1Tilemap += 2;
        gUnk_03004790.pBufBg2Tiles = (u8 *)gUnk_03004790.pBufBg2Tiles + 4;
        gUnk_03004790.pBufBg2Tilemap += 4;
    }

    if ((gUnk_03004C20.level == 0) && (sp0 == 1)) {
        DecompressDma(gUnk_08189544[gUnk_03004C20.world - 1], (void *)0x05000000, 0x200);
    } else {
        DecompressDma(gUnk_08188F5C[gUnk_03004C20.world - 1][gUnk_03004C20.level], (void *)0x05000000, 0x200);
    }

    gUnk_03003430.bg0HOfs = 0;
    gUnk_03003430.bg0VOfs = 0;
    gUnk_03003430.unk14 = 0;
    gUnk_03003430.pVramBg0Tiles = (void *)VRAM;
    gUnk_03003430.pVramBg0Tilemap = (void *)(VRAM + 0xE000);
    gUnk_03003430.unk10 = gUnk_08051C76[gUnk_03004C20.world - 1][gUnk_03004C20.level][0];
    gUnk_03003430.unk12 = gUnk_08051DBA[gUnk_03004C20.world - 1][gUnk_03004C20.level][0];
    gUnk_03003430.unk16 = gUnk_08051EFE[gUnk_03004C20.world - 1][gUnk_03004C20.level][0];
    gUnk_03003430.unk18 = gUnk_08052042[gUnk_03004C20.world - 1][gUnk_03004C20.level][0];
    gUnk_03003430.bg1HOfs = 0;
    gUnk_03003430.bg1VOfs = 0;
    gUnk_03003430.unk30 = 0;
    gUnk_03003430.pVramBg1Tiles = (void *)(VRAM + 0x4000);
    gUnk_03003430.pVramBg1Tilemap = (void *)(VRAM + 0xE800);

    if ((gUnk_03004C20.level == 0) && (sp0 == 1)) {
        gUnk_03003430.bg1MapWidth = gUnk_0805265A[gUnk_03004C20.world - 1];
        gUnk_03003430.unk2E = gUnk_08052666[gUnk_03004C20.world - 1];
        gUnk_03003430.unk32 = gUnk_08052672[gUnk_03004C20.world - 1];
        gUnk_03003430.unk34 = gUnk_0805267E[gUnk_03004C20.world - 1];
    } else {
        gUnk_03003430.bg1MapWidth = gUnk_08051C76[gUnk_03004C20.world - 1][gUnk_03004C20.level][1];
        gUnk_03003430.unk2E = gUnk_08051DBA[gUnk_03004C20.world - 1][gUnk_03004C20.level][1];
        gUnk_03003430.unk32 = gUnk_08051EFE[gUnk_03004C20.world - 1][gUnk_03004C20.level][1];
        gUnk_03003430.unk34 = gUnk_08052042[gUnk_03004C20.world - 1][gUnk_03004C20.level][1];
    }

    gUnk_03003430.unk4C = 0;
    gUnk_03003430.pVramBg2Tiles = (void *)(VRAM + 0x8000);
    gUnk_03003430.pVramBg2Tilemap = (void *)(VRAM + 0xF000);
    gUnk_03003430.bg2MapWidth = gUnk_08051C76[gUnk_03004C20.world - 1][gUnk_03004C20.level][2];
    gUnk_03003430.bg2MapHeight = gUnk_08051DBA[gUnk_03004C20.world - 1][gUnk_03004C20.level][2];
    gUnk_03003430.unk4E = gUnk_08051EFE[gUnk_03004C20.world - 1][gUnk_03004C20.level][2];
    gUnk_03003430.unk50 = gUnk_08052042[gUnk_03004C20.world - 1][gUnk_03004C20.level][2];
    gUnk_030052A0 = 0xFE;

    if (gUnk_03004C20.level >= 1 && gUnk_03004C20.level <= 7) {
        if (gUnk_03004C20.room == 0xFF) {
            for (gUnk_03004C20.room = 1; gUnk_03004C20.room < 6; gUnk_03004C20.room++) {
                gUnk_03005468.unk0 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk0;
                gUnk_03005468.unk2 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk2;
                gUnk_03005468.unk4 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk4;
                gUnk_03005468.unk6 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk6;

                temp_r4 = (gUnk_03004C20.unk8 >> ((gUnk_03004C20.room - 1) * 2)) & 3;
                temp_r3 = (gUnk_03005284->unk16 >> ((gUnk_03004C20.room - 1) * 2)) & 3;
                if (temp_r4 != temp_r3) {
                    CheckTileCollisionRect(((u8)(temp_r3 + 4) - temp_r4) & 3);
                    temp_r2 = (gUnk_03004C20.room - 1) * 2;
                    temp_r1 = ((gUnk_03004C20.unk8 >> temp_r2) + 1) & 3;
                    gUnk_03004C20.unk8 = (gUnk_03004C20.unk8 & ~(3 << temp_r2)) | (temp_r1 << temp_r2);
                }
            }

            gUnk_03004C20.room = 0xFF;
        } else if (gUnk_03003410.unk9 == 1) {
            temp_r4 = (gUnk_03004C20.unk8 >> ((gUnk_03004C20.room - 1) * 2)) & 3;
            temp_r3 = (gUnk_03005284->unk16 >> ((gUnk_03004C20.room - 1) * 2)) & 3;
            if (temp_r4 != temp_r3) {
                CheckTileCollisionRect(((u8)(temp_r3 + 4) - temp_r4) & 3);
                gUnk_03004C20.unk8 = gUnk_03005284->unk16;
            }
        }
    } else {
        gUnk_03005284->unk16 = 0;
        gUnk_03004C20.unk8 = 0;
    }
    InitLevelFromROMTable();

    DmaCopy16Wait(3, gUnk_03004790.pBufBg0Tiles, gUnk_03003430.pVramBg0Tiles, gUnk_03003430.unk18 * gUnk_03003430.unk16);
    DmaCopy16Wait(3, gUnk_03004790.pBufBg1Tiles, gUnk_03003430.pVramBg1Tiles, gUnk_03003430.unk34 * gUnk_03003430.unk32);
    DmaCopy16Wait(3, gUnk_03004790.pBufBg2Tiles, gUnk_03003430.pVramBg2Tiles, gUnk_03003430.unk50 * gUnk_03003430.unk4E);

    for (var_r4 = 0; var_r4 < 0x400; var_r4++) {
        gBgTilemapBufs[1][var_r4] = gUnk_03004790.pBufBg1Tilemap[var_r4];
        gBgTilemapBufs[0][var_r4] = gUnk_03004790.pBufBg0Tilemap[var_r4];
    }

    DmaCopy16Wait(3, &gBgTilemapBufs[0], gUnk_03003430.pVramBg0Tilemap, 0x800);
    DmaCopy16Wait(3, &gBgTilemapBufs[1], gUnk_03003430.pVramBg1Tilemap, 0x800);

    REG_BG0CNT = gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][0] | BGCNT_PRIORITY(3) | BGCNT_SCREENBASE(28)
        | BGCNT_MOSAIC | BGCNT_CHARBASE(0);
    REG_BG1CNT = gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][1] | BGCNT_PRIORITY(2) | BGCNT_SCREENBASE(29)
        | BGCNT_MOSAIC | BGCNT_CHARBASE(1);

    REG_BG0HOFS = (gUnk_03003430.bg0HOfs >> 4) & 0x1FF;
    REG_BG0VOFS = (gUnk_03003430.bg0VOfs >> 7) & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;

    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));

    REG_BG2X_L = gUnk_03003430.bg2HOfs << 8;
    REG_BG2X_H = gUnk_03003430.bg2HOfs >> 0x10;
    REG_BG2Y_L = gUnk_03003430.bg2VOfs << 8;
    REG_BG2Y_H = gUnk_03003430.bg2VOfs >> 0x10;

    REG_BG2PA = gBg2PA;
    REG_BG2PB = gBg2PB;
    REG_BG2PC = gBg2PC;
    REG_BG2PD = gBg2PD;

    gBg2YMag = 0x100;
    gBg2XMag = 0x100;
    gBg2Alpha = 0;

    REG_WIN0H = WIN_RANGE(0, DISPLAY_WIDTH);
    REG_WIN0V = WIN_RANGE((s32)(DISPLAY_HEIGHT * 0.9f), DISPLAY_HEIGHT);
    REG_WININ = WININ_WIN0_CLR | WININ_WIN0_BG0;
    REG_WINOUT = WINOUT_WIN01_CLR | WINOUT_WIN01_OBJ | WINOUT_WIN01_BG_ALL;

    if ((gUnk_03004C20.world == 6) && ((gUnk_03004C20.level == 1) || (gUnk_03004C20.level == 3))) {
        REG_WIN1H = WIN_RANGE((s32)(DISPLAY_WIDTH * 2.f / 3.f), DISPLAY_WIDTH);
        REG_WIN1V = WIN_RANGE(0, (s32)(DISPLAY_HEIGHT * 0.1f));
        REG_WININ = WININ_WIN1_CLR | WININ_WIN1_BG0 | WININ_WIN0_CLR | WININ_WIN0_BG0;
        REG_WINOUT = WINOUT_WIN01_CLR | WINOUT_WIN01_OBJ | WINOUT_WIN01_BG_ALL;
        REG_DISPCNT = DISPCNT_WIN1_ON | DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_BG2_ON | DISPCNT_BG1_ON | DISPCNT_BG0_ON
            | DISPCNT_OBJ_1D_MAP | DISPCNT_MODE_1;
        gUnk_03004C20.unk10 = 1;
        UpdateHUDTimerAndLives();
    } else {
        REG_DISPCNT = DISPCNT_WIN0_ON | DISPCNT_OBJ_ON | DISPCNT_BG2_ON | DISPCNT_BG1_ON | DISPCNT_BG0_ON | DISPCNT_OBJ_1D_MAP
            | DISPCNT_MODE_1;
        gUnk_03004C20.unk10 = 0;
    }

    if (gUnk_03004C20.level != 8) {
        REG_BG2CNT = gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][2] | BGCNT_TXT512x256 | BGCNT_WRAP
            | BGCNT_SCREENBASE(30) | BGCNT_MOSAIC | BGCNT_CHARBASE(2) | BGCNT_PRIORITY(1);
        gIntrTable.vBlank = VBlankHandler;
    } else {
        var_r4 = (u32)REG_ADDR_BG2CNT;
        REG_BG2CNT = gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][2] | BGCNT_TXT256x512 | BGCNT_SCREENBASE(30)
            | BGCNT_MOSAIC | BGCNT_CHARBASE(2) | BGCNT_PRIORITY(1);
        gIntrTable.vBlank = VBlankDmaTransfer;
        gUnk_03004C20.room = 1;
    }

    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
}
struct ScrollBGLayer_Args {
    u16 unk0;
    u16 unk2;
};
void ScrollBGLayer(u8 arg0, struct ScrollBGLayer_Args arg1) {
    s32 temp_r1;
    u32 temp_r0_3;
    u32 temp_r0_7;
    u32 temp_r2;
    u32 var_r3;
    s32 var_r5;
    s32 var_r6;
    s32 var_r7;

    if (arg0 & 0x10) {
        gUnk_03003430.bg2HOfs += arg1.unk0;
        if (gUnk_03003430.bg2HOfs > (gUnk_03005468.unk4 - 0xF0)) {
            gUnk_03003430.bg2HOfs = gUnk_03005468.unk4 - 0xF0;
        }
        temp_r2 = gUnk_03003430.bg2HOfs >> 3;
        if (temp_r2 != gUnk_03003430.bg2StreamColumn) {
            gUnk_03003430.bg2StreamColumn = temp_r2;
            temp_r0_3 = temp_r2 + 0x1E;
            var_r7 = temp_r0_3 & 0x1F;
            var_r5 = temp_r0_3 % gUnk_03003430.bg2MapWidth;
            var_r6 = gUnk_03003430.bg2VOfs >> 3;
            for (var_r3 = 0; var_r3 < 0x15; var_r3++) {
                temp_r1 = var_r3 + var_r6;
                gUnk_03004DB0[((temp_r1 & 0x1F) << 5) + var_r7]
                    = gUnk_03004790.pBufBg2Tilemap[(temp_r1 * gUnk_03003430.bg2MapWidth) + var_r5];
            }
        }
    } else if (arg0 & 0x20) {
        gUnk_03003430.bg2HOfs += arg1.unk0;
        if (((s16)(gUnk_03003430.bg2HOfs - gUnk_03005468.unk0)) > -0x100u) {
            gUnk_03003430.bg2HOfs = gUnk_03005468.unk0;
        }
        temp_r2 = gUnk_03003430.bg2HOfs >> 3;
        if (temp_r2 != gUnk_03003430.bg2StreamColumn) {
            gUnk_03003430.bg2StreamColumn = temp_r2;
            var_r7 = temp_r2 & 0x1F;
            var_r5 = (temp_r2 + gUnk_03003430.bg2MapWidth) % gUnk_03003430.bg2MapWidth;
            var_r6 = gUnk_03003430.bg2VOfs >> 3;
            for (var_r3 = 0; var_r3 < 0x15; var_r3++) {
                temp_r1 = var_r3 + var_r6;
                gUnk_03004DB0[((temp_r1 & 0x1F) << 5) + var_r7]
                    = gUnk_03004790.pBufBg2Tilemap[(temp_r1 * gUnk_03003430.bg2MapWidth) + var_r5];
            }
        }
    }

    if (arg0 & 0x40) {
        gUnk_03003430.bg2VOfs += arg1.unk2;
        if (((s16)(gUnk_03003430.bg2VOfs - gUnk_03005468.unk2)) > -0x100u) {
            gUnk_03003430.bg2VOfs = gUnk_03005468.unk2;
        }
        temp_r2 = gUnk_03003430.bg2VOfs >> 3;
        if (temp_r2 != gUnk_03003430.bg2StreamRow) {
            gUnk_03003430.bg2StreamRow = temp_r2;
            var_r6 = gUnk_03003430.bg2HOfs >> 3;
            var_r7 = (temp_r2 & 0x1F) << 5;
            var_r5 = (((temp_r2 + gUnk_03003430.bg2MapHeight) % gUnk_03003430.bg2MapHeight) * gUnk_03003430.bg2MapWidth) + var_r6;
            for (var_r3 = 0; var_r3 < 0x1F; var_r3++) {
                gUnk_03004DB0[(var_r7) + (((var_r3 + var_r6) & 0x1F))] = gUnk_03004790.pBufBg2Tilemap[var_r5 + var_r3];
            }
        }
    } else if (arg0 & 0x80) {
        if (gUnk_03003430.bg2VOfs == 0) {
            gUnk_03003430.bg2StreamRow = 0xF000;
        }
        gUnk_03003430.bg2VOfs += arg1.unk2;
        if (gUnk_03003430.bg2VOfs >= (gUnk_03005468.unk6 - 0xA0)) {
            gUnk_03003430.bg2VOfs = gUnk_03005468.unk6 - 0xA0;
        }
        temp_r2 = gUnk_03003430.bg2VOfs >> 3;
        if (temp_r2 != gUnk_03003430.bg2StreamRow) {
            gUnk_03003430.bg2StreamRow = temp_r2;
            var_r6 = gUnk_03003430.bg2HOfs >> 3;
            temp_r0_7 = temp_r2 + 0x14;
            var_r7 = (temp_r0_7 & 0x1F) << 5;
            var_r5 = ((temp_r0_7 % gUnk_03003430.bg2MapHeight) * gUnk_03003430.bg2MapWidth) + var_r6;
            for (var_r3 = 0; var_r3 < 0x1F; var_r3++) {
                gUnk_03004DB0[(var_r7) + ((var_r3 + var_r6) & 0x1F)] = gUnk_03004790.pBufBg2Tilemap[var_r5 + var_r3];
            }
        }
    }
}
void ProcessOamSpriteLayout(void) {
    u16 sp0;
    struct Unk_0300542C *var_r2;
    u16 var_ip;
    struct ScrollBGLayer_Args var_r3;
    u16 var_r8;
    u8 var_r7;

    sp0 = 0;
    var_r7 = 0;
    var_r8 = 0;
    var_ip = 0;

    if (gUnk_030052A0 == 0xFE) {
        if (gUnk_0300542C != NULL) {
            for (var_r2 = gUnk_0300542C; var_r2->unk0 != 0xFFFF;) {
                if ((var_r2->unk4 >= gUnk_03002920[0].xPosBg2) && (var_r2->unk0 < gUnk_03002920[0].xPosBg2)
                    && (var_r2->unk2 < gUnk_03002920[0].yPosBg2) && (gUnk_03002920[0].yPosBg2 < var_r2->unk6)) {
                    gUnk_030008E8 = 1;
                    gUnk_0300358C = 1;
                    var_r8 = var_r2->unk8;
                    var_ip = var_r2->unk9;
                    break;
                }
                var_r2++;
                if (var_r2 == (struct Unk_0300542C *)0xC) {
                    break;
                }
            }
        }

        if ((s16)(var_r8 + gUnk_03002920[0].xPosBg2) > (s16)(gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER)) {
            var_r7 = 0x10;
            if (gUnk_0300358C != 0) {
                var_r3.unk0 = 1;
                if ((s16)(gUnk_03002920[0].xPosBg2) > (s16)(gUnk_03003430.bg2HOfs + 0xD8)) {
                    gUnk_0300358C = 0;
                }
            } else {
                if ((gUnk_03002920[0].xPosBg2 - (gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER)) > 5) {
                    var_r3.unk0 = 2;
                } else {
                    var_r3.unk0 = 1;
                }
            }
            if (gHeldKeys & DPAD_LEFT) {
                var_r7 = 0;
                var_r3.unk0 = 0;
            }
        } else if ((s16)(var_r8 + gUnk_03002920[0].xPosBg2) < (s16)(gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER)) {
            var_r7 = 0x20;
            if (gUnk_0300358C != 0) {
                var_r3.unk0 = -1;
                if ((s16)(gUnk_03002920[0].xPosBg2) < (s16)(gUnk_03003430.bg2HOfs + 0x18)) {
                    gUnk_0300358C = 0;
                }
            } else {
                if ((((gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER) - gUnk_03002920[0].xPosBg2) > 5)
                    && ((var_r8 | gUnk_0300358C) == 0)) {
                    var_r3.unk0 = -2;
                } else {
                    var_r3.unk0 = -1;
                }
            }
            if (gHeldKeys & DPAD_RIGHT) {
                var_r7 = 0;
                var_r3.unk0 = 0;
            }
        } else {
            gUnk_0300358C = 0;
        }

        if (gUnk_03002920[0].yPosScreen > 0xA0) {
            sp0 = 0x44;
        }
        if ((u16)(sp0 + gUnk_03002920[0].yPosScreen + var_ip) > 0x63) {
            s32 flag;
            var_r7 |= 0x80;
            if (gUnk_030008E8 == 0) {
                flag = 1;
            } else {
                var_r3.unk2 = 1;
                if ((s16)gUnk_03002920[0].yPosBg2 <= (s16)(gUnk_03003430.bg2VOfs + 0x90)) {
                    flag = 0;
                } else {
                    gUnk_030008E8 = 0;
                    var_ip = 0;
                    flag = 1;
                }
            }
            if (flag != 0) {
                if (((s16)(sp0 + gUnk_03002920[0].yPosScreen) < 0x64 || (s16)(sp0 + gUnk_03002920[0].yPosScreen) > 0x82)
                    && (var_ip == 0)) {
                    var_r3.unk2 = 3;
                } else {
                    var_r3.unk2 = 1;
                }
            }
        } else if ((u16)(sp0 + gUnk_03002920[0].yPosScreen + var_ip) <= 0x46) {
            s32 flag;
            var_r7 |= 0x40;
            if (gUnk_030008E8 == 0) {
                flag = 1;
            } else {
                var_r3.unk2 = -1;
                if ((s16)gUnk_03002920[0].yPosBg2 >= (s16)(gUnk_03003430.bg2VOfs + 0x18)) {
                    flag = 0;
                } else {
                    var_ip = 0;
                    gUnk_030008E8 = 0;
                    flag = 1;
                }
            }
            if (flag) {
                if ((u16)(sp0 - (gUnk_03002920[0].yPosScreen - 0x46)) > 0x19) {
                    var_r3.unk2 = -3;
                } else {
                    var_r3.unk2 = -1;
                }
            }
        } else {
            gUnk_030008E8 = 0;
        }

        if (gUnk_03002920[0].xPosScreen > 0xE0) {
            if ((gUnk_03005220.unk35 | gUnk_030034E4) == 0) {
                var_r7 = 0x10;
                var_r3.unk0 = 3;
            }
        } else if ((gUnk_03002920[0].xPosScreen <= 0xF) && ((gUnk_03005220.unk35 | gUnk_030034E4) == 0)) {
            var_r7 = 0x20;
            var_r3.unk0 = -3;
        }
        if (gUnk_03002920[0].yPosScreen <= 0x1F) {
            if ((gUnk_03005220.unk35 | gUnk_030034E4) == 0) {
                var_r7 &= 0xC0;
                var_r7 |= 0x40;
                var_r3.unk2 = -2;
                if (gUnk_03002920[0].yPosScreen <= 0x17) {
                    var_r3.unk2 = -4;
                }
            }
        } else if ((gUnk_03002920[0].yPosScreen > 0x90) && ((gUnk_03005220.unk35 | gUnk_030034E4) == 0)) {
            var_r7 &= 0xC0;
            var_r7 |= 0x80;
            var_r3.unk2 = 3;
        }
        if (var_r7 != 0) {
            ScrollBGLayer(var_r7, var_r3);
        }
    }

    gUnk_03003430.bg1HOfs = (gUnk_03003430.bg2HOfs >> 1);
    if (gUnk_03003430.bg2VOfs >= gUnk_03005474) {
        gUnk_03003430.bg1VOfs += (gUnk_03003430.bg2VOfs - gUnk_03005474) & 1;
    } else {
        gUnk_03003430.bg1VOfs -= (gUnk_03005474 - gUnk_03003430.bg2VOfs) & 1;
    }

    if (gUnk_03003430.bg1VOfs & 0x8000) {
        gUnk_03003430.bg1VOfs = 0;
    } else if (gUnk_03003430.bg1VOfs > 0x50) {
        gUnk_03003430.bg1VOfs = 0x50;
    }

    gUnk_03005474 = gUnk_03003430.bg2VOfs;
    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2X
        = (((gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER) << 8) - (gBg2PA * DISPLAY_WIDTH_CENTER)) - (gBg2PB * DISPLAY_HEIGHT_CENTER);
    gBg2Y = (((gUnk_03003430.bg2VOfs + DISPLAY_HEIGHT_CENTER) << 8) - (gBg2PC * DISPLAY_WIDTH_CENTER))
        - (gBg2PD * DISPLAY_HEIGHT_CENTER);
}
void UpdateCameraScroll(void) {
    s32 temp_r2;
    s32 temp_r3_2;
    u16 var_r8;
    struct ScrollBGLayer_Args var_r6;
    u8 var_r7;

    var_r8 = 0;
    var_r7 = 0;
    var_r6.unk2 = var_r6.unk0 = 0;

    if (!(gUnk_030051B8 & 0x30)) {
        if ((s16)gUnk_03002920[0].xPosBg2 > (s16)(gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER)) {
            var_r7 = 0x10;
            if ((gUnk_03002920[0].xPosBg2 - (gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER)) > 5) {
                var_r6.unk0 = 2;
            } else {
                var_r6.unk0 = 1;
            }
            if (gHeldKeys & DPAD_LEFT) {
                var_r7 = 0;
                var_r6.unk0 = 0;
            }
        } else if ((s16)gUnk_03002920[0].xPosBg2 < (s16)(gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER)) {
            var_r7 = 0x20;
            if (((gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER) - gUnk_03002920[0].xPosBg2) > 5) {
                var_r6.unk0 = -2;
            } else {
                var_r6.unk0 = -1;
            }
            if (gHeldKeys & DPAD_RIGHT) {
                var_r7 = 0;
                var_r6.unk0 = 0;
            }
        }
    }
    if (!(gUnk_030051B8 & 0xC0)) {
        if (gUnk_03002920[0].yPosScreen > 0xA0) {
            var_r8 = 0x44;
        }
        if ((u16)(var_r8 + gUnk_03002920[0].yPosScreen) > 0x63) {
            var_r7 |= 0x80;
            if ((u16)((var_r8 + gUnk_03002920[0].yPosScreen) - 0x64) > 0x1E) {
                var_r6.unk2 = 3;
            } else {
                var_r6.unk2 = 1;
            }
        } else if ((u16)(var_r8 + gUnk_03002920[0].yPosScreen) <= 0x46) {
            var_r7 |= 0x40;
            if ((u16)(var_r8 - (gUnk_03002920[0].yPosScreen - 0x46)) > 0x19U) {
                var_r6.unk2 = -3;
            } else {
                var_r6.unk2 = -1;
            }
        }
    }

    var_r7 |= gUnk_030051B8;
    temp_r3_2 = gUnk_03005480 + gUnk_030034E8.unk0;
    temp_r2 = gUnk_030007C0 + gUnk_030034E8.unk4;
    var_r6.unk0 += (temp_r3_2 >> 0x10);
    var_r6.unk2 += (temp_r2 >> 0x10);
    gUnk_03005480 = temp_r3_2 & 0xFFFF;
    gUnk_030007C0 = temp_r2 & 0xFFFF;

    if (gUnk_03005220.deathSequenceTimer != 0) {
        var_r7 &= 0x30;
        var_r6.unk2 = var_r6.unk0 = 0;
    }

    if (var_r7 != 0) {
        ScrollBGLayer(var_r7, var_r6);
    }

    gUnk_03003430.bg1HOfs = (gUnk_03003430.bg2HOfs >> 1);
    if (!(gUnk_03004C20.globalFrameCounter & 3) && (gUnk_03004660 == 0)) {
        if (gUnk_03003430.bg2VOfs >= gUnk_03005474) {
            gUnk_03003430.bg1VOfs += ((gUnk_03003430.bg2VOfs - gUnk_03005474) & 1);
        } else {
            gUnk_03003430.bg1VOfs -= ((gUnk_03005474 - gUnk_03003430.bg2VOfs) & 1);
        }

        if (gUnk_03003430.bg1VOfs & 0x8000) {
            gUnk_03003430.bg1VOfs = 0;
        } else if (gUnk_03003430.bg1VOfs > 0x50) {
            gUnk_03003430.bg1VOfs = 0x50;
        }
    }

    gUnk_03005474 = gUnk_03003430.bg2VOfs;
    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2X
        = (((gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER) << 8) - (gBg2PA * DISPLAY_WIDTH_CENTER)) - (gBg2PB * DISPLAY_HEIGHT_CENTER);
    gBg2Y = (((gUnk_03003430.bg2VOfs + DISPLAY_HEIGHT_CENTER) << 8) - (gBg2PC * DISPLAY_WIDTH_CENTER))
        - (gBg2PD * DISPLAY_HEIGHT_CENTER);
}
void UpdateCameraScrollPlayer2(void) {
    struct ScrollBGLayer_Args var_r2;
    u8 var_r4;

    var_r4 = 0;
    if (gUnk_03005220.deathSequenceTimer == 0) {
        var_r4 = 0x10;
        if ((gUnk_03002920[0].xPosBg2 - (gUnk_03003430.bg2HOfs + 0x28)) > 0) {
            var_r2.unk0 = gUnk_03002920[0].xPosBg2 - (gUnk_03003430.bg2HOfs + 0x28);
        } else {
            var_r2.unk0 = 1;
        }
    }

    gUnk_03002920[0].yPosBg2 = gUnk_03002920[0].yPosBg2; /* FAKE for matching */
    if ((gUnk_03005220.unk2F == 0) || (gUnk_03005220.unk31 == 0)) {
        if (gUnk_03002920[0].yPosBg2 > (gUnk_03003430.bg2VOfs + 0x6E)) {
            var_r4 |= 0x80;
            if (gUnk_03002920[0].yPosBg2 > (gUnk_03003430.bg2VOfs + 0xA)) {
                var_r2.unk2 = 3;
            } else {
                var_r2.unk2 = 1;
            }
        } else if (gUnk_03002920[0].yPosBg2 < (gUnk_03003430.bg2VOfs + 0x6E)) {
            var_r4 |= 0x40;
            if (gUnk_03002920[0].yPosBg2 < (gUnk_03003430.bg2VOfs + 0x64)) {
                var_r2.unk2 = 0xFFFD;
            } else {
                var_r2.unk2 = 0xFFFF;
            }
        }
    } else if (gUnk_03005220.unk2F > 0) {
        if ((gUnk_03002920[0].yPosBg2 - 0x28) > gUnk_03003430.bg2VOfs) {
            var_r4 |= 0x80;
            if (gUnk_03002920[0].yPosBg2 > (gUnk_03003430.bg2VOfs + 0x32)) {
                var_r2.unk2 = 3;
            } else {
                var_r2.unk2 = 2;
            }
        }
    } else if (gUnk_03002920[0].yPosBg2 < (gUnk_03003430.bg2VOfs + 0x82)) {
        var_r4 |= 0x40;
        if (gUnk_03002920[0].yPosBg2 < (gUnk_03003430.bg2VOfs + 0x78)) {
            var_r2.unk2 = 0xFFFD;
        } else {
            var_r2.unk2 = 0xFFFE;
        }
    }

    if (var_r4 != 0) {
        ScrollBGLayer(var_r4, var_r2);
    }

    gUnk_03003430.bg1HOfs = (gUnk_03003430.bg2HOfs >> 1);
    if (!(gUnk_03004C20.globalFrameCounter & 3)) {
        if (gUnk_03003430.bg2VOfs >= gUnk_03005474) {
            gUnk_03003430.bg1VOfs += ((gUnk_03003430.bg2VOfs - gUnk_03005474) & 1);
        } else {
            gUnk_03003430.bg1VOfs -= ((gUnk_03005474 - gUnk_03003430.bg2VOfs) & 1);
        }

        if (gUnk_03003430.bg1VOfs & 0x8000) {
            gUnk_03003430.bg1VOfs = 0;
        } else if (gUnk_03003430.bg1VOfs > 0x50) {
            gUnk_03003430.bg1VOfs = 0x50;
        }
    }

    gUnk_03005474 = gUnk_03003430.bg2VOfs;
    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2X
        = (((gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER) << 8) - (gBg2PA * DISPLAY_WIDTH_CENTER)) - (gBg2PB * DISPLAY_HEIGHT_CENTER);
    gBg2Y = (((gUnk_03003430.bg2VOfs + 0x50) << 8) - (gBg2PC * DISPLAY_WIDTH_CENTER)) - (gBg2PD * DISPLAY_HEIGHT_CENTER);
}
void CameraModeSwitchHandler(void) {
    s16 var_r1;
    s16 var_r5;
    u32 cWorld;

    switch (gUnk_030007E0.unkC_0) {
        case 0:
            break;

        case 1:
            gUnk_030007E0.unk6 = gUnk_03002920[0].xPosBg2 - 0x78;
            gUnk_030007E0.unk8 = gUnk_03002920[0].yPosBg2 - 0x8C;
            break;

        case 2:
            gUnk_030007E0.unk6 = (gUnk_03002920[0].xPosBg2 + ((gUnk_03002920[0x12].xPosBg2 - gUnk_03002920[0].xPosBg2) / 2)) - 0x78;
            gUnk_030007E0.unk8
                = (gUnk_03002920[0].yPosBg2 + (((gUnk_03002920[0x12].yPosBg2 - 0x40) - (gUnk_03002920[0].yPosBg2)) / 2)) - 0x50;
            break;

        case 3:
            gUnk_030007E0.unk6 = gUnk_03002920[0].xPosBg2 - 0x78;
            break;

        case 4:
            gUnk_030007E0.unk6 = 0x1E0 - gUnk_03002920[0].xPosBg2;
            gUnk_030007E0.unk8 = 0x140 - gUnk_03002920[0].yPosBg2;
            break;

        case 5:
            gUnk_030007E0.unk6 = (gUnk_03002920[0].xPosBg2 + ((gUnk_03002920[0x12].xPosBg2 - gUnk_03002920[0].xPosBg2) / 2)) - 0x78;
            gUnk_030007E0.unk8 = 0x94;
            break;

        case 6:
            gUnk_030007E0.unk6 = gUnk_03002920[0].xPosBg2 - 0x78;
            gUnk_030007E0.unk8 = 0x5C;
            break;

        case 7:
            gUnk_030007E0.unk6 = gUnk_03002920[0].xPosBg2 - 0x78;
            if (gUnk_03002920[0].yPosBg2 <= 0xA9) {
                gUnk_030007E0.unk8 = 0x3C;
            } else {
                gUnk_030007E0.unk8 = 0xA0;
            }
            break;
    }

    if (gUnk_030007E0.unk0 > gUnk_030007E0.unk6) {
        gUnk_030007E0.unk0 -= 1;
    }
    if (gUnk_030007E0.unk0 < gUnk_030007E0.unk6) {
        gUnk_030007E0.unk0 += 1;
    }

    if (gUnk_030007E0.unk2 > gUnk_030007E0.unk8) {
        gUnk_030007E0.unk2 -= 1;
    }
    if (gUnk_030007E0.unk2 < gUnk_030007E0.unk8) {
        gUnk_030007E0.unk2 += 1;
    }

    if (gUnk_030007E0.unk0 < 0) {
        gUnk_030007E0.unk0 = 0;
    }
    if (gUnk_030007E0.unk2 < 0x3C) {
        gUnk_030007E0.unk2 = 0x3C;
    }

    if (gUnk_030007E0.unk0 > 0xF0) {
        gUnk_030007E0.unk0 = 0xF0;
    }
    if (gUnk_030007E0.unk2 > 0xA0) {
        gUnk_030007E0.unk2 = 0xA0;
    }

    gUnk_03003430.bg2HOfs = (u16)gUnk_030007E0.unk0;
    if (gBg2Alpha == 0) {
        gUnk_03003430.bg2VOfs = (u16)gUnk_030007E0.unk2 + 0x10;
    } else {
        gUnk_03003430.bg2VOfs = (u16)gUnk_030007E0.unk2;
    }
    gUnk_03003430.bg1HOfs = ((s16)gUnk_030007E0.unk0 / 15);

    switch (gUnk_030007E0.unkC_4) {
        case 0:
            break;

        case 1:
            var_r5 = Abs(gUnk_03002920[0].xPosBg2 - gUnk_03002920[0x12].xPosBg2) - 0xA0;
            if (var_r5 < 0) {
                var_r5 = 0;
            }

            var_r1 = Abs(gUnk_03002920[0].yPosBg2 - gUnk_03002920[0x12].yPosBg2);
            if (var_r1 < 0) {
                var_r1 = 0;
            }

            if (var_r5 > var_r1) {
                gUnk_030007E0.unkA = var_r5 & 0xFE;
            } else {
                gUnk_030007E0.unkA = var_r1 & 0xFE;
            }
            break;
    }

    if ((u16)gUnk_030007E0.unk4 > (u16)gUnk_030007E0.unkA) {
        gUnk_030007E0.unk4 -= 2;
    }
    if ((u16)gUnk_030007E0.unk4 < (u16)gUnk_030007E0.unkA) {
        gUnk_030007E0.unk4 += 2;
    }

    if (gUnk_030007E0.unkC_4) {
        if ((u16)gUnk_030007E0.unk4 > 0x60U) {
            gUnk_030007E0.unk4 = 0x60;
        }
        if ((u16)gUnk_030007E0.unk4 == 0) {
            gUnk_030007E0.unk4 = 0;
        }
    }

    gBg2XMag = 0x100 - gUnk_030007E0.unk4;
    gBg2YMag = 0x100 - gUnk_030007E0.unk4;

    cWorld = gUnk_03004C20.world;
    if (cWorld == 4) {
        if (gUnk_03005400.unkE_2 == 0) {
            gUnk_03005400.unk16 = gBg2Alpha;
            gUnk_03005400.unk14 = 0;
        } else if (gUnk_03004C20.sceneFrameCounter & 2) {
            if (Abs(gUnk_03005400.unk16) <= 4) {
                gBg2Alpha = ((gUnk_03005400.unk16 * COS(gUnk_03005400.unk14)) >> 8);
                if (gUnk_03004C20.sceneFrameCounter & 4) {
                    gUnk_03005400.unk14 += 4;
                }
            } else {
                gBg2Alpha = ((gUnk_03005400.unk16 * COS(gUnk_03005400.unk14)) >> 8);
                gUnk_03005400.unk14 += 4;
                if (!(gUnk_03005400.unk14 % 0x40)) {
                    if (gUnk_03005400.unk16 > 0) {
                        gUnk_03005400.unk16 -= 1;
                    } else {
                        gUnk_03005400.unk16 += 1;
                    }
                }
            }
        }

        gUnk_03005440.unk0 = -((COS(gBg2Alpha) * 0xF) >> 5);
        gUnk_03005440.unk4 = ((COS(gBg2Alpha) * 0xF) >> 5);
        gUnk_03005440.unk2 = -((SIN(gBg2Alpha) * 0xF) >> 5);
        gUnk_03005440.unk6 = ((SIN(gBg2Alpha) * 0xF) >> 5);

        if ((((u32)gUnk_03004C20.sceneFrameCounter % (u32)(0xA - Abs((s8)gBg2Alpha / 2))) == 0) && (gUnk_03005400.unkC != 0)
            && (gUnk_03005220.unk31 != 0) && (((s8)gBg2Alpha < -2) || ((s8)gBg2Alpha > 2))) {
            if ((s8)gBg2Alpha > 0) {
                gUnk_03002920[0].xPosBg2 += 3;
            }
            if ((s8)gBg2Alpha < 0) {
                gUnk_03002920[0].xPosBg2 -= 3;
            }
        }

        if ((s8)gBg2Alpha > 0x14) {
            gBg2Alpha = 0x14;
        }
        if ((s8)gBg2Alpha < -0x14) {
            gBg2Alpha = -0x14;
        }

        gUnk_03003430.bg2VOfs = 0x58;
        gUnk_03005440.unk0 += 0xF0;
        gUnk_03005440.unk4 += 0xF0;
        gUnk_03005440.unk2 += 0xDC;
        gUnk_03005440.unk6 += 0xDC;
    }

    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));

    ComputeScrollLimits();
    if (gUnk_03004C20.world == 1) {
        gUnk_03003430.bg1VOfs = (gBg2YMag / 3) - 0x28;
    }
}
void InitLevelFromROMTable(void) {
    u32 var_r6;

    var_r6 = 0;

    gUnk_03004654 = gLevelRoomData[gUnk_03004C20.world - 1][gUnk_03004C20.level + 8];
    gUnk_03000800 = gUnk_08052624[gUnk_03004C20.world - 1][gUnk_03004C20.level];

    if (gUnk_03004C20.level == 0) {
        gUnk_03005210 = 0xFFFF;
        gUnk_03004C20.room = 1;
        gUnk_03005468.unk0 = 0;
        gUnk_03005468.unk2 = 0;
        gUnk_03005468.unk4 = 0x100;
        gUnk_03005468.unk6 = 0x100;
    } else if (gUnk_03004C20.level == 8) {
        gUnk_03005210 = 0xFFFF;
        gUnk_03004C20.room = 1;
        gUnk_03005468.unk0 = 0;
        gUnk_03005468.unk2 = 0;
        gUnk_03005468.unk4 = 0x200;
        gUnk_03005468.unk6 = 0x200;
    } else {
        if (gUnk_03004C20.room == 0) {
            gUnk_030051C8 = gUnk_03004654[1] - 1;
            gUnk_03005210 = 0xFFFF;
            gUnk_03004C20.unk8 = 0;
            gUnk_03005284->unk6 = 0;
            gUnk_03005284->unk18 = gUnk_03005220.unk4 = 0;
        } else if (gUnk_03004C20.room == 0xFF) {
            gUnk_03005210 = 0xFFFF;
            if ((gUnk_03005284->unk6 == 0)
                || ((gUnk_03004C20.world == 6) && ((gUnk_03004C20.level == 1) || (gUnk_03004C20.level == 3)))) {
                gUnk_030051C8 = gUnk_03004654[1] - 1;
                gUnk_03004C20.unk8 = 0;
            } else {
                gUnk_030051C8 = gUnk_03005284->unk6;
                var_r6 = 1;
            }
        } else {
            var_r6 = 2;
        }

        gUnk_03004C20.room
            = gUnk_080D48C8[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_030051C8 - (gUnk_03004654[1] - 1)].unk4_2;
        gUnk_03005468.unk0 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk0;
        gUnk_03005468.unk2 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk2;
        gUnk_03005468.unk4 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk4;
        gUnk_03005468.unk6 = gUnk_080D2E88[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_03004C20.room - 1].unk6;
        gUnk_030051CC.unk0 = gUnk_03005468.unk0 + ((s32)(gUnk_03005468.unk4 - gUnk_03005468.unk0) >> 1);
        gUnk_030051CC.unk2 = gUnk_03005468.unk2 + ((s32)(gUnk_03005468.unk6 - gUnk_03005468.unk2) >> 1);
    }
    gUnk_03004C20.unkA = 0;
    gUnk_03004C20.unkB = 0;
    gUnk_03004C20.sceneFrameCounter = 0;
    InitScrollState();
    if (gUnk_03004C20.level != 0) {
        InitLevelGameplay(var_r6);
    } else {
        gUnk_030034E4 = 1;
        gCallbackQueue.next[0] = ReadKeyInput;
        gCallbackQueue.next[1] = InitGameplayFromWorldMap;
        gCallbackQueue.next[2] = VBlankCallback_Dialog;
        gUnk_03003410.unk5 = 0;
        gCallbackQueue.next[3] = &UpdateWorldMapLogic;
        gCallbackQueue.next[4] = &TransitionInitLevelMusic;
        gCallbackQueue.next[5] = 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = 0;
        gCallbackQueue.nextCount = 6;
    }
    if (gUnk_03002920[0].xPosBg2 < (gUnk_03005468.unk0 + 0x78)) {
        gUnk_03003430.bg2HOfs = gUnk_03005468.unk0;
    } else {
        if ((gUnk_03005468.unk4 - 0x78) < gUnk_03002920[0].xPosBg2) {
            gUnk_03003430.bg2HOfs = gUnk_03005468.unk4 - 0xF0;
        } else {
            gUnk_03003430.bg2HOfs = gUnk_03002920[0].xPosBg2 - 0x78;
        }
    }
    if (gUnk_03002920[0].yPosBg2 < (gUnk_03005468.unk2 + 0x78)) {
        gUnk_03003430.bg2VOfs = gUnk_03005468.unk2;
    } else {
        if ((gUnk_03005468.unk6 - 0x28) < gUnk_03002920[0].yPosBg2) {
            gUnk_03003430.bg2VOfs = gUnk_03005468.unk6 - 0xA0;
        } else {
            gUnk_03003430.bg2VOfs = gUnk_03002920[0].yPosBg2 - 0x78;
        }
    }
    if ((gUnk_03004C20.level == 6) && (gUnk_030034E8.unk0 == 0) && (gUnk_030034E8.unk4 != 0)) {
        gUnk_03003430.bg2VOfs += 0x30;
    }
    if (gUnk_03004C20.level != 8) {
        ScrollBGColumnLoad(0);
        DmaCopy16Wait(3, gUnk_03004DB0, gUnk_03003430.pVramBg2Tilemap, 0x400);
    } else {
        DmaFill16(3, 0, &gUnk_03003650, 0x1000);
        for (var_r6 = 0; var_r6 < 0x28; var_r6++) {
            DmaCopy16Wait(3, &gUnk_03004790.pBufBg2Tilemap[var_r6 * gUnk_03003430.bg2MapWidth],
                          (void *)((var_r6 << 6) + &gUnk_03003650), 0x1E * 2);
        }
        DmaCopy16Wait(3, &gUnk_03003650, gUnk_03003430.pVramBg2Tilemap, 0x1000);
    }
    if (gUnk_03004C20.level == 8) {
        gUnk_03003430.bg1VOfs = 0;
    } else {
        gUnk_03003430.bg1VOfs = 0x50;
    }
}
void ScrollBGColumnLoad(u8 arg0) {
    u32 temp_r5;
    u32 temp_r1;
    u32 temp_r2;
    u32 var_r6;

    gUnk_03003430.bg2VOfs -= arg0 * 8;

    for (var_r6 = 0; var_r6 < 0x400; var_r6++) {
        temp_r1 = gUnk_03003430.bg2HOfs >> 3;
        temp_r2 = (temp_r1 + (var_r6 & 0x1F)) >> 5;
        temp_r5 = (((gUnk_03003430.bg2VOfs >> 3) & 0x1F) << 5) + var_r6;
        if (temp_r2 != 0) {
            temp_r5 += (temp_r1 - (temp_r2 << 5));
        } else {
            temp_r5 += temp_r1;
        }
        gUnk_03004DB0[temp_r5 & 0x3FF]
            = gUnk_03004790
                  .pBufBg2Tilemap[(gUnk_03003430.bg2MapWidth * (var_r6 >> 5)) + (var_r6 & 0x1F)
                                  + ((gUnk_03003430.bg2VOfs >> 3) * gUnk_03003430.bg2MapWidth) + (gUnk_03003430.bg2HOfs >> 3)];
    }

    gUnk_03003430.bg2VOfs += (arg0 * 8);
}
INCLUDE_ASM("asm/nonmatchings/engine", InitVideoAndBG);
void ComputeRotationMatrix(void) {
    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));

    gBg2X
        = (((gUnk_03003430.bg2HOfs + DISPLAY_WIDTH_CENTER) << 8) - (gBg2PA * DISPLAY_WIDTH_CENTER)) - (gBg2PB * DISPLAY_HEIGHT_CENTER);
    gBg2Y = (((gUnk_03003430.bg2VOfs + DISPLAY_HEIGHT_CENTER) << 8) - (gBg2PC * DISPLAY_WIDTH_CENTER))
        - (gBg2PD * DISPLAY_HEIGHT_CENTER);

    if (gBg2XMag != 0x100) {
        gBg2Alpha += 8;
        gBg2XMag -= 0x10;
        gBg2YMag -= 0x10;
        gUnk_03004C20.sceneFrameCounter = 0;
    } else if ((gUnk_03004C20.sceneFrameCounter == 0x258) || (gNewKeys & (START_BUTTON | B_BUTTON | A_BUTTON))) {
        gMosaicSize = 0;
        gUnk_03003410.unk7 = 1;
        gUnk_03004C20.world = 6;
        gUnk_03004C20.level = 3;
        gCallbackQueue.current[1] = TransitionGameplayInit;
    }
}
INCLUDE_ASM("asm/nonmatchings/engine", ResetVideoRegisters); /* RenderFrame — per-frame rendering dispatch */
/**
 * ClearOamBuffer: zero `count` OAM shadow entries (0x1C bytes each), one word at a
 * time starting just past `oamEntry`. Shared by the three OAM-clear routines below.
 *
 * Both pins are irreducible (confirmed against the kleod reference decomp, which
 * writes all three callers longhand with the same two pins). They fix two
 * INDEPENDENT things — verified by removing each in isolation:
 *   - bytesLeft(r3): removes the innermost, highest-priority counter from the
 *     allocation pass. Without it that counter naturally grabs r0 and displaces
 *     `oamEntry` (ptr->r2, zero->r3), rotating every register.
 *   - entryCount(r1): anchors the decrement's SCHEDULE. As a plain pseudo, agbcc
 *     hoists `subs r1,#1` above the inner loop; the pin keeps it at the bottom
 *     next to its compare, matching the target.
 * Neither pin substitutes for the other; this is allocation/scheduling info that
 * tight register-counter loops can't express in plain C. (Our shared inline helper
 * is cleaner than kleod's three hand-copied bodies.)
 */
static inline void ClearOamBuffer(u32 *oamEntry, s32 count) {
    register s32 entryCount asm("r1") = count;
    register s32 bytesLeft asm("r3");
    do {
        bytesLeft = 0x1C;
        do {
            oamEntry++;
            bytesLeft -= 4;
            *oamEntry = 0;
        } while (bytesLeft != 0);
        entryCount--;
    } while (entryCount != 0);
}
/**
 * ClearVideoState: zero all 99 OAM shadow entries, then re-init the OAM table.
 */
void ClearVideoState(void) {
    ClearOamBuffer(gOamBuffer0, 0x63);
    InitOamEntries();
}
/**
 * ClearOamBufferExtended: zero OAM shadow buffer entries 1-98.
 */
void ClearOamBufferExtended(void) {
    ClearOamBuffer(gOamBuffer1, 0x62);
}
/**
 * ClearOamEntries6Plus: zero OAM shadow buffer entries 6-91.
 */
void ClearOamEntries6Plus(void) {
    ClearOamBuffer(gOamBuffer6, 0x56);
}
