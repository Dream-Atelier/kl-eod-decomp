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

/**
 * SetupOAMSprite — initialize a sprite slot (gUnk_03002920[arg0]) with
 * position, palette, sprite-type-specific OAM/affine setup, and bookkeeping
 * across many per-sprite-type tracker globals.  arg8 selects the sprite
 * sub-type and drives a series of switch cascades that record this slot
 * in whichever singleton tracker(s) match.
 */
void SetupOAMSprite(s32 arg0, u8 arg1, u16 arg2, u16 arg3, u8 arg4, u8 arg5, u8 arg6, u8 arg7, u8 arg8) {
    u32 var_r3_3;

    if (arg8 == 0 || arg8 >= 0x4A) {
        gUnk_03002920[arg0].xPosScreen = arg2;
        gUnk_03002920[arg0].yPosScreen = arg3;
    } else {
        gUnk_03002920[arg0].xPosBg2 = arg2;
        gUnk_03002920[arg0].yPosBg2 = arg3;
        gUnk_03002920[arg0].xPosScreen = arg2 - gUnk_03003430.bg2HOfs;
        gUnk_03002920[arg0].yPosScreen = arg3 - gUnk_03003430.bg2VOfs;
    }
    gUnk_03002920[arg0].unkA = arg1;
    gUnk_03002920[arg0].unk10 = 0;
    gUnk_03002920[arg0].unk8 = arg4;
    gUnk_03002920[arg0].unkF = arg7;
    gUnk_03002920[arg0].unkC_2 = 0;

    switch (arg8 - 0x2F) {
        case 9:
        case 10:
        case 11:
            gUnk_03002920[arg0].priority = 2;
            gUnk_03002920[arg0].objMode = 1;
            break;

        case 8:
            gUnk_03002920[arg0].unk9 = 0xFF;
            /* fallthrough */
        case 13:
            gUnk_03002920[arg0].priority = 2;
            break;

        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 14:
        case 15:
        case 18:
            gUnk_03002920[arg0].priority = 1;
            gUnk_03002920[arg0].objMode = 0;
            break;

        default:
            gUnk_03002920[arg0].priority = arg5;
            gUnk_03002920[arg0].objMode = 0;
            break;
    }

    switch (arg8 - 1) {
        case 0x71:
            gUnk_03002920[arg0].unkC_4 = 0;
            gUnk_03002920[arg0].unk8 = 0;
            gUnk_03002920[arg0].unk9 = arg4;
            if (arg4 == 1
                && ((gUnk_03005220.unk1_7
                     >> gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xC].unk0[gUnk_03004C20.room - 1].unk5)
                    & arg4)) {
                gUnk_03002920[arg0].unkC_4 = 1;
                gUnk_03002920[arg0 - 1].unkC_4 = 1;
                gUnk_03002920[arg0 - 2].unkC_4 = 1;
            }
            break;

        case 0x70:
            if ((arg7 == 0) && ((gUnk_03005220.unk58 >> arg4) & 1)) {
                arg6 = 1;
                gUnk_03004C00 = 1;
            } else {
                gUnk_03004C00 = 0;
            }
            gUnk_03002920[arg0].unk9 = arg4;
            gUnk_03002920[arg0].unkC_4 = arg6;
            break;

        case 0x37:
            if (gUnk_03004C00 != 0) {
                gUnk_03002920[arg0].yPosBg2 += 0x1A;
                arg6 = 1;
            }
            gUnk_03002920[arg0].unkC_4 = arg6;
            break;

        case 0x39:
            gUnk_03002920[arg0].unkC_4 = arg6;
            break;

        case 0x3B:
            if (((gUnk_03005220.unk1_7 >> arg6) & 1) && (arg4 == 0xFF)) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            gUnk_03002920[arg0].unk8 = 0;
            gUnk_03002920[arg0].unk9 = arg4;
            gUnk_03002920[arg0].unkC_4 = 0;
            break;

        case 0x26:
        case 0x27:
        case 0x28:
        case 0x29:
            gUnk_03002920[arg0].unk8 = 0;
            gUnk_03002920[arg0].unk9 = arg4;
            gUnk_03002920[arg0].unkC_4 = arg6;
            break;

        case 0x2E:
        case 0x2F:
        case 0x30:
        case 0x31:
        case 0x32:
            gUnk_03002920[arg0].unkC_4 = arg6;
            if (arg6 == 0) {
                gUnk_03002920[arg0].unk8 = 0;
            }
            break;

        case 0x41:
            gUnk_03002920[arg0].affineEnable = 1;
            gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288 + 1;
            gUnk_03004680[gUnk_03005288 + 1].unk0 = gUnk_03004680[gUnk_03005288 + 1].unk6 = COS(0);
            gUnk_03004680[gUnk_03005288 + 1].unk4 = SIN(0);
            gUnk_03004680[gUnk_03005288 + 1].unk2 = -SIN(0);
            gUnk_03002920[arg0].unkC_2 = arg6;
            break;

        case 0x35:
        case 0x74:
            gUnk_03002920[arg0].unkC_4 = arg6;
            if (arg6 != 3) {
                gUnk_03002920[arg0].unkC_2 = arg6;
            }
            break;

        case 0x73:
            gUnk_03002920[arg0].unk9 = 0;
            break;

        case 0x34:
            if (arg7 != 0x1C) {
                arg6 = arg6 ^ gUnk_03005220.unk3_6;
                if (arg6 == 1) {
                    if (gUnk_030047B8 == 0) {
                        gUnk_030047B8 = arg0;
                        gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
                        gUnk_03004680[gUnk_03005288].unk0 = gUnk_03004680[gUnk_03005288].unk6 = 0x100;
                        gUnk_03004680[gUnk_03005288].unk2 = gUnk_03004680[gUnk_03005288].unk4 = 0;
                        gUnk_03005288 += 1;
                    } else {
                        gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03002920[gUnk_030047B8].affineHFlip_matrixNum;
                    }
                } else {
                    if (gUnk_03005470 == 0) {
                        gUnk_03005470 = arg0;
                        gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
                        gUnk_03004680[gUnk_03005288].unk0 = gUnk_03004680[gUnk_03005288].unk6 = 0x200;
                        gUnk_03004680[gUnk_03005288].unk2 = gUnk_03004680[gUnk_03005288].unk4 = 0;
                        gUnk_03005288 += 1;
                    } else {
                        gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03002920[gUnk_03005470].affineHFlip_matrixNum;
                    }
                    gUnk_03002920[arg0].unkF = 0x1C;
                }
                gUnk_03002920[arg0].affineEnable = 1;
                gUnk_03002920[arg0].unkC_4 = arg6;
            }
            break;

        case 0x6F:
            if ((arg7 == 0) && ((gUnk_03005220.unk2E >> arg4) & 1)) {
                gUnk_03002920[arg0].unkF = 0x1C;
            } else {
                gUnk_03002920[arg0].affineEnable = 1;
                gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
                gUnk_03004680[gUnk_03005288].unk0 = gUnk_03004680[gUnk_03005288].unk6 = 0x100;
                gUnk_03004680[gUnk_03005288].unk2 = gUnk_03004680[gUnk_03005288].unk4 = 0;
                gUnk_03005288 += 1;
            }
            break;

        case 0x3C:
            gUnk_03002920[arg0].affineEnable = 1;
            gUnk_03002920[arg0].unkC_4 = arg6;
            gUnk_03002920[arg0].unk8 = 0x80;
            if (gUnk_03003610[gUnk_03005298].unk0 == 0) {
                gUnk_03003610[gUnk_03005298].unk0 = arg0;
                gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
                gUnk_03004680[gUnk_03005288].unk0 = MultiplyQ8(COS(0), ReciprocalQ8(0x100));
                gUnk_03004680[gUnk_03005288].unk2 = MultiplyQ8(SIN(0), ReciprocalQ8(0x100));
                gUnk_03004680[gUnk_03005288].unk4 = MultiplyQ8(-SIN(0), ReciprocalQ8(0x100));
                gUnk_03004680[gUnk_03005288].unk6 = MultiplyQ8(COS(0), ReciprocalQ8(gUnk_03002920[arg0].unk8));
                gUnk_03005288 += 1;
                gUnk_03002920[arg0].yPosBg2 += (u8)-gUnk_03002920[arg0].unk8 >> 3;
                gUnk_03002920[arg0 - 1].yPosBg2 += (u8)-gUnk_03002920[arg0].unk8 >> 2;
            } else {
                gUnk_03003610[gUnk_03005298++].unk1 = arg0;
                gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
                gUnk_03004680[gUnk_03005288].unk0 = MultiplyQ8(COS(0), ReciprocalQ8(0x100));
                gUnk_03004680[gUnk_03005288].unk2 = MultiplyQ8(SIN(0), ReciprocalQ8(0x100));
                gUnk_03004680[gUnk_03005288].unk4 = MultiplyQ8(-SIN(0), ReciprocalQ8(0x100));
                gUnk_03004680[gUnk_03005288].unk6 = MultiplyQ8(COS(0), ReciprocalQ8(gUnk_03002920[arg0].unk8));
                gUnk_03005288 += 1;
                gUnk_03002920[arg0].yPosBg2 += (u8)-gUnk_03002920[arg0].unk8 >> 3;
                gUnk_03002920[arg0 - 1].yPosBg2 += (u8)-gUnk_03002920[arg0].unk8 >> 2;
            }
            break;

        case 0x7A:
        case 0x7B:
            gUnk_0300528C += 1;
            /* fallthrough */
        case 0x75:
        case 0x76:
        case 0x78:
            gUnk_03002920[arg0].unk8 = 0;
            gUnk_03002920[arg0].unk9 = arg4;
            gUnk_03002920[arg0].unkC_2 = arg6 & 1;
            if (arg8 == 0x79) {
                gUnk_03002920[arg0].unkC_4 = arg6;
            }
            break;

        case 0x7C:
            gUnk_0300528C += 1;
            /* fallthrough */
        case 0x77:
        case 0x79:
            gUnk_03002920[arg0].unk8 = 0;
            gUnk_03002920[arg0].unk9 = arg4;
            gUnk_03002920[arg0].unkC_4 = arg6;
            break;

        case 0x5:
        case 0x7:
            gUnk_03002920[arg0].unk8 = 0;
            gUnk_03002920[arg0].unk9 = arg4;
            gUnk_03002920[arg0].unkC_4 = arg6 & 1;
            if (arg6 & 2) {
                gUnk_03002920[arg0].affineEnable = 1;
                gUnk_03002920[arg0].affineDouble = 1;
                if (gUnk_030052A8 == 0) {
                    gUnk_030052A8 = arg0;
                    gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
                    gUnk_03004680[gUnk_03005288].unk0 = gUnk_03004680[gUnk_03005288].unk6 = 0x70;
                    gUnk_03004680[gUnk_03005288].unk2 = gUnk_03004680[gUnk_03005288].unk4 = 0;
                    gUnk_03005288 += 1;
                } else {
                    gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03002920[gUnk_030052A8].affineHFlip_matrixNum;
                }
            }
            break;

        case 0x25:
            gUnk_03002920[arg0].affineEnable = 1;
            gUnk_03002920[arg0].affineDouble = 1;
            gUnk_03002920[arg0].affineHFlip_matrixNum = gUnk_03005288;
            gUnk_03004680[gUnk_03005288].unk0 = gUnk_03004680[gUnk_03005288].unk6 = 0x8000;
            gUnk_03004680[gUnk_03005288].unk2 = gUnk_03004680[gUnk_03005288].unk4 = 0;
            gUnk_03005288 += 1;
            gUnk_03004C38 += 1;
            break;

        case 0x8:
            gUnk_03000790[arg4].unk0 = arg2 - 0xE;
            gUnk_03000790[arg4].unk2 = arg2 + 0xE;
            gUnk_03000790[arg4].unk6 = arg3;
            if (arg7 == 0) {
                for (var_r3_3 = arg3 - 0x20; var_r3_3 >= gUnk_03005468.unk2; var_r3_3 -= 8) {
                    if (gUnk_03004654[0x18] <= gUnk_03004790.pBufBg2Tilemap[(arg2 >> 3) + (gUnk_03003430.unk48 * (var_r3_3 >> 3))]) {
                        gUnk_03000790[arg4].unk4 = var_r3_3 + 8;
                        gUnk_03000790[arg4].unk8 = var_r3_3 + 8;
                        break;
                    }
                }
            }
            /* fallthrough */
        case 0x9:
            gUnk_03002920[arg0].yPosBg2 = gUnk_03000790[arg4].unk8;
            gUnk_03002920[arg0].unkC_4 = arg4;
            break;

        case 0x0:
        case 0x1:
        case 0x3:
            if (gUnk_03005220.unk1_4 & arg8) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            break;

        case 0x4:
            if ((arg7 == 0) && ((gUnk_03005220.unk14 >> arg4) & 1)) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            gUnk_03002920[arg0].unk9 = arg6;
            break;

        case 0x2D:
            if ((arg7 == 0) && (gUnk_03005220.unk4 & (1 << arg4))) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            break;

        case 0x2B:
            if (gUnk_0300547C == 0) {
                gUnk_0300547C = arg0;
            }
            /* fallthrough */
        case 0x2C:
            if ((arg8 == 0x2D) && (gUnk_03000818 == 0)) {
                gUnk_03000818 = arg0;
            }
            if (arg4 <= 0x1FU) {
                if ((arg7 == 0) && (gUnk_03005220.unk8 & (1 << arg4))) {
                    gUnk_03002920[arg0].unkF = 0x1C;
                }
            } else if ((arg7 == 0) && (gUnk_03005220.unkC & (1 << (arg4 - 0x20)))) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            gUnk_03002920[arg0].unkC_2 = arg6;
            break;

        case 0x6:
            if (gUnk_030051B4 == 0) {
                gUnk_030051B4 = arg0;
                if (gUnk_03004C20.level == 8) {
                    gUnk_030051B4 = arg0 + 1;
                }
            }
            if ((arg7 == 0) && ((gUnk_03005220.unk2_7 >> arg4) & 1)) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            gUnk_03002920[arg0].unkC_2 = arg6;
            break;

        case 0x2:
            if (gUnk_030008EC == 0) {
                gUnk_030008EC = arg0;
            }
            if ((gUnk_03005220.unk0_2 >> arg4) & 1) {
                gUnk_03002920[arg0].unkF = 0x1C;
            }
            gUnk_03002920[arg0].unkC_2 = arg6;
            break;

        default:
            gUnk_03002920[arg0].unkC_2 = arg6;
            break;
    }

    gUnk_03002920[arg0].unk11 = arg8;

    switch (arg8 - 0x25) {
        case 0x0:
            if (gUnk_030007F4 == 0) {
                gUnk_030007F4 = arg0;
            }
            gUnk_0300290C = arg0;
            /* fallthrough */
        case 0x4:
        case 0x6:
        case 0x10:
        case 0x4A:
        case 0x4B:
            if (gUnk_03005484 == 0) {
                gUnk_03005484 = arg0;
            }
            gUnk_03004650 = arg0;
            /* fallthrough */
        case 0x2:
        case 0x3:
        case 0x5:
        case 0x13:
        case 0x19:
        case 0x1A:
        case 0x1C:
            if (gUnk_030051D4 == 0) {
                gUnk_030051D4 = arg0;
            }
            gUnk_030034C8 = arg0;
            break;
    }

    switch (arg8 - 0xB) {
        case 0x1A:
        case 0x64:
            if (gUnk_03003500 == 0) {
                gUnk_03003500 = arg0;
            }
            gUnk_03004664 = arg0;
            /* fallthrough */
        case 0x70:
        case 0x71:
        case 0x72:
            if (gUnk_030047BC == 0) {
                gUnk_030047BC = arg0;
            }
            gUnk_030007D4 = arg0;
            /* fallthrough */
        case 0x6B:
        case 0x6C:
        case 0x6D:
            if (gUnk_030047F8 == 0) {
                gUnk_030047F8 = arg0;
            }
            gUnk_03003504 = arg0;
            /* fallthrough */
        case 0x0:
        case 0x8:
        case 0xB:
        case 0x14:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x27:
        case 0x28:
        case 0x30:
        case 0x31:
        case 0x66:
        case 0x67:
        case 0x68:
        case 0x69:
        case 0x6A:
        case 0x6E:
        case 0x6F:
            if (gUnk_030051D8 == 0) {
                gUnk_030051D8 = arg0;
            }
            gUnk_03004788 = arg0;
            break;
    }

    switch (arg8 - 5) {
        case 0x1:
        case 0x3:
        case 0x20:
        case 0x22:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x30:
        case 0x39:
        case 0x3A:
        case 0x6A:
        case 0x6B:
            if (gUnk_03003634 == 0) {
                gUnk_03003634 = arg0;
            }
            gUnk_03005430 = arg0;
            /* fallthrough */
        case 0x23:
        case 0x31:
        case 0x33:
        case 0x3C:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
            if (gUnk_03003630 == 0) {
                gUnk_03003630 = arg0;
            }
            gUnk_03004674 = arg0;
            /* fallthrough */
        case 0x0:
        case 0x36:
        case 0x37:
            if (arg8 != 0x38) {
                if (gUnk_030034D8 == 0) {
                    gUnk_030034D8 = arg0;
                }
                gUnk_0300541C = arg0;
            }
            break;
    }

    switch (arg8 - 5) {
        case 0x0:
        case 0x19:
        case 0x20:
        case 0x30:
        case 0x33:
        case 0x36:
        case 0x37:
        case 0x3A:
        case 0x3C:
        case 0x6A:
        case 0x6B:
            if (gUnk_030034CC == 0) {
                gUnk_030034CC = arg0;
            }
            gUnk_0300529C = arg0;
            break;
    }

    switch (arg8 - 6) {
        case 0x1F:
        case 0x21:
        case 0x22:
        case 0x2F:
        case 0x32:
        case 0x38:
        case 0x39:
        case 0x3B:
        case 0x69:
        case 0x6A:
            if (gUnk_03000804 == 0) {
                gUnk_03000804 = arg0;
            }
            gUnk_0300082C = arg0;
            /* fallthrough */
        case 0x0:
        case 0x2:
        case 0x5:
        case 0x11:
        case 0x15:
        case 0x1C:
        case 0x23:
        case 0x24:
        case 0x25:
        case 0x30:
        case 0x6B:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
            if (gUnk_030007F0 == 0) {
                gUnk_030007F0 = arg0;
            }
            gUnk_03004C04 = arg0;
            break;
    }

    switch (arg8 - 5) {
        case 0x1:
        case 0x3:
        case 0x6:
        case 0x13:
        case 0x20:
        case 0x22:
        case 0x24:
        case 0x25:
        case 0x26:
        case 0x30:
        case 0x3A:
        case 0x6A:
        case 0x6B:
        case 0x71:
        case 0x72:
        case 0x73:
        case 0x74:
        case 0x75:
        case 0x76:
        case 0x77:
        case 0x78:
            if (gUnk_030047B4 == 0) {
                gUnk_030047B4 = arg0;
            }
            gUnk_03003640 = arg0;
            /* fallthrough */
        case 0x0:
        case 0x12:
        case 0x16:
        case 0x1D:
        case 0x23:
        case 0x31:
        case 0x35:
        case 0x36:
        case 0x37:
        case 0x39:
        case 0x3C:
        case 0x6C:
        case 0x6D:
        case 0x6E:
        case 0x6F:
        case 0x70:
            if (gUnk_030008F4 == 0) {
                gUnk_030008F4 = arg0;
            }
            gUnk_030008F0 = arg0;
            break;
    }

    if (arg8 == 0x38) {
        if (gUnk_03000824 == 0) {
            gUnk_03000824 = arg0;
        }
        gUnk_03005424 = arg0;
    }

    switch (arg8 - 0x25) {
        case 0x0:
        case 0x4A:
        case 0x51:
        case 0x56:
        case 0x57:
        case 0x58:
            if (gUnk_03002904 == 0) {
                gUnk_03002904 = arg0;
            }
            gUnk_030008FC = arg0;
            /* fallthrough */
        case 0x52:
        case 0x53:
        case 0x54:
        case 0x55:
            if (gUnk_030034A4 == 0) {
                gUnk_030034A4 = arg0;
            }
            gUnk_030052B0 = arg0;
            /* fallthrough */
        case 0x10:
        case 0x49:
            if (gUnk_03003638 == 0) {
                gUnk_03003638 = arg0;
            }
            gUnk_03002908 = arg0;
            break;
    }

    switch (arg8) {
        case 0x6E ... 0x78:
        case 0x7A ... 0x7D:
            if (gUnk_030052B4 == 0) {
                gUnk_030052B4 = arg0;
            }
            gUnk_030051C4 = arg0;
            break;
    }
}
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
void RenderDialogSprites(void) {
    s32 sp0;
    s32 sp4;
    s32 var_sb;
    s32 var_sb_3;
    s32 var_sb_5;
    s32 var_sl;
    s32 var_sl_2;
    struct Unk_0300466C_4 *var_r7;
    struct Unk_0300466C_4 *var_r7_2;
    struct Unk_0300466C_4 *var_r7_3;
    struct Unk_0300466C_4 *var_r7_4;
    u8 temp_r0;
    u8 temp_r0_5;
    u8 temp_r0_8;
    u8 temp_r1_12;
    u8 temp_r1_15;
    u8 temp_r1_3;
    u8 temp_r1_6;
    u8 temp_r1_7;

    InitOamEntries();
    gUnk_03000820 = gUnk_03004800;

    sp0 = gUnk_03002920->yPosScreen;
    sp4 = gUnk_03002920->xPosScreen;

    for (var_sb = 0xD; var_sb < gUnk_03005428; var_sb++) {
        if (gUnk_03002920[var_sb].unk10 == 1) {
            if ((gUnk_03002920[var_sb].unk11 == 0x52) && (gUnk_03002920[var_sb].xPosScreen > (sp4 - 0x10))
                && (gUnk_03002920[var_sb].xPosScreen < (sp4 + 0x10)) && (gUnk_03002920[var_sb].yPosScreen <= (sp0 + 0xA))
                && (gUnk_03002920[var_sb].yPosScreen >= (u32)sp0)) {
            } else if ((gUnk_03002920[var_sb].yPosScreen + gUnk_03002920[var_sb].unk8) <= sp0) {
            } else {
                temp_r0 = gUnk_03002920[var_sb].unkA;
                if (gUnk_03002920[var_sb].unkA < 0xD) {
                    gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[var_sb].unkA];
                } else {
                    gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[var_sb].unkA - 0xD];
                }

                temp_r1_3 = gUnk_0300466C->unk0;
                var_r7 = gUnk_0300466C->unk4;
                for (var_sl = 0; var_sl < temp_r1_3; var_sl++) {
                    if (gUnk_03002920[var_sb].affineDouble) {
                        if (gUnk_03002920[var_sb].unk11 == 0x1C) {
                            gUnk_03000820->split.x = (var_r7->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                                + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x18);
                            gUnk_03000820->split.y = var_r7->unk4 + ((s8)var_r7->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                                + (((s8)gUnk_03002920[var_sb].unkB >> 4) << 4);
                        } else {
                            gUnk_03000820->split.x = (var_r7->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                                + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                            gUnk_03000820->split.y = var_r7->unk4 + ((s8)var_r7->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                                + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                        }
                    } else {
                        gUnk_03000820->split.x
                            = var_r7->unk3 + gUnk_03002920[var_sb].xPosScreen + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y
                            = gUnk_03002920[var_sb].yPosScreen + var_r7->unk4 + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                    }

                    gUnk_03000820->split.bpp = var_r7->bpp_paletteNum >> 7;
                    gUnk_03000820->split.tileNum = var_r7->tileNum;
                    gUnk_03000820->split.paletteNum = var_r7->bpp_paletteNum & 0x7F;
                    gUnk_03000820->split.shape = (var_r7->shape_size & 0xC) >> 2;
                    gUnk_03000820->split.size = var_r7->shape_size & 3;

                    gUnk_03000820->split.priority = gUnk_03002920[var_sb].priority;
                    gUnk_03000820->split.objMode = gUnk_03002920[var_sb].objMode;
                    gUnk_03000820->split.affineMode = (gUnk_03002920[var_sb].affineDouble << 1) | gUnk_03002920[var_sb].affineEnable;
                    gUnk_03000820->split.matrixNum = gUnk_03002920[var_sb].affineHFlip_matrixNum;

                    if (gUnk_03002920[var_sb].affineEnable) {
                        gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].affineHFlip_matrixNum >> 3;
                    } else {
                        gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].unkC_2 & 1;
                        gUnk_03000820->split.vFlip = gUnk_03002920[var_sb].unkC_2 >> 1;
                    }

                    gUnk_03000820 += 1;
                    var_r7 += 1;
                }
            }
        }
    }

    for (var_sb = 0; var_sb <= 0xC; var_sb++) {
        if (gUnk_03002920[var_sb].unk10 == 1) {
            temp_r1_6 = gUnk_03002920[var_sb].unkA;
            if (gUnk_03002920[var_sb].unkA < 0xD) {
                gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[var_sb].unkA];
            } else {
                gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[var_sb].unkA - 0xD];
            }

            temp_r1_7 = gUnk_0300466C->unk0;
            var_r7_2 = gUnk_0300466C->unk4;
            for (var_sb_3 = 0; var_sb_3 < temp_r1_7; var_sb_3++) {
                if (gUnk_03002920[var_sb].affineDouble) {
                    if (gUnk_03002920[var_sb].unk11 == 0x1C) {
                        gUnk_03000820->split.x = (var_r7_2->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                            + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x18);
                        gUnk_03000820->split.y = var_r7_2->unk4 + ((s8)var_r7_2->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                            + (((s8)gUnk_03002920[var_sb].unkB >> 4) << 4);
                    } else {
                        gUnk_03000820->split.x = (var_r7_2->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                            + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y = var_r7_2->unk4 + ((s8)var_r7_2->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                            + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                    }
                } else {
                    gUnk_03000820->split.x
                        = var_r7_2->unk3 + gUnk_03002920[var_sb].xPosScreen + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                    gUnk_03000820->split.y
                        = gUnk_03002920[var_sb].yPosScreen + var_r7_2->unk4 + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                }

                gUnk_03000820->split.bpp = var_r7_2->bpp_paletteNum >> 7;
                gUnk_03000820->split.tileNum = var_r7_2->tileNum;
                gUnk_03000820->split.paletteNum = var_r7_2->bpp_paletteNum & 0x7F;
                gUnk_03000820->split.shape = (var_r7_2->shape_size & 0xC) >> 2;
                gUnk_03000820->split.size = var_r7_2->shape_size & 3;

                gUnk_03000820->split.priority = gUnk_03002920[var_sb].priority;
                gUnk_03000820->split.objMode = gUnk_03002920[var_sb].objMode;
                gUnk_03000820->split.affineMode = (gUnk_03002920[var_sb].affineDouble << 1) | gUnk_03002920[var_sb].affineEnable;
                gUnk_03000820->split.matrixNum = gUnk_03002920[var_sb].affineHFlip_matrixNum;

                if (gUnk_03002920[var_sb].affineEnable) {
                    gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].affineHFlip_matrixNum >> 3;
                } else {
                    gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].unkC_2 & 1;
                    gUnk_03000820->split.vFlip = gUnk_03002920[var_sb].unkC_2 >> 1;
                }

                gUnk_03000820 += 1;
                var_r7_2 += 1;
            }
        }
    }

    for (var_sb = 0xD; var_sb < gUnk_03005428; var_sb++) {
        if (gUnk_03002920[var_sb].unk10 == 1) {
            if (gUnk_03002920[var_sb].unk11 == 0x52) {
                if ((gUnk_03002920[var_sb].xPosScreen > (sp4 - 0x10) && gUnk_03002920[var_sb].xPosScreen < (sp4 + 0x10))
                    && (gUnk_03002920[var_sb].yPosScreen <= (sp0 + 0xA) && gUnk_03002920[var_sb].yPosScreen >= (u32)sp0)) {
                    temp_r0_8 = gUnk_03002920[var_sb].unkA;
                    if (gUnk_03002920[var_sb].unkA < 0xD) {
                        gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[var_sb].unkA];
                    } else {
                        gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[var_sb].unkA - 0xD];
                    }

                    temp_r1_15 = gUnk_0300466C->unk0;
                    var_r7_3 = gUnk_0300466C->unk4;
                    for (var_sl_2 = 0; var_sl_2 < temp_r1_15; var_sl_2++) {
                        if (gUnk_03002920[var_sb].affineDouble) {
                            if (gUnk_03002920[var_sb].unk11 == 0x1C) {
                                gUnk_03000820->split.x = (var_r7_3->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                                    + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x18);
                                gUnk_03000820->split.y = var_r7_3->unk4 + ((s8)var_r7_3->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                                    + (((s8)gUnk_03002920[var_sb].unkB >> 4) << 4);
                            } else {
                                gUnk_03000820->split.x = (var_r7_3->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                                    + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                                gUnk_03000820->split.y = var_r7_3->unk4 + ((s8)var_r7_3->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                                    + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                            }
                        } else {
                            gUnk_03000820->split.x = var_r7_3->unk3 + gUnk_03002920[var_sb].xPosScreen
                                + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                            gUnk_03000820->split.y
                                = gUnk_03002920[var_sb].yPosScreen + var_r7_3->unk4 + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                        }

                        gUnk_03000820->split.bpp = var_r7_3->bpp_paletteNum >> 7;
                        gUnk_03000820->split.tileNum = var_r7_3->tileNum;
                        gUnk_03000820->split.paletteNum = var_r7_3->bpp_paletteNum & 0x7F;
                        gUnk_03000820->split.shape = (var_r7_3->shape_size & 0xC) >> 2;
                        gUnk_03000820->split.size = var_r7_3->shape_size & 3;

                        gUnk_03000820->split.priority = gUnk_03002920[var_sb].priority;
                        gUnk_03000820->split.objMode = gUnk_03002920[var_sb].objMode;
                        gUnk_03000820->split.affineMode
                            = (gUnk_03002920[var_sb].affineDouble << 1) | gUnk_03002920[var_sb].affineEnable;
                        gUnk_03000820->split.matrixNum = gUnk_03002920[var_sb].affineHFlip_matrixNum;

                        if (gUnk_03002920[var_sb].affineEnable) {
                            gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].affineHFlip_matrixNum >> 3;
                        } else {
                            gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].unkC_2 & 1;
                            gUnk_03000820->split.vFlip = gUnk_03002920[var_sb].unkC_2 >> 1;
                        }

                        gUnk_03000820 += 1;
                        var_r7_3 += 1;
                    }
                }
            }

            if ((gUnk_03002920[var_sb].yPosScreen + gUnk_03002920[var_sb].unk8) <= sp0) {
                temp_r0_5 = gUnk_03002920[var_sb].unkA;
                if (gUnk_03002920[var_sb].unkA < 0xD) {
                    gUnk_0300466C = &gUnk_08078FC8[gUnk_03002920[var_sb].unkA];
                } else {
                    gUnk_0300466C = (void *)&gUnk_030051DC[gUnk_03002920[var_sb].unkA - 0xD];
                }

                temp_r1_12 = gUnk_0300466C->unk0;
                var_r7_4 = gUnk_0300466C->unk4;
                for (var_sb_5 = 0; var_sb_5 < temp_r1_12; var_sb_5++) {
                    if (gUnk_03002920[var_sb].affineDouble) {
                        if (gUnk_03002920[var_sb].unk11 == 0x1C) {
                            gUnk_03000820->split.x = (var_r7_4->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                                + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x18);
                            gUnk_03000820->split.y = var_r7_4->unk4 + ((s8)var_r7_4->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                                + (((s8)gUnk_03002920[var_sb].unkB >> 4) << 4);
                        } else {
                            gUnk_03000820->split.x = (var_r7_4->unk3 * 2) + gUnk_03002920[var_sb].xPosScreen
                                + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                            gUnk_03000820->split.y = var_r7_4->unk4 + ((s8)var_r7_4->unk4 >> 1) + gUnk_03002920[var_sb].yPosScreen
                                + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                        }
                    } else {
                        gUnk_03000820->split.x
                            = var_r7_4->unk3 + gUnk_03002920[var_sb].xPosScreen + ((s32)(gUnk_03002920[var_sb].unkB << 0x1C) >> 0x1C);
                        gUnk_03000820->split.y
                            = gUnk_03002920[var_sb].yPosScreen + var_r7_4->unk4 + ((s8)(gUnk_03002920[var_sb].unkB) >> 0x4);
                    }

                    gUnk_03000820->split.bpp = var_r7_4->bpp_paletteNum >> 7;
                    gUnk_03000820->split.tileNum = var_r7_4->tileNum;
                    gUnk_03000820->split.paletteNum = var_r7_4->bpp_paletteNum & 0x7F;
                    gUnk_03000820->split.shape = (var_r7_4->shape_size & 0xC) >> 2;
                    gUnk_03000820->split.size = var_r7_4->shape_size & 3;

                    gUnk_03000820->split.priority = gUnk_03002920[var_sb].priority;
                    gUnk_03000820->split.objMode = gUnk_03002920[var_sb].objMode;
                    gUnk_03000820->split.affineMode = (gUnk_03002920[var_sb].affineDouble << 1) | gUnk_03002920[var_sb].affineEnable;
                    gUnk_03000820->split.matrixNum = gUnk_03002920[var_sb].affineHFlip_matrixNum;

                    if (gUnk_03002920[var_sb].affineEnable) {
                        gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].affineHFlip_matrixNum >> 3;
                    } else {
                        gUnk_03000820->split.hFlip = gUnk_03002920[var_sb].unkC_2 & 1;
                        gUnk_03000820->split.vFlip = gUnk_03002920[var_sb].unkC_2 >> 1;
                    }

                    gUnk_03000820 += 1;
                    var_r7_4 += 1;
                }
            }
        }
    }
}
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
extern void VBlankCallback_Gameplay(void); /* VBlankCallback_Gameplay — literal-pool stored callback */
extern void InitGameplayState(void); /* InitGameplayState — literal-pool stored callback */
extern void TransitionGameOver(void); /* TransitionGameOver */
extern void TransitionFadeOutWithMusic(void); /* TransitionFadeOutWithMusic */
extern void VBlankCallback_Credits(void); /* VBlankCallback_Credits */
extern void ApplyEntityTileMovement(void); /* ApplyEntityTileMovement */
extern void UpdatePaletteAnimations(void); /* UpdatePaletteAnimations */
extern void EntityGravityAndFloorCheck(u8); /* EntityGravityAndFloorCheck */
extern void PlayerFollowEntityMovement(u8); /* PlayerFollowEntityMovement */
extern void EntityBehaviorMasterUpdate(u8); /* EntityBehaviorMasterUpdate */
extern void sub_08016EEC(u8); /* code_1 alternate-entry, see asm/thumb_aliases.s */
extern void sub_0801BB6C(u8);
extern void sub_0801BCC0(u8);
extern void sub_0801B044(u8);
extern void sub_0801E1A8(u8);
extern void sub_0801E354(u8);
extern void EntityStateSwitch_Carried(u8); /* EntityStateSwitch_Carried */
extern void EntityPairUpdate(u8); /* EntityPairUpdate */
extern void EntityProximityDamageCheck(u8); /* EntityProximityDamageCheck */
extern void EntitySpriteFrameUpdate(u32); /* EntitySpriteFrameUpdate — kleod declares as u32 */
extern void EntityPositionFromLevelTable(u8); /* EntityPositionFromLevelTable */
extern void EntityThrowUpdate(u8); /* EntityThrowUpdate */
extern void EntityPlatformRide(u8); /* EntityPlatformRide */
extern void EntityDeathAnimation(u8); /* EntityDeathAnimation */
extern void EntityBounceOffWall(u8); /* EntityBounceOffWall */
extern void EntityProjectileUpdate(void); /* EntityProjectileUpdate */

