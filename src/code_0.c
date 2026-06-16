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
extern void SaveGameWithVerify(s32, s32); /* SaveGameWithVerify */
extern void ReadKeyInput(void); /* ReadKeyInput == kleod InputHandler_Normal */
extern void ProcessInputAndTimers(void); /* ProcessInputAndTimers == kleod InputHandler_AttractMode */
extern void InitLevelState(void); /* InitLevelState */
extern void CameraModeSwitchHandler(void); /* CameraModeSwitchHandler */
extern void UpdateUIState(void); /* UpdateUIState */
extern void IntroScrollAnimation(void); /* IntroScrollAnimation */
extern void AnimatePaletteEffects(void); /* AnimatePaletteEffects */
extern void HandlePauseMenuInput(void); /* HandlePauseMenuInput */
extern void UpdateCameraScroll(void); /* UpdateCameraScroll */
extern void UpdateCameraScrollPlayer2(void); /* UpdateCameraScrollPlayer2 */
extern void ProcessOamSpriteLayout(void); /* ProcessOamSpriteLayout */
extern void VBlankCallback_Gameplay(void); /* VBlankCallback_Gameplay */
extern void TransitionInitLevelMusic(void); /* sub_080242C0 (off-by-2 in our split) */

INCLUDE_ASM("asm/nonmatchings/code_0", SetupOAMSprite); /* DrawSpriteTiles — core sprite/tile VRAM writer */
extern void InitOamEntries(void); /* InitOamEntries — used by RenderHUDTop */
void RenderHUDTop(void) {
    s32 sp0;
    s32 sp4;
    struct Unk_0300466C_4 *var_r5;
    struct Unk_0300466C_4 *var_r5_2;
    struct Unk_0300466C_4 *var_r6;
    struct Unk_0300466C_4 *var_r6_2;
    struct Unk_0300466C_4 *var_r6_3;
    struct Unk_0300466C_4 *var_r6_4;
    struct Unk_0300466C_4 *var_r6_5;
    u8 temp_r0_4;
    u8 temp_r1;
    u8 temp_r1_12;
    u8 temp_r1_13;
    u8 temp_r1_17;
    u8 temp_r1_18;
    u8 temp_r1_22;
    u8 temp_r1_25;
    u8 temp_r1_26;
    u8 temp_r1_29;
    u8 temp_r1_30;
    u8 temp_r1_4;
    u8 temp_r1_8;
    u8 temp_r1_9;
    s32 var_sb;
    s32 var_sb_2;
    s32 var_sb_3;
    s32 var_sb_4;
    s32 var_sb_5;
    s32 var_sl;

    InitOamEntries();
    gUnk_03000820 = gUnk_03004800;

    if (gUnk_03002920[0xB].unk10 == 1) {
        temp_r1_29 = gUnk_03002920[0xB].unkA;
        if (gUnk_03002920[0xB].unkA < 0xD) {
            gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[0xB].unkA];
        } else {
            gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[0xB].unkA - 0xD];
        }

        temp_r1 = gUnk_0300466C->unk0;
        var_r5 = gUnk_0300466C->unk4;
        for (var_sl = 0; var_sl < temp_r1; var_sl++) {
            if (gUnk_03002920[0xB].affineDouble) {
                if (gUnk_03002920[0xB].unk11 == 0x1C) {
                    gUnk_03000820->split.x
                        = (var_r5->unk3 * 2) + gUnk_03002920[0xB].xPosScreen + ((s32)(gUnk_03002920[0xB].unkB << 0x1C) >> 0x18);
                    gUnk_03000820->split.y = var_r5->unk4 + ((s8)var_r5->unk4 >> 1) + gUnk_03002920[0xB].yPosScreen
                        + (((s8)gUnk_03002920[0xB].unkB >> 4) << 4);
                } else {
                    gUnk_03000820->split.x
                        = (var_r5->unk3 * 2) + gUnk_03002920[0xB].xPosScreen + ((s32)(gUnk_03002920[0xB].unkB << 0x1C) >> 0x1C);
                    gUnk_03000820->split.y = var_r5->unk4 + ((s8)var_r5->unk4 >> 1) + gUnk_03002920[0xB].yPosScreen
                        + ((s8)(gUnk_03002920[0xB].unkB) >> 0x4);
                }
            } else {
                gUnk_03000820->split.x
                    = var_r5->unk3 + gUnk_03002920[0xB].xPosScreen + ((s32)(gUnk_03002920[0xB].unkB << 0x1C) >> 0x1C);
                gUnk_03000820->split.y = gUnk_03002920[0xB].yPosScreen + var_r5->unk4 + ((s8)(gUnk_03002920[0xB].unkB) >> 0x4);
            }

            gUnk_03000820->split.bpp = var_r5->bpp_paletteNum >> 7;
            gUnk_03000820->split.tileNum = var_r5->tileNum;
            gUnk_03000820->split.paletteNum = var_r5->bpp_paletteNum & 0x7F;
            gUnk_03000820->split.shape = (var_r5->shape_size & 0xC) >> 2;
            gUnk_03000820->split.size = var_r5->shape_size & 3;

            gUnk_03000820->split.priority = gUnk_03002920[0xB].priority;
            gUnk_03000820->split.objMode = gUnk_03002920[0xB].objMode;
            gUnk_03000820->split.affineMode = (gUnk_03002920[0xB].affineDouble << 1) | gUnk_03002920[0xB].affineEnable;
            gUnk_03000820->split.matrixNum = gUnk_03002920[0xB].affineHFlip_matrixNum;

            if (gUnk_03002920[sp0].affineEnable) {
                gUnk_03000820->split.hFlip = gUnk_03002920[0xB].affineHFlip_matrixNum >> 3;
            } else {
                gUnk_03000820->split.hFlip = gUnk_03002920[0xB].unkC_2 & 1;
                gUnk_03000820->split.vFlip = gUnk_03002920[0xB].unkC_2 >> 1;
            }

            gUnk_03000820 += 1;
            var_r5 += 1;
        }
    }

    if (gUnk_03002920[0xC].unk10 == 1) {
        temp_r1_30 = gUnk_03002920[0xC].unkA;
        if (gUnk_03002920[0xC].unkA < 0xD) {
            gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[0xC].unkA];
        } else {
            gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[0xC].unkA - 0xD];
        }

        temp_r1_4 = gUnk_0300466C->unk0;
        var_r5_2 = gUnk_0300466C->unk4;
        for (sp4 = 0; sp4 < temp_r1_4; sp4++) {
            if (gUnk_03002920[0xC].affineDouble) {
                if (gUnk_03002920[0xC].unk11 == 0x1C) {
                    gUnk_03000820->split.x
                        = (var_r5_2->unk3 * 2) + gUnk_03002920[0xC].xPosScreen + ((s32)(gUnk_03002920[0xC].unkB << 0x1C) >> 0x18);
                    gUnk_03000820->split.y = var_r5_2->unk4 + ((s8)var_r5_2->unk4 >> 1) + gUnk_03002920[0xC].yPosScreen
                        + (((s8)gUnk_03002920[0xC].unkB >> 4) << 4);
                } else {
                    gUnk_03000820->split.x
                        = (var_r5_2->unk3 * 2) + gUnk_03002920[0xC].xPosScreen + ((s32)(gUnk_03002920[0xC].unkB << 0x1C) >> 0x1C);
                    gUnk_03000820->split.y = var_r5_2->unk4 + ((s8)var_r5_2->unk4 >> 1) + gUnk_03002920[0xC].yPosScreen
                        + ((s8)(gUnk_03002920[0xC].unkB) >> 0x4);
                }
            } else {
                gUnk_03000820->split.x
                    = var_r5_2->unk3 + gUnk_03002920[0xC].xPosScreen + ((s32)(gUnk_03002920[0xC].unkB << 0x1C) >> 0x1C);
                gUnk_03000820->split.y = gUnk_03002920[0xC].yPosScreen + var_r5_2->unk4 + ((s8)(gUnk_03002920[0xC].unkB) >> 0x4);
            }

            gUnk_03000820->split.bpp = var_r5_2->bpp_paletteNum >> 7;
            gUnk_03000820->split.tileNum = var_r5_2->tileNum;
            gUnk_03000820->split.paletteNum = var_r5_2->bpp_paletteNum & 0x7F;
            gUnk_03000820->split.shape = (var_r5_2->shape_size & 0xC) >> 2;
            gUnk_03000820->split.size = var_r5_2->shape_size & 3;

            gUnk_03000820->split.priority = gUnk_03002920[0xC].priority;
            gUnk_03000820->split.objMode = gUnk_03002920[0xC].objMode;
            gUnk_03000820->split.affineMode = (gUnk_03002920[0xC].affineDouble << 1) | gUnk_03002920[0xC].affineEnable;
            gUnk_03000820->split.matrixNum = gUnk_03002920[0xC].affineHFlip_matrixNum;

            if (gUnk_03002920[sp0].affineEnable) {
                gUnk_03000820->split.hFlip = gUnk_03002920[0xC].affineHFlip_matrixNum >> 3;
            } else {
                gUnk_03000820->split.hFlip = gUnk_03002920[0xC].unkC_2 & 1;
                gUnk_03000820->split.vFlip = gUnk_03002920[0xC].unkC_2 >> 1;
            }

            gUnk_03000820 += 1;
            var_r5_2 += 1;
        }
    }

    for (sp0 = 1; sp0 <= 8; sp0++) {
        if (gUnk_03002920[sp0].unk10 == 1) {
            if (gUnk_03000830[sp0].unk0 != 1) {
                temp_r1_8 = gUnk_03002920[sp0].unkA;
                if (gUnk_03002920[sp0].unkA < 0xD) {
                    gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[sp0].unkA];
                } else {
                    gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[sp0].unkA - 0xD];
                }

                temp_r1_9 = gUnk_0300466C->unk0;
                var_r6 = gUnk_0300466C->unk4;
                for (var_sb = 0; var_sb < temp_r1_9; var_sb++) {
                    if (gUnk_03002920[sp0].affineDouble) {
                        if (gUnk_03002920[sp0].unk11 == 0x1C) {
                            gUnk_03000820->split.x = (var_r6->unk3 * 2) + gUnk_03002920[sp0].xPosScreen
                                + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x18);
                            gUnk_03000820->split.y = var_r6->unk4 + ((s8)var_r6->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                                + (((s8)gUnk_03002920[sp0].unkB >> 4) << 4);
                        } else {
                            gUnk_03000820->split.x = (var_r6->unk3 * 2) + gUnk_03002920[sp0].xPosScreen
                                + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                            gUnk_03000820->split.y = var_r6->unk4 + ((s8)var_r6->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                                + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                        }
                    } else {
                        gUnk_03000820->split.x
                            = var_r6->unk3 + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y = gUnk_03002920[sp0].yPosScreen + var_r6->unk4 + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                    }

                    gUnk_03000820->split.bpp = var_r6->bpp_paletteNum >> 7;
                    gUnk_03000820->split.tileNum = var_r6->tileNum;
                    gUnk_03000820->split.paletteNum = var_r6->bpp_paletteNum & 0x7F;
                    gUnk_03000820->split.shape = (var_r6->shape_size & 0xC) >> 2;
                    gUnk_03000820->split.size = var_r6->shape_size & 3;

                    gUnk_03000820->split.priority = gUnk_03002920[sp0].priority;
                    gUnk_03000820->split.objMode = gUnk_03002920[sp0].objMode;
                    gUnk_03000820->split.affineMode = (gUnk_03002920[sp0].affineDouble << 1) | gUnk_03002920[sp0].affineEnable;
                    gUnk_03000820->split.matrixNum = gUnk_03002920[sp0].affineHFlip_matrixNum;

                    if (gUnk_03002920[sp0].affineEnable) {
                        gUnk_03000820->split.hFlip = gUnk_03002920[sp0].affineHFlip_matrixNum >> 3;
                    } else {
                        gUnk_03000820->split.hFlip = gUnk_03002920[sp0].unkC_2 & 1;
                        gUnk_03000820->split.vFlip = gUnk_03002920[sp0].unkC_2 >> 1;
                    }

                    gUnk_03000820 += 1;
                    var_r6 += 1;
                }
            }

            if (gUnk_03000830[sp0].unk0 == 7) {
                temp_r1_12 = gUnk_03002920[sp0].unkA;
                if (gUnk_03002920[sp0].unkA < 0xD) {
                    gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[sp0].unkA];
                } else {
                    gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[sp0].unkA - 0xD];
                }

                temp_r1_13 = gUnk_0300466C->unk0;
                var_r6_2 = gUnk_0300466C->unk4;
                for (var_sb_2 = 0; var_sb_2 < temp_r1_13; var_sb_2++) {
                    if (gUnk_03002920[sp0].affineDouble) {
                        if (gUnk_03002920[sp0].unk11 == 0x1C) {
                            gUnk_03000820->split.x = (var_r6_2->unk3 * 2) + gUnk_03002920[sp0].xPosScreen
                                + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x18);
                            gUnk_03000820->split.y = var_r6_2->unk4 + ((s8)var_r6_2->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                                + (((s8)gUnk_03002920[sp0].unkB >> 4) << 4);
                        } else {
                            gUnk_03000820->split.x = (var_r6_2->unk3 * 2) + gUnk_03002920[sp0].xPosScreen
                                + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                            gUnk_03000820->split.y = var_r6_2->unk4 + ((s8)var_r6_2->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                                + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                        }
                    } else {
                        gUnk_03000820->split.x
                            = var_r6_2->unk3 + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y
                            = gUnk_03002920[sp0].yPosScreen + var_r6_2->unk4 + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                    }

                    gUnk_03000820->split.bpp = var_r6_2->bpp_paletteNum >> 7;
                    gUnk_03000820->split.tileNum = var_r6_2->tileNum;
                    gUnk_03000820->split.paletteNum = var_r6_2->bpp_paletteNum & 0x7F;
                    gUnk_03000820->split.shape = (var_r6_2->shape_size & 0xC) >> 2;
                    gUnk_03000820->split.size = var_r6_2->shape_size & 3;

                    gUnk_03000820->split.priority = gUnk_03002920[sp0].priority;
                    gUnk_03000820->split.objMode = gUnk_03002920[sp0].objMode;
                    gUnk_03000820->split.affineMode = (gUnk_03002920[sp0].affineDouble << 1) | gUnk_03002920[sp0].affineEnable;
                    gUnk_03000820->split.matrixNum = gUnk_03002920[sp0].affineHFlip_matrixNum;

                    if (gUnk_03002920[sp0].affineEnable) {
                        gUnk_03000820->split.hFlip = gUnk_03002920[sp0].affineHFlip_matrixNum >> 3;
                    } else {
                        gUnk_03000820->split.hFlip = gUnk_03002920[sp0].unkC_2 & 1;
                        gUnk_03000820->split.vFlip = gUnk_03002920[sp0].unkC_2 >> 1;
                    }

                    gUnk_03000820 += 1;
                    var_r6_2 += 1;
                }
            }
        }
    }

    for (sp0 = 0; sp0 <= 0xC; sp0++) {
        if (gUnk_03002920[sp0].unk11 == 0x34)
            continue;

        if (sp0 == 0xB || sp0 == 0xC)
            continue;

        if (gUnk_03002920[sp0].unk10 == 1) {
            temp_r1_17 = gUnk_03002920[sp0].unkA;
            if (gUnk_03002920[sp0].unkA < 0xD) {
                gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[sp0].unkA];
            } else {
                gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[sp0].unkA - 0xD];
            }

            temp_r1_18 = gUnk_0300466C->unk0;
            var_r6_3 = gUnk_0300466C->unk4;
            for (var_sb_3 = 0; var_sb_3 < temp_r1_18; var_sb_3++) {
                if (gUnk_03002920[sp0].affineDouble) {
                    if (gUnk_03002920[sp0].unk11 == 0x1C) {
                        gUnk_03000820->split.x
                            = (var_r6_3->unk3 * 2) + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x18);
                        gUnk_03000820->split.y = var_r6_3->unk4 + ((s8)var_r6_3->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                            + (((s8)gUnk_03002920[sp0].unkB >> 4) << 4);
                    } else {
                        gUnk_03000820->split.x
                            = (var_r6_3->unk3 * 2) + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y = var_r6_3->unk4 + ((s8)var_r6_3->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                            + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                    }
                } else {
                    gUnk_03000820->split.x
                        = var_r6_3->unk3 + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                    gUnk_03000820->split.y = gUnk_03002920[sp0].yPosScreen + var_r6_3->unk4 + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                }

                gUnk_03000820->split.bpp = var_r6_3->bpp_paletteNum >> 7;
                gUnk_03000820->split.tileNum = var_r6_3->tileNum;
                gUnk_03000820->split.paletteNum = var_r6_3->bpp_paletteNum & 0x7F;
                gUnk_03000820->split.shape = (var_r6_3->shape_size & 0xC) >> 2;
                gUnk_03000820->split.size = var_r6_3->shape_size & 3;

                gUnk_03000820->split.priority = gUnk_03002920[sp0].priority;
                gUnk_03000820->split.objMode = gUnk_03002920[sp0].objMode;
                gUnk_03000820->split.affineMode = (gUnk_03002920[sp0].affineDouble << 1) | gUnk_03002920[sp0].affineEnable;
                gUnk_03000820->split.matrixNum = gUnk_03002920[sp0].affineHFlip_matrixNum;

                if (gUnk_03002920[sp0].affineEnable) {
                    gUnk_03000820->split.hFlip = gUnk_03002920[sp0].affineHFlip_matrixNum >> 3;
                } else {
                    gUnk_03000820->split.hFlip = gUnk_03002920[sp0].unkC_2 & 1;
                    gUnk_03000820->split.vFlip = gUnk_03002920[sp0].unkC_2 >> 1;
                }

                gUnk_03000820 += 1;
                var_r6_3 += 1;
            }
        }
    }

    for (sp0 = 0xD; sp0 < gUnk_03005428; sp0++) {
        if (gUnk_03002920[sp0].unk10 == 1) {
            temp_r0_4 = gUnk_03002920[sp0].unkA;
            if (gUnk_03002920[sp0].unkA < 0xD) {
                gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[sp0].unkA];
            } else {
                gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[sp0].unkA - 0xD];
            }

            temp_r1_22 = gUnk_0300466C->unk0;
            var_r6_4 = gUnk_0300466C->unk4;
            for (var_sb_4 = 0; var_sb_4 < temp_r1_22; var_sb_4++) {
                if (gUnk_03002920[sp0].affineDouble) {
                    if (gUnk_03002920[sp0].unk11 == 0x1C) {
                        gUnk_03000820->split.x
                            = (var_r6_4->unk3 * 2) + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x18);
                        gUnk_03000820->split.y = var_r6_4->unk4 + ((s8)var_r6_4->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                            + (((s8)gUnk_03002920[sp0].unkB >> 4) << 4);
                    } else {
                        gUnk_03000820->split.x
                            = (var_r6_4->unk3 * 2) + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y = var_r6_4->unk4 + ((s8)var_r6_4->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                            + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                    }
                } else {
                    gUnk_03000820->split.x
                        = var_r6_4->unk3 + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                    gUnk_03000820->split.y = gUnk_03002920[sp0].yPosScreen + var_r6_4->unk4 + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                }

                gUnk_03000820->split.bpp = var_r6_4->bpp_paletteNum >> 7;
                gUnk_03000820->split.tileNum = var_r6_4->tileNum;
                gUnk_03000820->split.paletteNum = var_r6_4->bpp_paletteNum & 0x7F;
                gUnk_03000820->split.shape = (var_r6_4->shape_size & 0xC) >> 2;
                gUnk_03000820->split.size = var_r6_4->shape_size & 3;

                gUnk_03000820->split.priority = gUnk_03002920[sp0].priority;
                gUnk_03000820->split.objMode = gUnk_03002920[sp0].objMode;
                gUnk_03000820->split.affineMode = (gUnk_03002920[sp0].affineDouble << 1) | gUnk_03002920[sp0].affineEnable;
                gUnk_03000820->split.matrixNum = gUnk_03002920[sp0].affineHFlip_matrixNum;

                if (gUnk_03002920[sp0].affineEnable) {
                    gUnk_03000820->split.hFlip = gUnk_03002920[sp0].affineHFlip_matrixNum >> 3;
                } else {
                    gUnk_03000820->split.hFlip = gUnk_03002920[sp0].unkC_2 & 1;
                    gUnk_03000820->split.vFlip = gUnk_03002920[sp0].unkC_2 >> 1;
                }

                gUnk_03000820 += 1;
                var_r6_4 += 1;
            }
        }
    }

    for (sp0 = 1; sp0 <= 8; sp0++) {
        if (gUnk_03002920[sp0].unk10 == 1) {
            if (gUnk_03000830[sp0].unk0 == 1) {
                temp_r1_25 = gUnk_03002920[sp0].unkA;
                if (gUnk_03002920[sp0].unkA < 0xD) {
                    gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[sp0].unkA];
                } else {
                    gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[sp0].unkA - 0xD];
                }

                temp_r1_26 = gUnk_0300466C->unk0;
                var_r6_5 = gUnk_0300466C->unk4;
                for (var_sb_5 = 0; var_sb_5 < temp_r1_26; var_sb_5++) {
                    if (gUnk_03002920[sp0].affineDouble) {
                        if (gUnk_03002920[sp0].unk11 == 0x1C) {
                            gUnk_03000820->split.x = (var_r6_5->unk3 * 2) + gUnk_03002920[sp0].xPosScreen
                                + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x18);
                            gUnk_03000820->split.y = var_r6_5->unk4 + ((s8)var_r6_5->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                                + (((s8)gUnk_03002920[sp0].unkB >> 4) << 4);
                        } else {
                            gUnk_03000820->split.x = (var_r6_5->unk3 * 2) + gUnk_03002920[sp0].xPosScreen
                                + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                            gUnk_03000820->split.y = var_r6_5->unk4 + ((s8)var_r6_5->unk4 >> 1) + gUnk_03002920[sp0].yPosScreen
                                + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                        }
                    } else {
                        gUnk_03000820->split.x
                            = var_r6_5->unk3 + gUnk_03002920[sp0].xPosScreen + ((s32)(gUnk_03002920[sp0].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y
                            = gUnk_03002920[sp0].yPosScreen + var_r6_5->unk4 + ((s8)(gUnk_03002920[sp0].unkB) >> 0x4);
                    }

                    gUnk_03000820->split.bpp = var_r6_5->bpp_paletteNum >> 7;
                    gUnk_03000820->split.tileNum = var_r6_5->tileNum;
                    gUnk_03000820->split.paletteNum = var_r6_5->bpp_paletteNum & 0x7F;
                    gUnk_03000820->split.shape = (var_r6_5->shape_size & 0xC) >> 2;
                    gUnk_03000820->split.size = var_r6_5->shape_size & 3;

                    gUnk_03000820->split.priority = gUnk_03002920[sp0].priority;
                    gUnk_03000820->split.objMode = gUnk_03002920[sp0].objMode;
                    gUnk_03000820->split.affineMode = (gUnk_03002920[sp0].affineDouble << 1) | gUnk_03002920[sp0].affineEnable;
                    gUnk_03000820->split.matrixNum = gUnk_03002920[sp0].affineHFlip_matrixNum;

                    if (gUnk_03002920[sp0].affineEnable) {
                        gUnk_03000820->split.hFlip = gUnk_03002920[sp0].affineHFlip_matrixNum >> 3;
                    } else {
                        gUnk_03000820->split.hFlip = gUnk_03002920[sp0].unkC_2 & 1;
                        gUnk_03000820->split.vFlip = gUnk_03002920[sp0].unkC_2 >> 1;
                    }

                    gUnk_03000820 += 1;
                    var_r6_5 += 1;
                }
            }
        }
    }
}
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
/**
 * TransformSingleEntityToScreen: project entity arg0's BG2 position into
 * screen space, applying the BG2 affine inverse (mag) and a per-sprite Y
 * offset selected by the sprite's shape_size class.
 */
