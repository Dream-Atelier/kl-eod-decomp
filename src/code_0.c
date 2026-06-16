#include "global.h"
#include "globals.h"
#include "include_asm.h"
#include "data/trig.h"
#include "structs/variables.h"

extern void TransformEntityScreenPositions(void);
extern void RenderHUDTop(void);
extern void RenderDialogSprites(void);
extern void thunk_UpdateRng(void);
extern void m4aSoundMain(void);
extern void VBlankIntrWait(void);
extern s16 MultiplyQ8(s16 a, s16 b);
extern s16 ReciprocalQ8(s16 a);

INCLUDE_ASM("asm/nonmatchings/code_0", SetupOAMSprite); /* DrawSpriteTiles — core sprite/tile VRAM writer */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderHUDTop); /* RenderHUDTop */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderHUDBottom); /* RenderHUDBottom */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderMenuUI); /* RenderMenuUI */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderDialogBox); /* RenderDialogBox */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderDialogSprites); /* RenderDialogSprites */
/**
 * InitOamEntries: initialize 128 OAM entries from a ROM template.
 *
 * Copies the 8-byte template at 0x080E2A7C into each slot of the OAM
 * shadow buffer at 0x03004800, then overrides the affineParam halfword
 * of each slot from the rotation/scaling table at 0x03004680.
 */
void InitOamEntries(void) {
    s32 var_r2;
    u16 *var_r5;
    union Unk_03000820 var;

    var_r5 = (u16 *)gUnk_03004680;
    var = gUnk_080E2A7C;

    for (var_r2 = 0; var_r2 < 0x80; var_r2++) {
        gUnk_03004800[var_r2].all = var.all;
        gUnk_03004800[var_r2].all.affineParam = *var_r5++;
    }
}
void TransformEntityScreenPositions(void) {
    s32 var_r5;

    gUnk_03002920[0].xPosScreen = gUnk_03002920[0].xPosBg2 - gUnk_03003430.bg2HOfs;
    gUnk_03002920[0].yPosScreen = gUnk_03002920[0].yPosBg2 - gUnk_03003430.bg2VOfs;

    if (gUnk_03002920[0x9].unk10 == 1) {
        gUnk_03002920[0x9].xPosScreen = gUnk_03002920[0x9].xPosBg2 - gUnk_03003430.bg2HOfs;
        gUnk_03002920[0x9].yPosScreen = gUnk_03002920[0x9].yPosBg2 - gUnk_03003430.bg2VOfs;
        gUnk_03002920[0xA].xPosScreen = gUnk_03002920[0xA].xPosBg2 - gUnk_03003430.bg2HOfs;
        gUnk_03002920[0xA].yPosScreen = gUnk_03002920[0xA].yPosBg2 - gUnk_03003430.bg2VOfs;
    }

    for (var_r5 = 1; var_r5 < gUnk_03005428; var_r5++) {
        if (var_r5 == 9) {
            var_r5 = 0xD;
        }

        if (gUnk_03002920[var_r5].unkF < 0x19) {
            gUnk_03002920[var_r5].xPosScreen = gUnk_03002920[var_r5].xPosBg2 - gUnk_03003430.bg2HOfs;
            gUnk_03002920[var_r5].yPosScreen = gUnk_03002920[var_r5].yPosBg2 - gUnk_03003430.bg2VOfs;
            if ((gUnk_03002920[var_r5].xPosScreen >= (DISPLAY_WIDTH + 35) && gUnk_03002920[var_r5].xPosScreen <= (u16)-36)
                || (gUnk_03002920[var_r5].yPosScreen >= (DISPLAY_HEIGHT + 64) && gUnk_03002920[var_r5].yPosScreen <= (u16)-36)) {
                gUnk_03002920[var_r5].unk10 = 0;
            } else {
                gUnk_03002920[var_r5].unk10 = 1;
            }
        }
    }
}
INCLUDE_ASM("asm/nonmatchings/code_0", TransformSingleEntityToScreen);
INCLUDE_ASM("asm/nonmatchings/code_0", TransformAllEntitiesToScreen);
INCLUDE_ASM("asm/nonmatchings/code_0", HandlePauseMenuInput);
INCLUDE_ASM("asm/nonmatchings/code_0", UpdateUIState); /* UpdateUIState */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderCharacterTiles); /* RenderCharacterTiles */
/**
 * UpdateTextScroll: advance a packed (X|Y<<16) position one Bresenham step
 * along a fixed direction toward arg1's endpoint, using gUnk_030034DC as
 * the rolling fractional error.
 */