/**
 * HandlePauseMenuInput: on START press (with guards) save the active
 * VBlank-callback array and push the pause-menu sub-state pair
 * (InitGameplayState / VBlankCallback_Gameplay).  Otherwise iterate
 * gUnk_03002920[D..count] dispatching a per-unk11 entity update handler.
 */
void HandlePauseMenuInput(void) {
    u32 var_r2;
    u32 var_r4;

    if ((gNewKeys & START_BUTTON) && (gUnk_030034E4 == 0) && (gUnk_03005220.unk46 == 0)) {
        for (var_r2 = 0; var_r2 < 10; var_r2++) {
            gCallbackQueue.previous[var_r2] = gCallbackQueue.current[var_r2];
        }

        gCallbackQueue.previousCount = gCallbackQueue.currentCount;
        gUnk_030034BC = 0;
        gUnk_03003410.unk4 = 1;
        gCallbackQueue.next[0] = InitGameplayState;
        gCallbackQueue.next[1] = VBlankCallback_Gameplay;
        gCallbackQueue.next[2] = (void (*)())1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 3;
        gCallbackQueue.currentCount = 1;
        gCallbackQueue.current[0] = NULL;
        return;
    }

    if (gUnk_030052A0 == 0xFE) {
        if ((gCallbackQueue.current[4] != TransitionGameOver) && (gCallbackQueue.current[4] != TransitionFadeOutWithMusic)) {
            VBlankCallback_Credits();
        }

        for (var_r4 = 0xD; var_r4 < gUnk_03005428; var_r4++) {
            if (gUnk_03002920[var_r4].unkF != 0x1C) {
                switch (gUnk_03002920[var_r4].unk11 - 1) {
                    case 0x24:
                    case 0x6E:
                        if (gUnk_03002920[var_r4].unk8 != 1) {
                            EntityBehaviorMasterUpdate((u8)var_r4);
                        }
                        break;

                    case 0x5:
                    case 0x7:
                    case 0xA:
                    case 0x75:
                    case 0x76:
                    case 0x77:
                    case 0x78:
                    case 0x79:
                    case 0x7A:
                    case 0x7B:
                    case 0x7C:
                        sub_08016EEC((u8)var_r4);
                        break;

                    case 0x0:
                    case 0x1:
                    case 0x3:
                        EntityPositionFromLevelTable((u8)var_r4);
                        break;

                    case 0x2E:
                    case 0x2F:
                    case 0x30:
                    case 0x31:
                    case 0x32:
                        PlayerFollowEntityMovement((u8)var_r4);
                        break;

                    case 0x26:
                    case 0x27:
                    case 0x28:
                    case 0x29:
                    case 0x2A:
                        EntityStateSwitch_Carried((u8)var_r4);
                        break;

                    case 0x3E:
                        sub_0801BB6C((u8)var_r4);
                        break;

                    case 0x3F:
                        sub_0801BCC0((u8)var_r4);
                        break;

                    case 0x36:
                    case 0x70:
                    case 0x71:
                    case 0x73:
                        EntityPairUpdate((u8)var_r4);
                        break;

                    case 0x38:
                        EntityProximityDamageCheck((u8)var_r4);
                        break;

                    case 0x3C:
                        EntitySpriteFrameUpdate(var_r4);
                        break;

                    case 0x8:
                    case 0x9:
                        EntityGravityAndFloorCheck((u8)var_r4);
                        break;

                    case 0x34:
                        if ((var_r4 == gUnk_030047B8) || (var_r4 == gUnk_03005470)) {
                            EntityThrowUpdate((u8)var_r4);
                        }
                        break;

                    case 0x6F:
                        EntityThrowUpdate((u8)var_r4);
                        break;

                    case 0xB:
                        EntityDeathAnimation((u8)var_r4);
                        break;

                    case 0x42:
                        EntityBounceOffWall((u8)var_r4);
                        break;

                    case 0x74:
                        if (gUnk_03002920[var_r4].unk8 != 0) {
                            gUnk_03002920[var_r4].unk8 -= 1;
                        }
                        break;

                    case 0x25:
                        EntityPlatformRide((u8)var_r4);
                        break;

                    case 0x41: {
                        u16 *tmp = (void *)&gUnk_03002920[var_r4];
                        if (tmp[4] != 0) {
                            sub_0801B044((u8)var_r4);
                        }
                    } break;

                    case 0x2:
                        if (gUnk_03002920[var_r4].unkF == 2) {
                            sub_0801E1A8((u8)var_r4);
                        }
                        break;

                    case 0x2D:
                        if (gUnk_03002920[var_r4].unkF == 2) {
                            sub_0801E354((u8)var_r4);
                        }
                        break;
                }
            }
        }

        UpdatePaletteAnimations();
    }

    if (gUnk_0300528C != 0) {
        EntityProjectileUpdate();
    }

    if (gUnk_03005220.unk3F != 0) {
        ApplyEntityTileMovement();
    }
}
extern void InitGameplayState(void); /* InitGameplayState — literal-pool stored callback */
extern void EntityPositionFromROMTable(u8); /* EntityPositionFromROMTable */
extern void EntityScrollBoundsCheck(u8); /* EntityScrollBoundsCheck */
extern void EntityItemDrop(s32); /* EntityItemDrop */
extern void EntityTimerAction(u8); /* EntityTimerAction */
extern void EntityComplexBehavior(u8); /* EntityComplexBehavior */
extern void EntityMovingObstacle(u8); /* sub_0801FFF0 */
extern void EntityBossPhaseA(u8); /* sub_080202D4 */
extern void EntityCrushingBlock(u8); /* EntityCrushingBlock */
extern void EntityBossPhaseB(u8); /* EntityBossPhaseB */
extern void EntitySpringBoard(u8); /* sub_0802192C */
extern void EntityCutsceneActor(u8); /* EntityCutsceneActor */
extern void EntityBossPhaseD(u8); /* EntityBossPhaseD */
extern void EntityMiniBoss(u8); /* sub_08023988 */
extern void EntityBossPhaseC(s32); /* sub_08021DAC */
extern void EntityMiniBossAlt(u8); /* sub_08023BC0 */
extern void UpdateEntitySpawnState(u8); /* sub_0803CE14 */
extern void UpdatePaletteAnimations(void); /* UpdatePaletteAnimations */
extern void VBlankCallback_Credits(void); /* VBlankCallback_Credits */
extern void ApplyEntityTileMovement(void); /* ApplyEntityTileMovement */
extern void TransitionFadeOutWithMusic(void); /* TransitionFadeOutWithMusic */
extern void TransitionGameOver(void); /* TransitionGameOver */
extern void SetPaletteAnimEntry(s32, u8);
extern void PlayerFollowEntityMovement(u8);
extern void EntityGravityAndFloorCheck(u8);