void TransformSingleEntityToScreen(u8 arg0, s8 arg1, s8 arg2) {
    s32 temp_r0;
    s32 temp_r0_2;
    s16 temp_r1;
    s16 temp_r2;
    s32 var_r2;
    s32 var_r4;
    struct Unk_0300466C_4 *var_r0;

    arg1++, arg1--; /* fake */
    gUnk_03002920[arg0].xPosScreen = gUnk_03002920[arg0].xPosBg2 - arg1 - gUnk_03003430.bg2HOfs;
    gUnk_03002920[arg0].yPosScreen = gUnk_03002920[arg0].yPosBg2 - arg2 - gUnk_03003430.bg2VOfs;

    temp_r1 = gUnk_03003430.bg2HOfs - gUnk_03002920[arg0].xPosScreen;
    temp_r2 = gUnk_03003430.bg2VOfs - gUnk_03002920[arg0].yPosScreen;

    temp_r0 = temp_r1 * gBg2XMag;
    if (temp_r0 < 0) {
        temp_r0 += 0xFF;
    }
    var_r4 = temp_r0 >> 8;

    temp_r0_2 = temp_r2 * gBg2YMag;
    if (temp_r0_2 < 0) {
        temp_r0_2 += 0xFF;
    }
    var_r2 = temp_r0_2 >> 8;

    var_r4 = gUnk_03003430.bg2HOfs - var_r4;
    if (arg0 > 0xC) {
        var_r0 = (void *)gUnk_030051DC[arg0 - 0xD].unk4;
    } else {
        var_r0 = gUnk_08078FC8[arg0].unk4;
    }

    switch (var_r0->shape_size & 0xF) {
        case 3:
        case 11:
            var_r2 = gUnk_03003430.bg2VOfs - var_r2 + ((0x100 - gBg2YMag) >> 3);
            break;
        case 1:
        case 6:
        case 8:
            var_r2 = gUnk_03003430.bg2VOfs - var_r2 + ((0x100 - gBg2YMag) >> 5);
            break;
        case 0:
        case 4:
        case 5:
            var_r2 = gUnk_03003430.bg2VOfs - var_r2 + ((0x100 - gBg2YMag) >> 6);
            break;
        default:
            var_r2 = gUnk_03003430.bg2VOfs - var_r2 + ((0x100 - gBg2YMag) >> 4);
            break;
    }

    gUnk_03002920[arg0].xPosScreen = var_r4;
    gUnk_03002920[arg0].yPosScreen = var_r2;
}
/**
 * TransformAllEntitiesToScreen: project every active entity into screen
 * space.  Entities 0..0xA always get the affine transform; entities 0xD..
 * gUnk_03005428 are filtered by their unkF (sprite type) and affineEnable
 * fields, with bounds-based culling that sets unk10 (visible flag).
 */
