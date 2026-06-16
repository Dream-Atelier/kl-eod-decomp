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
extern void InitLevelGameplay(u32 arg0); /* InitLevelGameplay */
extern void ReadKeyInput(void); /* ReadKeyInput == kleod InputHandler_Normal */
extern void InitGameplayFromWorldMap(void);
extern void VBlankCallback_Dialog(void); /* VBlankCallback_Dialog */
extern void UpdateWorldMapLogic(void); /* sub_08048028 (off-by-2 in our split) */
extern void TransitionInitLevelMusic(void); /* sub_080242C0 */
extern void ScrollBGColumnLoad(u8 arg0); /* ScrollBGColumnLoad */

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
    vu8 *vcount_reg = (vu8 *)0x04000006;
    u8 *entity = (u8 *)0x03002920;
    u8 fade = sub_08051A0C(*vcount_reg, entity[0x08]);

    if (fade <= 16) {
        *(vu16 *)0x04000052 = ((u32)fade << 8) | fade;
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
    u16 vcount = *(volatile u16 *)0x04000006;
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
        u32 winAddr = 0x04000042;
        volatile u16 *win;
        asm("" : "=r"(win) : "0"(winAddr));
        *win = ((0x78 - hw) << 8) | (hw + 0x78);
    } else {
        *(volatile u16 *)0x04000042 = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/engine", UpdateBGScrollWithWave);
INCLUDE_ASM("asm/nonmatchings/engine", WaitVBlankAndClearMosaic);
INCLUDE_ASM("asm/nonmatchings/engine", AcknowledgeInterrupt);
INCLUDE_ASM("asm/nonmatchings/engine", InitLevelBG); /* InitGraphicsSystem — full GFX init: decompress assets, configure BG/VRAM */
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
        if (temp_r2 != gUnk_03003430.unk44) {
            gUnk_03003430.unk44 = temp_r2;
            temp_r0_3 = temp_r2 + 0x1E;
            var_r7 = temp_r0_3 & 0x1F;
            var_r5 = temp_r0_3 % gUnk_03003430.unk48;
            var_r6 = gUnk_03003430.bg2VOfs >> 3;
            for (var_r3 = 0; var_r3 < 0x15; var_r3++) {
                temp_r1 = var_r3 + var_r6;
                gUnk_03004DB0[((temp_r1 & 0x1F) << 5) + var_r7]
                    = gUnk_03004790.pBufBg2Tilemap[(temp_r1 * gUnk_03003430.unk48) + var_r5];
            }
        }
    } else if (arg0 & 0x20) {
        gUnk_03003430.bg2HOfs += arg1.unk0;
        if (((s16)(gUnk_03003430.bg2HOfs - gUnk_03005468.unk0)) > -0x100u) {
            gUnk_03003430.bg2HOfs = gUnk_03005468.unk0;
        }
        temp_r2 = gUnk_03003430.bg2HOfs >> 3;
        if (temp_r2 != gUnk_03003430.unk44) {
            gUnk_03003430.unk44 = temp_r2;
            var_r7 = temp_r2 & 0x1F;
            var_r5 = (temp_r2 + gUnk_03003430.unk48) % gUnk_03003430.unk48;
            var_r6 = gUnk_03003430.bg2VOfs >> 3;
            for (var_r3 = 0; var_r3 < 0x15; var_r3++) {
                temp_r1 = var_r3 + var_r6;
                gUnk_03004DB0[((temp_r1 & 0x1F) << 5) + var_r7]
                    = gUnk_03004790.pBufBg2Tilemap[(temp_r1 * gUnk_03003430.unk48) + var_r5];
            }
        }
    }

    if (arg0 & 0x40) {
        gUnk_03003430.bg2VOfs += arg1.unk2;
        if (((s16)(gUnk_03003430.bg2VOfs - gUnk_03005468.unk2)) > -0x100u) {
            gUnk_03003430.bg2VOfs = gUnk_03005468.unk2;
        }
        temp_r2 = gUnk_03003430.bg2VOfs >> 3;
        if (temp_r2 != gUnk_03003430.unk46) {
            gUnk_03003430.unk46 = temp_r2;
            var_r6 = gUnk_03003430.bg2HOfs >> 3;
            var_r7 = (temp_r2 & 0x1F) << 5;
            var_r5 = (((temp_r2 + gUnk_03003430.unk4A) % gUnk_03003430.unk4A) * gUnk_03003430.unk48) + var_r6;
            for (var_r3 = 0; var_r3 < 0x1F; var_r3++) {
                gUnk_03004DB0[(var_r7) + (((var_r3 + var_r6) & 0x1F))] = gUnk_03004790.pBufBg2Tilemap[var_r5 + var_r3];
            }
        }
    } else if (arg0 & 0x80) {
        if (gUnk_03003430.bg2VOfs == 0) {
            gUnk_03003430.unk46 = 0xF000;
        }
        gUnk_03003430.bg2VOfs += arg1.unk2;
        if (gUnk_03003430.bg2VOfs >= (gUnk_03005468.unk6 - 0xA0)) {
            gUnk_03003430.bg2VOfs = gUnk_03005468.unk6 - 0xA0;
        }
        temp_r2 = gUnk_03003430.bg2VOfs >> 3;
        if (temp_r2 != gUnk_03003430.unk46) {
            gUnk_03003430.unk46 = temp_r2;
            var_r6 = gUnk_03003430.bg2HOfs >> 3;
            temp_r0_7 = temp_r2 + 0x14;
            var_r7 = (temp_r0_7 & 0x1F) << 5;
            var_r5 = ((temp_r0_7 % gUnk_03003430.unk4A) * gUnk_03003430.unk48) + var_r6;
            for (var_r3 = 0; var_r3 < 0x1F; var_r3++) {
                gUnk_03004DB0[(var_r7) + ((var_r3 + var_r6) & 0x1F)] = gUnk_03004790.pBufBg2Tilemap[var_r5 + var_r3];
            }
        }
    }
}
INCLUDE_ASM("asm/nonmatchings/engine", ProcessOamSpriteLayout);
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

    if (gUnk_03005220.unk46 != 0) {
        var_r7 &= 0x30;
        var_r6.unk2 = var_r6.unk0 = 0;
    }

    if (var_r7 != 0) {
        ScrollBGLayer(var_r7, var_r6);
    }

    gUnk_03003430.bg1HOfs = (gUnk_03003430.bg2HOfs >> 1);
    if (!(gUnk_03004C20.unk4 & 3) && (gUnk_03004660 == 0)) {
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
    if (gUnk_03005220.unk46 == 0) {
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
    if (!(gUnk_03004C20.unk4 & 3)) {
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
        } else if (gUnk_03004C20.unk0 & 2) {
            if (Abs(gUnk_03005400.unk16) <= 4) {
                gBg2Alpha = ((gUnk_03005400.unk16 * COS(gUnk_03005400.unk14)) >> 8);
                if (gUnk_03004C20.unk0 & 4) {
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

        if ((((u32)gUnk_03004C20.unk0 % (u32)(0xA - Abs((s8)gBg2Alpha / 2))) == 0) && (gUnk_03005400.unkC != 0)
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

    gUnk_03004654 = ((u8(*)[8][0x1C])((u8 *)gUnk_08051EFE + 0xEA))[gUnk_03004C20.world - 1][gUnk_03004C20.level + 8];
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
    gUnk_03004C20.unk0 = 0;
    InitScrollState();
    if (gUnk_03004C20.level != 0) {
        InitLevelGameplay(var_r6);
    } else {
        gUnk_030034E4 = 1;
        gUnk_03003510.unk28[0] = ReadKeyInput;
        gUnk_03003510.unk28[1] = InitGameplayFromWorldMap;
        gUnk_03003510.unk28[2] = VBlankCallback_Dialog;
        gUnk_03003410.unk5 = 0;
        gUnk_03003510.unk34 = &UpdateWorldMapLogic;
        gUnk_03003510.unk38 = &TransitionInitLevelMusic;
        gUnk_03003510.unk3C = 1;
        gUnk_03003510.unk0[gUnk_03003510.unk78 - 1] = 0;
        gUnk_03003510.unk79 = 6;
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
            DmaCopy16Wait(3, &gUnk_03004790.pBufBg2Tilemap[var_r6 * gUnk_03003430.unk48], (void *)((var_r6 << 6) + &gUnk_03003650),
                          0x1E * 2);
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
            = gUnk_03004790.pBufBg2Tilemap[(gUnk_03003430.unk48 * (var_r6 >> 5)) + (var_r6 & 0x1F)
                                           + ((gUnk_03003430.bg2VOfs >> 3) * gUnk_03003430.unk48) + (gUnk_03003430.bg2HOfs >> 3)];
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
        gUnk_03004C20.unk0 = 0;
    } else if ((gUnk_03004C20.unk0 == 0x258) || (gNewKeys & (START_BUTTON | B_BUTTON | A_BUTTON))) {
        gUnk_030007D8 = 0;
        gUnk_03003410.unk7 = 1;
        gUnk_03004C20.world = 6;
        gUnk_03004C20.level = 3;
        gUnk_03003510.unk0[1] = TransitionGameplayInit;
    }
}
INCLUDE_ASM("asm/nonmatchings/engine", ResetVideoRegisters); /* RenderFrame — per-frame rendering dispatch */
/**
 * ClearVideoState: zeroes all 99 OAM shadow entries then calls InitOamEntries.
 */
void ClearVideoState(void) {
    register u32 *oamEntry asm("r0") = (u32 *)gOamBuffer0;
    register s32 entryCount asm("r1") = 0x63;
    register s32 zeroFill asm("r2") = 0;
    do {
        s32 bytesLeft = 0x1C;
        do {
            oamEntry++;
            bytesLeft -= 4;
            *oamEntry = zeroFill;
        } while (bytesLeft != 0);
        entryCount--;
    } while (entryCount != 0);
    InitOamEntries();
}
/**
 * ClearOamBufferExtended: zero OAM shadow buffer entries 1-98.
 */
void ClearOamBufferExtended(void) {
    register u32 *oamEntry asm("r0") = (u32 *)gOamBuffer1;
    register s32 entryCount asm("r1") = 0x62;
    register s32 zeroFill asm("r2") = 0;
    do {
        s32 bytesLeft = 0x1C;
        do {
            oamEntry++;
            bytesLeft -= 4;
            *oamEntry = zeroFill;
        } while (bytesLeft != 0);
        entryCount--;
    } while (entryCount != 0);
}
/**
 * ClearOamEntries6Plus: zero OAM shadow buffer entries 6-91.
 */
void ClearOamEntries6Plus(void) {
    register u32 *oamEntry asm("r0") = (u32 *)gOamBuffer6;
    register s32 entryCount asm("r1") = 0x56;
    register s32 zeroFill asm("r2") = 0;
    do {
        s32 bytesLeft = 0x1C;
        do {
            oamEntry++;
            bytesLeft -= 4;
            *oamEntry = zeroFill;
        } while (bytesLeft != 0);
        entryCount--;
    } while (entryCount != 0);
}