/**
 * UpdateUIState: per-frame entity dispatcher.  On START press (with
 * other guards) saves the active VBlank-callback array and pushes the
 * pause-menu sub-state (InitGameplayState / AnimatePaletteEffects).
 * Otherwise sweeps gUnk_03002920[0..count] dispatching one of ~20
 * entity-type-specific update handlers per slot based on unk11.
 */
void UpdateUIState(void) {
    s32 var_r6;
    struct Unk_03002920 *temp_r1_2;
    struct Unk_03002920 *temp_r1_5;
    u32 var_r2;

    if (gUnk_03005220.unk0_0 == 0) {
        gUnk_03002920->priority = 0;
    }

    if ((gNewKeys & START_BUTTON) && (gUnk_030034E4 == 0) && (gUnk_03005220.unk46 == 0) && (gUnk_03005400.unkC != 0)) {
        for (var_r2 = 0; var_r2 < 10; var_r2++) {
            gCallbackQueue.previous[var_r2] = gCallbackQueue.current[var_r2];
        }

        gCallbackQueue.previousCount = gCallbackQueue.currentCount;
        if ((gUnk_03004C20.world == 5) && ((gUnk_03005400.unkA == 5) || (gUnk_03005400.unkA == 7) || (gUnk_03005400.unkA == 9))) {
            gUnk_030034BC = 1;
        } else {
            gUnk_030034BC = 0;
        }

        gUnk_03003410.unk4 = 1;
        gCallbackQueue.next[0] = InitGameplayState;
        gCallbackQueue.next[1] = AnimatePaletteEffects;
        gCallbackQueue.next[2] = (void (*)())1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 3;
        gCallbackQueue.currentCount = 1;
        gCallbackQueue.current[0] = NULL;
        return;
    }

    UpdatePaletteAnimations();

    for (var_r6 = 0; var_r6 < gUnk_03005428; var_r6++) {
        if (gUnk_03002920[var_r6].unk11 == 7) {
            if (gUnk_03002920[var_r6].unkF == 0x1C) {
                if (gUnk_03002920[var_r6].unk8 == 0) {
                    if (gUnk_03005400.unk2 < 0xE10) {
                        gUnk_03005400.unk2 += 1;
                    }
                    if (gUnk_03005400.unk2 == 0xE10) {
                        gUnk_03002920[var_r6].unkF = 0x19;
                        gUnk_03005400.unk2 = 0;
                    }
                }
            }
        }

        if (gUnk_03002920[var_r6].unkF != 0x1C) {
            if (gUnk_03002920[var_r6].unk11 != 0) {
                gUnk_03002920[var_r6].affineHFlip_matrixNum = gUnk_03002920[var_r6].unkC_2;
                gUnk_03002920[var_r6].affineDouble = 0;

                switch (gUnk_03002920[var_r6].unk11 - 7) {
                    case 0x4:
                    case 0x6F:
                    case 0x70:
                    case 0x71:
                        if (gUnk_03004C20.world == 2) {
                            if (gUnk_03002920[var_r6].unkF == 0x13) {
                                gUnk_03002920[var_r6].affineHFlip_matrixNum = gUnk_03005288;
                            }
                        }
                        sub_08016EEC((u8)var_r6);
                        break;

                    case 0x0:
                        gUnk_03002920[var_r6].affineHFlip_matrixNum = 4;
                        gUnk_03002920[var_r6].affineDouble = 1;
                        EntityPositionFromROMTable((u8)var_r6);
                        break;

                    case 0x9:
                        switch (gUnk_03002920[var_r6].unkF) {
                            case 0:
                                break;

                            case 25:
                                SetPaletteAnimEntry(var_r6, 0);
                                gUnk_03002920[var_r6].unk10 = 1;
                                gUnk_03002920[var_r6].unkF = 0;
                                break;
                        }
                        break;

                    case 0xA:
                        EntityScrollBoundsCheck((u8)var_r6);
                        break;

                    case 0xB:
                        EntityItemDrop(var_r6);
                        break;

                    case 0xC:
                        EntityTimerAction((u8)var_r6);
                        break;

                    case 0xD:
                        EntityComplexBehavior((u8)var_r6);
                        break;

                    case 0xF:
                        EntityMovingObstacle((u8)var_r6);
                        break;

                    case 0xE:
                        gUnk_03002920[var_r6].affineEnable = 1;
                        gUnk_03002920[var_r6].affineHFlip_matrixNum = 3;
                        if (gUnk_03000830[0x12 - gUnk_0300363C].unk0 == 0) {
                            gUnk_03002920[var_r6].unkF = 0;
                            gUnk_03002920[var_r6].unk10 = 1;
                            gUnk_03002920[var_r6].xPosBg2 = gUnk_03002920[0x12].xPosBg2;
                            gUnk_03002920[var_r6].yPosBg2 = gUnk_03002920[0x12].yPosBg2 - 0x3C;
                        } else {
                            gUnk_03002920[var_r6].unkF = 0x1A;
                            gUnk_03002920[var_r6].unk10 = 0;
                        }
                        break;

                    case 0x10:
                        gUnk_03002920[var_r6].affineDouble = 1;
                        EntityBossPhaseA((u8)var_r6);
                        break;

                    case 0x11:
                        EntityBossPhaseB((u8)var_r6);
                        break;

                    case 0x12:
                        EntityCrushingBlock((u8)var_r6);
                        break;

                    case 0x13:
                        EntitySpringBoard((u8)var_r6);
                        break;

                    case 0x16:
                        EntityCutsceneActor((u8)var_r6);
                        break;

                    case 0x15:
                        gUnk_03002920[var_r6].affineHFlip_matrixNum = 5;
                        gUnk_03002920[var_r6].affineDouble = 1;
                        EntityBossPhaseD((u8)var_r6);
                        break;

                    case 0x17:
                        gUnk_03002920[var_r6].affineHFlip_matrixNum = 6;
                        gUnk_03003590[3].unk5_0 = gUnk_03002920[var_r6].unkC_2;
                        EntityMiniBoss((u8)var_r6);
                        break;

                    case 0x14:
                        EntityBossPhaseC(var_r6);
                        break;

                    case 0x18:
                        if (var_r6 > 0x18) {
                            gUnk_03002920[var_r6].affineHFlip_matrixNum = 10;
                        } else {
                            gUnk_03002920[var_r6].affineHFlip_matrixNum = var_r6 - 0xF;
                        }
                        EntityMiniBossAlt((u8)var_r6);
                        break;

                    case 0x1B:
                        gUnk_030034A8((u8)var_r6);
                        break;

                    case 0x2:
                    case 0x3:
                        EntityGravityAndFloorCheck((u8)var_r6);
                        break;

                    case 0x28:
                    case 0x29:
                    case 0x2A:
                    case 0x2B:
                    case 0x2C:
                        PlayerFollowEntityMovement((u8)var_r6);
                        break;

                    case 0x2F:
                        gUnk_03002920[var_r6].affineHFlip_matrixNum = var_r6 - 0x13;
                        gUnk_03003590[var_r6 - 0x16].unk2 = 0x40;
                        gUnk_03003590[var_r6 - 0x16].unk0 = 0x40;
                        gUnk_03003590[var_r6 - 0x16].unk5_0 = gUnk_03002920[var_r6].unkC_2;

                        if (gUnk_03002920[var_r6].unkC_2 & 2) {
                            gUnk_03003590[var_r6 - 0x16].unk4 = 0x80;
                        } else {
                            gUnk_03003590[var_r6 - 0x16].unk4 = 0;
                        }
                        break;

                    case 0x3D:
                    case 0x3E:
                        temp_r1_2 = &gUnk_03002920[gUnk_080E2A84[gUnk_03004C20.world - 1]
                                                                [var_r6 - gUnk_080E2A84[gUnk_03004C20.world - 1][0]]];
                        if (((temp_r1_2->unk11 != 0x22) && (temp_r1_2->unkF == 0x1C)) || (temp_r1_2->unkF == 0x19)) {
                            gUnk_03002920[var_r6].xPosBg2 = 0x200;
                            break;
                        }

                        gUnk_03002920[var_r6].xPosBg2 = temp_r1_2->xPosBg2;
                        gUnk_03002920[var_r6].yPosBg2
                            = gUnk_03002920[gUnk_080E2A84[gUnk_03004C20.world - 1][var_r6 - gUnk_080E2A84[gUnk_03004C20.world - 1][0]]]
                                  .yPosBg2
                            & ~7;

                        temp_r1_5 = &gUnk_03002920[gUnk_080E2A84[gUnk_03004C20.world - 1]
                                                                [var_r6 - gUnk_080E2A84[gUnk_03004C20.world - 1][0]]];
                        if (temp_r1_5->unk11 == 0x22) {
                            if (temp_r1_5->unkC_2 == 0) {
                                gUnk_03002920[var_r6].xPosBg2 -= 6;
                            } else {
                                gUnk_03002920[var_r6].xPosBg2 += 9;
                            }

                            gUnk_03002920[var_r6].affineHFlip_matrixNum = 5;
                            gUnk_03002920[var_r6].affineDouble = 1;
                            gUnk_03003590[2].unk0 = 0x100;
                            gUnk_03003590[2].unk2 = 0x80;
                        }

                        while (1) {
                            u32 tmp2 = gUnk_03004790.pBufBg2Tilemap[(gUnk_03002920[var_r6].xPosBg2 >> 3)
                                                                    + ((gUnk_03002920[var_r6].yPosBg2 >> 3) * gUnk_03003430.unk48)];
                            if (gUnk_03004654[0x1B] <= tmp2) {
                                gUnk_03002920[var_r6].yPosBg2 += 0xA;
                                break;
                            }

                            gUnk_03002920[var_r6].yPosBg2 += 8;
                        }

                        break;

                    case 0x19:
                        UpdateEntitySpawnState((u8)var_r6);
                        break;
                }
            }
        }
    }

    if (gUnk_03005400.unkE_7 && (gCallbackQueue.current[4] != TransitionGameOver) && (gCallbackQueue.current[4] != TransitionFadeOutWithMusic)) {
        VBlankCallback_Credits();
    }

    if (gUnk_03005220.unk37 != 0) {
        UpdatePaletteAnimations();
    }

    if (gUnk_03005220.unk3F != 0) {
        ApplyEntityTileMovement();
    }
}
extern void ClearOamEntries6Plus(void);
extern void SetPaletteAnimEntry(s32, u8); /* SetPaletteAnimEntry */
extern void DecompressAndLoadLevel(u8); /* DecompressAndLoadLevel */
extern u8 gUnk_08061FC8[0x80];
extern u8 gUnk_080627C8[0x80];
extern u8 gUnk_08063368[0x80];
extern u8 gUnk_080635E8[0x80];
extern u8 gUnk_08063FE8[0x80];
extern u8 gUnk_08064868[0x200];
extern u8 gUnk_08064A68[0x200];
extern u8 gUnk_080B8F68[0x80];
extern u8 gUnk_080B9068[0x80];
extern u8 gUnk_080B90E8[0x80];
extern u8 gUnk_080B9168[0x80];
extern u8 gUnk_080B91E8[0x80];
extern u8 gUnk_080B9268[0x80];
extern u8 gUnk_080B92E8[0x80];
extern u8 gUnk_080B9468[0x200];
extern u8 gUnk_080B9668[0x200];