void TransformAllEntitiesToScreen(s8 arg0, s8 arg1) {
    s32 var_r5;

    for (var_r5 = 0; var_r5 < 0xB; var_r5++) {
        TransformSingleEntityToScreen(var_r5, arg0, arg1);
    }

    for (var_r5 = 0xD; var_r5 < gUnk_03005428; var_r5++) {
        if (gUnk_03002920[var_r5].unkF == 0x1A) {
            gUnk_03002920[var_r5].unk10 = 0;
            continue;
        }

        if (gUnk_03002920[var_r5].affineEnable == 1) {
            if (gUnk_03002920[var_r5].unkF < 0x19) {
                TransformSingleEntityToScreen(var_r5, arg0, arg1);
                if (gUnk_03002920[var_r5].unk11 != 0) {
                    if ((gUnk_03002920[var_r5].xPosScreen >= (DISPLAY_WIDTH + 67) && gUnk_03002920[var_r5].xPosScreen <= (u16)(-68))
                        || (gUnk_03002920[var_r5].yPosScreen >= (DISPLAY_HEIGHT + 96)
                            && gUnk_03002920[var_r5].yPosScreen <= (u16)(-68))) {
                        gUnk_03002920[var_r5].unk10 = 0;
                    } else {
                        gUnk_03002920[var_r5].unk10 = 1;
                    }
                }
            }
        } else {
            if (gUnk_03002920[var_r5].unkF == 0x1C) {
                gUnk_03002920[var_r5].unk10 = 0;
            } else {
                gUnk_03002920[var_r5].xPosScreen = gUnk_03002920[var_r5].xPosBg2;
                gUnk_03002920[var_r5].yPosScreen = gUnk_03002920[var_r5].yPosBg2;
                gUnk_03002920[var_r5].unk10 = 1;
            }
        }
    }
}
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
/**
 * InitLevelGameplay (InitLevelGameplay): set up the per-tick gameplay
 * dispatch for the current world/level — resolves a ROM config entry
 * from gUnk_080D821C, seeds the player's initial BG2 position from
 * the per-world (level==8) or per-room (other) ROM table, populates
 * gUnk_03005220 / gUnk_03005284, and writes gUnk_03003510.unk28/
 * unk34/unk38/unk3C/unk40 for either the boss/cutscene flow or the
 * normal gameplay loop.
 */
