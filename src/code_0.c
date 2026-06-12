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
 * Copies a 2-word template from 0x080E2A7C to each 8-byte OAM slot
 * at 0x03004800, then overwrites bytes 6-7 of each slot with successive
 * halfwords from the rotation/scaling table at 0x03004680.
 */
void InitOamEntries(void) {
    u32 addr1 = 0x03004680;
    register u16 *rotTable asm("r5");
    u32 addr2 = 0x080E2A7C;
    register u32 template_lo asm("r3");
    register u32 template_hi asm("r4");
    u32 addr3 = 0x03004800;
    register u8 *oam asm("r1");
    register s32 i asm("r2");
    asm("" : "=r"(rotTable) : "0"(addr1));
    {
        u32 *tmpl;
        asm("" : "=r"(tmpl) : "0"(addr2));
        template_lo = tmpl[0];
        template_hi = tmpl[1];
    }
    asm("" : "=r"(oam) : "0"(addr3));
    i = 0x7F;
    do {
        *(u32 *)(oam + 0) = template_lo;
        *(u32 *)(oam + 4) = template_hi;
        *(u16 *)(oam + 6) = *rotTable;
        rotTable++;
        oam += 8;
        i--;
    } while (i >= 0);
}
INCLUDE_ASM("asm/nonmatchings/code_0", TransformEntityScreenPositions);
INCLUDE_ASM("asm/nonmatchings/code_0", TransformSingleEntityToScreen);
INCLUDE_ASM("asm/nonmatchings/code_0", TransformAllEntitiesToScreen);
INCLUDE_ASM("asm/nonmatchings/code_0", HandlePauseMenuInput);
INCLUDE_ASM("asm/nonmatchings/code_0", UpdateUIState); /* UpdateUIState */
INCLUDE_ASM("asm/nonmatchings/code_0", RenderCharacterTiles); /* RenderCharacterTiles */
INCLUDE_ASM("asm/nonmatchings/code_0", UpdateTextScroll); /* UpdateTextScroll */
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