/**
 * RenderCharacterTiles: per-room sprite setup pass.  Resets all per-sprite
 * tracker globals, walks the level's sprite table at gUnk_080E2B64 calling
 * SetupOAMSprite for each, then walks gUnk_03002920[D..count] dispatching
 * tile-VRAM DMA blits or palette-anim entries by sprite unk11 subtype.
 */
void RenderCharacterTiles(void) {
    s32 sp14;
    s32 sp18;
    s32 sp1C;
    s32 sp20;
    s32 sp24;
    s32 sp28;
    s32 temp_r3_3;
    u32 var_sb;

    sp14 = 0;
    sp18 = 0;
    sp1C = 0;
    sp20 = 0;
    sp24 = 0;
    sp28 = 0;

    ClearOamEntries6Plus();
    gUnk_03005428 = 0xD;

    gUnk_03005298 = gUnk_0300528C = gUnk_03005288 = gUnk_030047B8 = gUnk_03005470 = gUnk_030034E0 = gUnk_030052A8 = gUnk_03004C38
        = gUnk_03003610[0].unk2 = gUnk_03003610[0].unk3 = 0;

    gUnk_03005484 = gUnk_03004650 = gUnk_030007F4 = gUnk_0300290C = gUnk_030051D4 = gUnk_030034C8 = gUnk_03003500 = gUnk_03004664
        = gUnk_030051D8 = gUnk_03004788 = gUnk_03003630 = gUnk_03004674 = gUnk_03003634 = gUnk_03005430 = gUnk_030034D8 = gUnk_0300541C
        = gUnk_030034CC = gUnk_0300529C = gUnk_03000804 = gUnk_0300082C = gUnk_030047B4 = gUnk_03003640 = gUnk_030008F4 = gUnk_030008F0
        = gUnk_030007F0 = gUnk_03004C04 = gUnk_03000824 = gUnk_03005424 = gUnk_030052B4 = gUnk_030051C4 = gUnk_030034A4 = gUnk_030052B0
        = gUnk_03003638 = gUnk_03002908 = gUnk_030047BC = gUnk_030007D4 = gUnk_030047F8 = gUnk_03003504 = gUnk_03002904 = gUnk_030008FC
        = 0;

    for (var_sb = 0; var_sb < 3; var_sb++) {
        gUnk_03003610[var_sb].unk1 = 0;
        gUnk_03003610[var_sb].unk0 = 0;
    }

    gUnk_0300547C = gUnk_03000818 = gUnk_030051B4 = gUnk_030008EC = 0;

    for (var_sb = 0; gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk0[0].unk0 != 0xFFFF; var_sb++) {
        SetupOAMSprite(gUnk_03005428++, gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk28,
                       gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk0[gUnk_03004C20.room - 1].unk0,
                       gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk0[gUnk_03004C20.room - 1].unk2,
                       gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk0[gUnk_03004C20.room - 1].unk4, 0,
                       gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk0[gUnk_03004C20.room - 1].unk5,
                       gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk0[gUnk_03004C20.room - 1].unk6,
                       gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb].unk29);

        if ((gUnk_03002920[gUnk_03005428 - 1].unk11 <= 0x37 || gUnk_03002920[gUnk_03005428 - 1].unk11 >= 0x42)) {
            if (gUnk_03002920[gUnk_03005428 - 1].unk11 <= 8 || gUnk_03002920[gUnk_03005428 - 1].unk11 >= 11) {
                DecompressAndLoadLevel(gUnk_03005428 - 1);
            }
        }
    }

    if (gUnk_03004C20.unkA == 1) {
        gUnk_03005220.unk1A = gUnk_03002920->xPosBg2;
        gUnk_03005220.unk10 = gUnk_03002920->xPosBg2 << 0x10;
        gUnk_03005220.unk2F = 0;
    }
    gUnk_03004C10 = NULL;

    for (var_sb = 0xD; var_sb < gUnk_03005428; var_sb++) {
        switch (gUnk_03002920[var_sb].unk11 - 6) {
            case 0x5:
                if (gUnk_03004C20.level != 8) {
                    SetPaletteAnimEntry(var_sb, 3);
                } else {
                    SetPaletteAnimEntry(var_sb, 1);
                }
                break;

            case 0x70:
            case 0x75:
                SetPaletteAnimEntry(var_sb, 1);
                break;

            case 0x0:
            case 0x2:
                if (sp14 == 0) {
                    SetPaletteAnimEntry(var_sb, 0);
                    sp14 = 1;
                }
                break;

            case 0x3D:
                if (sp18 == 0) {
                    SetPaletteAnimEntry(var_sb, 0);
                    sp18 = 1;
                }
                break;

            case 0x3B:
                if (gUnk_03005220.unk3_5 != 0) {
                    SetPaletteAnimEntry(var_sb, 2);
                } else {
                    SetPaletteAnimEntry(var_sb, 0);
                }
                break;

            case 0x29:
            case 0x2A:
            case 0x2B:
            case 0x2C:
            case 0x2D:
            case 0x39:
            case 0x43:
            case 0x71:
            case 0x72:
            case 0x73:
            case 0x74:
            case 0x76:
            case 0x77:
                SetPaletteAnimEntry(var_sb, 0);
                break;

            case 0x6B:
                if (gUnk_03002920[var_sb].unkC_4 == 0) {
                    DmaCopy16(
                        3, &gUnk_08063FE8,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x80);
                } else {
                    DmaCopy16(
                        3, &gUnk_080B9268,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x80);
                }
                break;

            case 0x6C:
                if (gUnk_03002920[var_sb].yPosBg2 & 1) {
                    if (gUnk_03002920[var_sb].unk9 == 1) {
                        if (gUnk_03002920[var_sb].unkC_4 == 0) {
                            DmaCopy16(
                                3, &gUnk_080635E8,
                                OBJ_VRAM0
                                    + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum
                                       << 5),
                                0x80);
                            DmaCopy16(
                                3, &gUnk_080635E8,
                                OBJ_VRAM0
                                    + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xE].tileNum
                                       << 5),
                                0x80);
                            DmaCopy16(
                                3, &gUnk_080635E8,
                                OBJ_VRAM0
                                    + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xF].tileNum
                                       << 5),
                                0x80);
                        } else {
                            DmaCopy16(
                                3, &gUnk_080B9068,
                                OBJ_VRAM0
                                    + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum
                                       << 5),
                                0x80);
                            DmaCopy16(
                                3, &gUnk_080B9068,
                                OBJ_VRAM0
                                    + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xE].tileNum
                                       << 5),
                                0x80);
                            DmaCopy16(
                                3, &gUnk_080B9068,
                                OBJ_VRAM0
                                    + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xF].tileNum
                                       << 5),
                                0x80);
                        }
                    }
                } else {
                    if (gUnk_03002920[var_sb].unkC_4 == 0) {
                        DmaCopy16(
                            3, &gUnk_08061FC8,
                            OBJ_VRAM0
                                + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                            0x80);
                    } else {
                        DmaCopy16(
                            3, &gUnk_080B8F68,
                            OBJ_VRAM0
                                + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                            0x80);
                    }
                    break;
                }
                break;

            case 0x6E:
                gUnk_03002920[var_sb].unkC_4 = gUnk_03005220.unk3_6;
                if (gUnk_03005220.unk3_6 == 0) {
                    DmaCopy16(
                        3, &gUnk_08063368,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x80);
                } else {
                    DmaCopy16(
                        3, &gUnk_080B92E8,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x80);
                }
                break;

            case 0x6D:
                temp_r3_3 = ((s32)gUnk_03004C20.unk8 >> ((gUnk_03004C20.room - 1) * 2)) & 3;
                if (gUnk_03004C10 == NULL) {
                    gUnk_03004C10 = OBJ_VRAM0
                        + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5);
                }

                if (temp_r3_3 == 0) {
                    DmaCopy16(3, &gUnk_080627C8, gUnk_03004C10, 0x80);
                } else if (temp_r3_3 == 1) {
                    DmaCopy16(3, &gUnk_080B90E8, gUnk_03004C10, 0x80);
                } else if (temp_r3_3 == 2) {
                    DmaCopy16(3, &gUnk_080B9168, gUnk_03004C10, 0x80);
                } else {
                    DmaCopy16(3, &gUnk_080B91E8, gUnk_03004C10, 0x80);
                }
                break;

            case 0x30:
                if ((gUnk_03002920[var_sb].unkC_4 == 3) || (gUnk_03002920[var_sb].unkC_4 == 1)) {
                    DmaCopy16(
                        3, &gUnk_08064868,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x200);
                } else {
                    DmaCopy16(
                        3, &gUnk_080B9468,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x200);
                }
                break;

            case 0x6F:
                if ((gUnk_03002920[var_sb].unkC_4 == 3) || (gUnk_03002920[var_sb].unkC_4 == 1)) {
                    DmaCopy16(
                        3, &gUnk_08064A68,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x200);
                } else {
                    DmaCopy16(
                        3, &gUnk_080B9668,
                        OBJ_VRAM0
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb - 0xD].tileNum << 5),
                        0x200);
                }
                break;

            case 0x32:
                if (sp1C == 0) {
                    SetPaletteAnimEntry(var_sb, 0);
                    sp1C = 1;
                }
                break;

            case 0x34:
                if (sp20 == 0) {
                    SetPaletteAnimEntry(var_sb, 2);
                    sp20 = 1;
                }
                break;

            case 0x33:
                if (sp24 == 0) {
                    SetPaletteAnimEntry(var_sb, 1);
                    sp24 = 1;
                }
                break;

            case 0x28:
                if (sp28 == 0) {
                    SetPaletteAnimEntry(var_sb, 0);
                    sp28 = 1;
                }
                break;

            case 0x20:
                SetPaletteAnimEntry(var_sb, 0);
                break;
        }
    }
}
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
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, gMosaicSize, gMosaicSize);
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
extern void RenderMenuUI(void);
extern void InitGameplayState(void);
extern void ProcessInputAndUpdateEntities(void);
extern void UpdateWorldMapInput(void);
extern void sub_0803A8B8(void);