void InitLevelGameplay(u32 arg0) {
    u32 var_r4;

    gUnk_03003508 = 3;
    gUnk_03004C20.unkB = 0;
    gUnk_03004C20.unkA = 0;

    for (var_r4 = 0; var_r4 < 0xD; var_r4++) {
        if ((gUnk_03004C20.world == gUnk_080D821C[var_r4].unk8) && (gUnk_03004C20.level == gUnk_080D821C[var_r4].unk9)) {
            gUnk_03004D80 = &gUnk_080D821C[var_r4];
            gUnk_03004C20.unkA = 1;
            if (gUnk_03004C20.level != 8) {
                gUnk_03004C20.unkB = 1;
                gUnk_03003508 = 6;
            }
            break;
        }
    }

    gUnk_03000810 = 0;
    if (gUnk_03004C20.level == 8) {
        gUnk_03002920->xPosBg2 = gUnk_080D6458[gUnk_03004C20.world - 1].unk0;
        gUnk_03002920->yPosBg2 = gUnk_080D6458[gUnk_03004C20.world - 1].unk2;
        gUnk_03002920->unkC_2 = gUnk_080D6458[gUnk_03004C20.world - 1].unk4_0;
    } else {
        gUnk_03002920->xPosBg2
            = gUnk_080D48C8[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_030051C8 - (gUnk_03004654[1] - 1)].unk0;
        gUnk_03002920->yPosBg2
            = gUnk_080D48C8[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_030051C8 - (gUnk_03004654[1] - 1)].unk2;
        gUnk_03002920->unkC_2
            = gUnk_080D48C8[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][gUnk_030051C8 - (gUnk_03004654[1] - 1)].unk4_0;
    }

    if (arg0 == 0) {
        gUnk_03005284->unk6 = 0;
        gUnk_03005284->unk1 = gUnk_03004C20.world;
        gUnk_03005284->unk2 = gUnk_03004C20.level;
        gUnk_03005284->unk16 = 0;
        if (gUnk_03003410.unkA == 0) {
            SaveGameWithVerify(0, 1);
            SaveGameWithVerify(1, 0);
        }
        gUnk_03005220.unk0_2 = 0;
        gUnk_03005220.unk0_5 = 0;
        gUnk_03005220.unk0_0 = 3;
        gUnk_03005220.unk1_4 = 0;
        gUnk_03005220.unk14 = 0;
        gUnk_03005220.unk3_6 = gUnk_03005220.unk3_5 = 0;
        if ((gUnk_03004C20.unkB != 0) || ((gUnk_03004C20.world == 6) && ((gUnk_03004C20.level == 1) || (gUnk_03004C20.level == 3)))) {
            gUnk_03005220.unk4 = gUnk_03005284->unk18;
        } else {
            gUnk_03005220.unk4 = 0;
        }
        gUnk_03005220.unk8 = 0;
        gUnk_03005220.unkC = 0;
        gUnk_03005220.unk2_7 = 0;
        gUnk_03005220.unk2E = 0;
        gUnk_03005220.unk58 = 0;
        gUnk_03005220.unk1_7 = 0;
        gUnk_03005220.unk60 = 0;
        gUnk_03005220.unk4F = 0;
        gUnk_03005220.unk4E = 0;
        gUnk_03005220.unk4D = 0;
        gUnk_03004C20.unk8 = 0;
        gUnk_03005220.unk1C = 0;
        gUnk_03005220.unk5E = 0;
    }

    if (arg0 == 1) {
        gUnk_03005220.unk4C = gUnk_03005284->unk0;
        gUnk_03005220.unk0_0 = gUnk_03005284->unk8_0;
        gUnk_03005220.unk0_2 = gUnk_03005284->unk8_2;
        gUnk_03005220.unk0_5 = gUnk_03005284->unk8_5;
        gUnk_03005220.unk1_4 = gUnk_03005284->unk9_4;
        gUnk_03005220.unk4 = gUnk_03005284->unk18;
        gUnk_03005220.unk2_7 = gUnk_03005284->unkA_7;
        gUnk_03005220.unk3_5 = gUnk_03005284->unkB_5;
        gUnk_03005220.unk3_6 = gUnk_03005284->unkB_6;
        gUnk_03005220.unk8 = gUnk_03005284->unkC;
        gUnk_03005220.unkC = gUnk_03005284->unk10;
        gUnk_03005220.unk14 = gUnk_03005284->unk14;
        gUnk_03005220.unk2E = gUnk_03005284->unk5;
        gUnk_03005220.unk58 = gUnk_03005284->unk7;
        do {
            gUnk_03005220.unk1_7 = gUnk_03005284->unk9_7;
            gUnk_03004C20.unk8 = gUnk_03005284->unk16;
            gUnk_03005220.unk60 = 0;
            gUnk_03005220.unk4F = 0;
            gUnk_03005220.unk4E = 0;
            gUnk_03005220.unk4D = 0;
        } while (0);
    } else {
        gUnk_03005284->unk0 = gUnk_03005220.unk4C;
        gUnk_03005284->unk1 = gUnk_03004C20.world;
        gUnk_03005284->unk2 = gUnk_03004C20.level;
        gUnk_03005284->unk8_0 = gUnk_03005220.unk0_0;
        gUnk_03005284->unk16 = gUnk_03004C20.unk8;
        gUnk_03005284->unk8_2 = gUnk_03005220.unk0_2;
        gUnk_03005284->unk9_4 = gUnk_03005220.unk1_4;
        gUnk_03005284->unk14 = gUnk_03005220.unk14;
        gUnk_03005284->unkB_5 = gUnk_03005220.unk3_5;
        gUnk_03005284->unkB_6 = gUnk_03005220.unk3_6;
        gUnk_03005284->unk5 = gUnk_03005220.unk2E;
        gUnk_03005284->unk7 = gUnk_03005220.unk58;
        gUnk_03005284->unk9_7 = gUnk_03005220.unk1_7;
        gUnk_03005284->unk18 = gUnk_03005220.unk4;
        if (gUnk_03004C20.unkB == 0) {
            gUnk_03005284->unk8_5 = gUnk_03005220.unk0_5;
            gUnk_03005284->unkC = gUnk_03005220.unk8;
            gUnk_03005284->unk10 = gUnk_03005220.unkC;
            gUnk_03005284->unkA_7 = gUnk_03005220.unk2_7;
        } else {
            gUnk_03005284->unk8_5 = 0;
            gUnk_03005284->unkC = 0;
            gUnk_03005284->unk10 = 0;
            gUnk_03005284->unkA_7 = 0;
        }
    }

    if (gUnk_03003410.unkA == 0) {
        gUnk_03003510.unk28[0] = ReadKeyInput;
    } else {
        gUnk_03003510.unk28[0] = ProcessInputAndTimers;
    }

    gUnk_03003410.unk5 = 0;
    gUnk_03003410.unk0 = 0;
    gUnk_03003410.unkB = 0;
    gUnk_030051E0 = 0;
    gUnk_030034C4 = 0xFE;
    gUnk_03003430.unk46 = 0;
    gUnk_03003430.unk44 = 0;

    if (gUnk_03004C20.level == 8) {
        gUnk_03003510.unk28[1] = InitLevelState;
        gUnk_03003510.unk28[2] = CameraModeSwitchHandler;
        gUnk_03003510.unk34 = UpdateUIState;
        gUnk_03003510.unk38 = TransitionInitLevelMusic;
        gUnk_03003510.unk3C = (u32)IntroScrollAnimation;
        gUnk_03003510.unk40 = AnimatePaletteEffects;
        gUnk_03003510.unk44 = 1;
        gUnk_03003510.unk0[gUnk_03003510.unk78 - 1] = NULL;
        gUnk_03003510.unk79 = 8;
    } else {
        gUnk_03003510.unk28[1] = HandlePauseMenuInput;
        if (gUnk_03004C20.unkB == 1) {
            gUnk_03003510.unk28[2] = UpdateCameraScrollPlayer2;
        } else if (gUnk_03004C20.level == 6) {
            gUnk_03003510.unk28[2] = UpdateCameraScroll;
        } else {
            gUnk_03003510.unk28[2] = ProcessOamSpriteLayout;
        }
        gUnk_03003510.unk34 = TransitionInitLevelMusic;

        if (arg0 < 2) {
            if (gUnk_03003410.unkA == 0) {
                gUnk_03003510.unk38 = IntroScrollAnimation;
                gUnk_03003510.unk3C = (u32)VBlankCallback_Gameplay;
                gUnk_03003510.unk40 = (IntrFunc)1;
                gUnk_03003510.unk0[gUnk_03003510.unk78 - 1] = NULL;
                gUnk_03003510.unk79 = 7;
            } else {
                gUnk_03003510.unk38 = VBlankCallback_Gameplay;
                gUnk_03003510.unk3C = 1;
                gUnk_03003510.unk0[gUnk_03003510.unk78 - 1] = NULL;
                gUnk_03003510.unk79 = 6;
            }
        } else {
            gUnk_03003410.unk5 = 1;
            gUnk_03003510.unk38 = VBlankCallback_Gameplay;
            gUnk_03003510.unk3C = 1;
            gUnk_03003510.unk0[gUnk_03003510.unk78 - 1] = NULL;
            gUnk_03003510.unk79 = 6;
        }
    }
    gUnk_030034E4 = 1;
    if (gUnk_03004C20.level == 6) {
        gUnk_030034E8.unk0 = gUnk_080D89A8[gUnk_03004C20.world - 1][gUnk_03004C20.room - 1].unk0;
        gUnk_030034E8.unk4 = gUnk_080D89A8[gUnk_03004C20.world - 1][gUnk_03004C20.room - 1].unk4;
        gUnk_030051B8 = 0;

        if (gUnk_030034E8.unk0 > 0) {
            gUnk_030051B8 = 0x10;
        } else if (gUnk_030034E8.unk0 < 0) {
            gUnk_030051B8 = 0x20;
        }

        if (gUnk_030034E8.unk4 > 0) {
            gUnk_030051B8 |= 0x80;
        } else if (gUnk_030034E8.unk4 < 0) {
            gUnk_030051B8 |= 0x40;
        }

        gUnk_03005480 = 0;
        gUnk_030007C0 = 0;
    }

    if ((gUnk_03004C20.world == 5) && (gUnk_03004C20.level == 2 || gUnk_03004C20.level == 3)) {
        gUnk_03004C20.pad11[0] = 1;
    } else {
        gUnk_03004C20.pad11[0] = 0;
    }
    gUnk_0300542C = gUnk_0818B704[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1];
}
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