void *UpdateTextScroll(s32 *arg0, struct Unk_0800BEF0 arg1) {
    u32 temp_r0;
    u32 var_r4;
    s8 var_r2;
    s8 var_r3;
    s8 var_r5;
    s8 var_r7;

    var_r3 = (arg1.unk4 - arg1.unk0) >> 3;
    var_r5 = (arg1.unk6 - arg1.unk2) >> 3;
    var_r4 = (arg1.unk2 << 0x10) | arg1.unk0;

    if (var_r3 < 0) {
        temp_r0 = arg1.unk8;
        var_r7 = -temp_r0;
        var_r3 = -var_r3;
    } else {
        var_r7 = arg1.unk8;
    }

    if (var_r5 < 0) {
        temp_r0 = arg1.unk8;
        var_r2 = -temp_r0;
        var_r5 = -var_r5;
    } else {
        var_r2 = arg1.unk8;
    }

    if (var_r3 >= var_r5) {
        var_r4 = (u16)(var_r7 + var_r4) | (var_r4 & 0xFFFF0000);
        gUnk_030034DC += var_r5;
        if (gUnk_030034DC >= var_r3) {
            var_r4 = (((var_r4 >> 0x10) + var_r2) << 0x10) | (var_r4 & 0xFFFF);
            gUnk_030034DC -= var_r3;
        }
    } else {
        var_r4 = (((var_r4 >> 0x10) + var_r2) << 0x10) | (var_r4 & 0xFFFF);
        gUnk_030034DC += var_r3;
        if (gUnk_030034DC >= var_r5) {
            var_r4 = (u16)(var_r7 + var_r4) | (var_r4 & 0xFFFF0000);
            gUnk_030034DC -= var_r5;
        }
    }

    *arg0 = var_r4;
    return arg0;
}
void VBlankCallback_Gameplay(void) {
    TransformEntityScreenPositions();
    RenderHUDTop();
    VBlankIntrWait();
    REG_BG0HOFS = (gUnk_03003430.bg0HOfs >> 2) & 0x1FF;
    REG_BG0VOFS = (gUnk_03003430.bg0VOfs >> 5) & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;
    REG_BG2X_L = gBg2X;
    REG_BG2X_H = (gBg2X & 0x0FFF0000) >> 0x10;
    REG_BG2Y_L = gBg2Y;
    REG_BG2Y_H = (gBg2Y & 0x0FFF0000) >> 0x10;
    REG_BG2PA = gBg2PA;
    REG_BG2PB = gBg2PB;
    REG_BG2PC = gBg2PC;
    REG_BG2PD = gBg2PD;
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = (gUnk_030007D8 << 0xC) | (gUnk_030007D8 << 8) | (gUnk_030007D8 << 4) | gUnk_030007D8;
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_0", AnimatePaletteEffects);
void VBlankCallback_Dialog(void) {
    RenderDialogSprites();
    gUnk_03004678 = SIN(gBg2Alpha);
    gUnk_030051B0 = COS(gBg2Alpha);
    VBlankIntrWait();
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = (gUnk_030007D8 << 4) | gUnk_030007D8;
    REG_BG0HOFS = (gUnk_03003430.bg0HOfs >> 2) & 0x1FF;
    REG_BG0VOFS = (gUnk_03003430.bg0VOfs >> 5) & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;
    REG_BG2PA = gBg2PA;
    REG_BG2PA = gBg2PA;
    REG_BG2PB = gBg2PB;
    REG_BG2PC = gBg2PC;
    REG_BG2PD = gBg2PD;
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
void VBlankCallback_MapScreen(void) {
    RenderHUDTop();
    VBlankIntrWait();
    REG_BG0HOFS = gUnk_03003430.bg0HOfs & 0x1FF;
    REG_BG0VOFS = gUnk_03003430.bg0VOfs & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;
    REG_BG2HOFS = (gUnk_03003430.bg2HOfs >> 4) & 0x1FF;
    REG_BG2VOFS = (gUnk_03003430.bg2VOfs >> 4) & 0x1FF;
    REG_BG3HOFS = (gUnk_03003430.bg3HOfs >> 4) & 0x1FF;
    REG_BG3VOFS = (gUnk_03003430.bg3VOfs >> 4) & 0x1FF;
    REG_BG2X_L = gBg2X;
    REG_BG2X_H = (gBg2X & 0x0FFF0000) >> 0x10;
    REG_BG2Y_L = gBg2Y;
    REG_BG2Y_H = (gBg2Y & 0x0FFF0000) >> 0x10;
    REG_BG2PA = gBg2PA;
    REG_BG2PB = gBg2PB;
    REG_BG2PC = gBg2PC;
    REG_BG2PD = gBg2PD;
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = (gUnk_030007D8 << 0xC) | (gUnk_030007D8 << 8) | (gUnk_030007D8 << 4) | gUnk_030007D8;
    gUnk_030034F8 = MultiplyQ8(SIN((gUnk_03004C20.unk0 * 0x10) & 0xFF), MultiplyQ8(0x200, SIN((gUnk_03004C20.unk0 * 4) & 0x7F)));
    gBg2PA = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PB = MultiplyQ8(SIN(gBg2Alpha), ReciprocalQ8(gBg2XMag));
    gBg2PC = MultiplyQ8(-SIN(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2PD = MultiplyQ8(COS(gBg2Alpha), ReciprocalQ8(gBg2YMag));
    gBg2X = ((gUnk_03003430.bg2HOfs * 0x10) - (gBg2PA * DISPLAY_WIDTH_CENTER)) - (gBg2PB * 0x78);
    gBg2Y = ((gUnk_03003430.bg2VOfs * 0x10) - (gBg2PC * 0x28)) - (gBg2PD * 0x28);
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
void VBlankCallback_GameplayWithHUD(void) {
    TransformEntityScreenPositions();
    RenderHUDTop();
    VBlankIntrWait();
    REG_BG0HOFS = (gUnk_03003430.bg0HOfs >> 2) & 0x1FF;
    REG_BG0VOFS = (gUnk_03003430.bg0VOfs >> 5) & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;
    REG_BG2X_L = gBg2X;
    REG_BG2X_H = (gBg2X & 0x0FFF0000) >> 0x10;
    REG_BG2Y_L = gBg2Y;
    REG_BG2Y_H = (gBg2Y & 0x0FFF0000) >> 0x10;
    REG_BG2PA = gBg2PA;
    REG_BG2PB = gBg2PB;
    REG_BG2PC = gBg2PC;
    REG_BG2PD = gBg2PD;
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = (gUnk_030007D8 << 0xC) | (gUnk_030007D8 << 8) | (gUnk_030007D8 << 4) | gUnk_030007D8;
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
void VBlankCallback_MinimalHW(void) {
    VBlankIntrWait();
    REG_BG0HOFS = (gUnk_03003430.bg0HOfs >> 2) & 0x1FF;
    REG_BG0VOFS = (gUnk_03003430.bg0VOfs >> 5) & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;
    REG_BG2X_L = gBg2X;
    REG_BG2X_H = (gBg2X & 0x0FFF0000) >> 0x10;
    REG_BG2Y_L = gBg2Y;
    REG_BG2Y_H = (gBg2Y & 0x0FFF0000) >> 0x10;
    REG_BG2PA = gBg2PA;
    REG_BG2PB = gBg2PB;
    REG_BG2PC = gBg2PC;
    REG_BG2PD = gBg2PD;
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = (gUnk_030007D8 << 0xC) | (gUnk_030007D8 << 8) | (gUnk_030007D8 << 4) | gUnk_030007D8;
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_0", VBlankCallback_Cutscene); /* SetupDisplayConfig */
void VBlankCallback_TitleScreen(void) {
    RenderHUDTop();
    VBlankIntrWait();
    REG_BG0HOFS = gUnk_03003430.bg0HOfs & 0x1FF;
    REG_BG0VOFS = gUnk_03003430.bg0VOfs & 0x1FF;
    REG_BG1HOFS = gUnk_03003430.bg1HOfs & 0x1FF;
    REG_BG1VOFS = gUnk_03003430.bg1VOfs & 0x1FF;
    REG_BG2HOFS = gUnk_03003430.bg2HOfs & 0x1FF;
    REG_BG2VOFS = gUnk_03003430.bg2VOfs & 0x1FF;
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = (gUnk_030007D8 << 0xC) | (gUnk_030007D8 << 8) | (gUnk_030007D8 << 4) | gUnk_030007D8;
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_0", VBlankCallback_Credits); /* TextStateMachine — master UI/text state machine */
INCLUDE_ASM("asm/nonmatchings/code_0", sub_0800DE24);