/**
 * AnimatePaletteEffects: per-frame screen-shake, affine wobble, and register flush.
 *
 * Advances the two sine-driven screen-shake oscillators, applies the resulting
 * offset to all entities and the BG2 affine origin, builds the per-scanline BG2
 * scroll tables for worlds 5/6, then waits for VBlank and flushes the BG scroll,
 * affine, blend, and mosaic registers before pumping the RNG and sound.
 */
void AnimatePaletteEffects(void) {
    s8 sp0;
    s8 sp4;
    s32 temp_r0_2;
    s32 var_r3_2;
    s32 var_r8;
    s32 var_sb;
    u16 var_r2_2;
    u32 var_r5_2;
    u32 var_r6;
    u8 temp_r4;

    if (gUnk_03005400.unkE_1) {
        gUnk_03000001 = gUnk_03005400.unkD * 2;
        gUnk_03000003 = 0x20;
        gUnk_03005400.unkE_1 = 0;
    }

    if (gUnk_03005400.unkE_0) {
        gUnk_03000000 = gUnk_03005400.unkD * 2;
        gUnk_03000002 = 0x20;
        gUnk_03005400.unkE_0 = 0;
    }

    if (gUnk_03000000 != 0) {
        gUnk_03000002 += 0x20;
        gUnk_03000002 %= 0x100;
        if ((gUnk_03000002 % 0x80) == 0) {
            gUnk_03000000 -= 1;
        }
    }
    if (gUnk_03000001 != 0) {
        gUnk_03000003 += 0x20;
        gUnk_03000003 %= 0x100;
        if ((gUnk_03000003 % 0x80) == 0) {
            gUnk_03000001 -= 1;
        }
    }

    sp0 = ((s16)(gUnk_03000000 * gSineTable[gUnk_03000002]) >> 0x8) * 2;
    sp4 = ((s16)(-gUnk_03000001 * gSineTable[gUnk_03000003]) >> 0x8) * 2;
    TransformAllEntitiesToScreen(sp0, sp4);
    RenderMenuUI();

    if (gUnk_03004C20.world == 0x5 || gUnk_03004C20.world == 0x6) {
        if ((gCallbackQueue.current[1] == ProcessInputAndUpdateEntities) || (gCallbackQueue.current[0] == InitGameplayState) || (gCallbackQueue.current[1] == InitGameplayState) || (gCallbackQueue.current[1] == UpdateWorldMapInput) || (gCallbackQueue.current[1] == sub_0803A8B8)) {
            if (gUnk_030034BC == 0) {
                var_sb = 0;
                var_r8 = 1;
            } else {
                var_sb = 1;
                var_r8 = 0;
            }
            temp_r4 = (8 - Abs(8 - gUnk_030051F0.unkE)) * 8;
            var_r5_2 = gUnk_030051F0.unk0;
        } else {
            var_sb = 1;
            var_r8 = 1;
            temp_r4 = (8 - Abs(8 - gUnk_03005498)) * 8;
            var_r5_2 = gUnk_03004C20.unk0;
        }

        for (var_r6 = 0; var_r6 < 0xA0; var_r6++) {
            if (var_r6 < 0x90) {
                temp_r0_2 = (temp_r4 * gSineTable[(u8)(var_r5_2 * 4 + var_r6 * 4) + 0x40]) >> 8;
                gUnk_03004C40[var_r6] = temp_r0_2 * var_sb;
                gUnk_030052C0[var_r6] = -temp_r0_2 * var_r8;
            } else {
                gUnk_03004C40[var_r6] = 0;
                gUnk_030052C0[var_r6] = 0;
            }
        }
    }

    var_r3_2 = gUnk_03003430.bg2HOfs;
    var_r2_2 = gUnk_03003430.bg2VOfs;
    if (gUnk_03004C20.world == 0x4 && gUnk_03004C20.level == 0x8) {
        var_r2_2 += 0x20;
    }
    gBg2X = (((var_r3_2 << 8) - (var_r3_2 * gBg2PA)) - (var_r2_2 * gBg2PB)) + ((gUnk_03003430.bg2HOfs + sp0) << 8);
    gBg2Y = (((var_r2_2 << 8) - (var_r3_2 * gBg2PC)) - (var_r2_2 * gBg2PD)) + ((gUnk_03003430.bg2VOfs + sp4) << 8);

    VBlankIntrWait();

    REG_BG0HOFS = (gUnk_03003430.bg0HOfs >> 4) & 0x1FF;
    REG_BG0VOFS = (gUnk_03003430.bg0VOfs >> 7) & 0x1FF;
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
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, gMosaicSize, gMosaicSize);

    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
void VBlankCallback_Dialog(void) {
    RenderDialogSprites();
    gUnk_03004678 = SIN(gBg2Alpha);
    gUnk_030051B0 = COS(gBg2Alpha);
    VBlankIntrWait();
    REG_BLDALPHA = gUnk_03005498 | ((0x10 - gUnk_03005498) << 8);
    REG_BLDY = gUnk_03005498;
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, 0, 0);
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
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, gMosaicSize, gMosaicSize);
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
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, gMosaicSize, gMosaicSize);
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
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, gMosaicSize, gMosaicSize);
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
 * gUnk_03005220 / gUnk_03005284, and writes gCallbackQueue.next/
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
        gCallbackQueue.next[0] = ReadKeyInput;
    } else {
        gCallbackQueue.next[0] = ProcessInputAndTimers;
    }

    gUnk_03003410.unk5 = 0;
    gUnk_03003410.unk0 = 0;
    gUnk_03003410.unkB = 0;
    gUnk_030051E0 = 0;
    gUnk_030034C4 = 0xFE;
    gUnk_03003430.unk46 = 0;
    gUnk_03003430.unk44 = 0;

    if (gUnk_03004C20.level == 8) {
        gCallbackQueue.next[1] = InitLevelState;
        gCallbackQueue.next[2] = CameraModeSwitchHandler;
        gCallbackQueue.next[3] = UpdateUIState;
        gCallbackQueue.next[4] = TransitionInitLevelMusic;
        gCallbackQueue.next[5] = IntroScrollAnimation;
        gCallbackQueue.next[6] = AnimatePaletteEffects;
        gCallbackQueue.next[7] = 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 8;
    } else {
        gCallbackQueue.next[1] = HandlePauseMenuInput;
        if (gUnk_03004C20.unkB == 1) {
            gCallbackQueue.next[2] = UpdateCameraScrollPlayer2;
        } else if (gUnk_03004C20.level == 6) {
            gCallbackQueue.next[2] = UpdateCameraScroll;
        } else {
            gCallbackQueue.next[2] = ProcessOamSpriteLayout;
        }
        gCallbackQueue.next[3] = TransitionInitLevelMusic;

        if (arg0 < 2) {
            if (gUnk_03003410.unkA == 0) {
                gCallbackQueue.next[4] = IntroScrollAnimation;
                gCallbackQueue.next[5] = VBlankCallback_Gameplay;
                gCallbackQueue.next[6] = (IntrFunc)1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
                gCallbackQueue.nextCount = 7;
            } else {
                gCallbackQueue.next[4] = VBlankCallback_Gameplay;
                gCallbackQueue.next[5] = 1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
                gCallbackQueue.nextCount = 6;
            }
        } else {
            gUnk_03003410.unk5 = 1;
            gCallbackQueue.next[4] = VBlankCallback_Gameplay;
            gCallbackQueue.next[5] = 1;
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
            gCallbackQueue.nextCount = 6;
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
    REG_MOSAIC = MOSAIC_SET(gMosaicSize, gMosaicSize, gMosaicSize, gMosaicSize);
    thunk_UpdateRng();
    gUnk_03004C20.unk4 += 1;
    gUnk_03004C20.unk0 += 1;
    m4aSoundMain();
    gUnk_03003420 = 1;
}
INCLUDE_ASM("asm/nonmatchings/code_0", VBlankCallback_Credits); /* TextStateMachine — master UI/text state machine */
INCLUDE_ASM("asm/nonmatchings/code_0", sub_0800DE24);
