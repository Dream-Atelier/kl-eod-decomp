#include "global.h"
#include "gba.h"
#include "globals.h"
#include "include_asm.h"
#include "structs/variables.h"
/* Cross-module function prototypes referenced by the decompiled functions below.
 * (ROM data symbols live in structs/variables.h.) */
extern void AnimatePaletteEffects();
extern void CameraModeSwitchHandler();
extern void IntroSequenceUpdate();
extern void VBlankCallback_MinimalHW();
extern void InitVideoAndBG();
extern void TransitionFadeOutFull();
extern void WaitVBlankAndClearMosaic();
extern void InitWorldMapGfx();
extern void UpdateSceneTransition();
extern void IntroScrollAnimation();
extern void VBlankCallback_TitleScreen();
extern void LoadBGPalette();
extern void ResetVideoRegisters();
extern void InitLevelBG();
extern void TransitionFadeInRestoreWindows();
extern void UpdateAllEntities();
extern u8 gBlendValue;
extern u8 gSoundVolume;
extern struct MP2KPlayerState gMPlayInfo_0, gMPlayInfo_1, gMPlayInfo_2, gMPlayInfo_3;
extern struct EntityAnimationInfo gEntityAnimationInfo[];
void m4aMPlayVolumeControl(struct MP2KPlayerState *, u16, u16);
void m4aMPlayAllStop(void);
void m4aSongNumStart(u16);
void m4aSongNumContinue(u16);
void m4aSoundVSyncOff(void);
void m4aSoundVSyncOn(void);
#define PLTT ((void *)0x05000000)

#ifndef MAX
#define MAX(a, b) (((a) >= (b)) ? (a) : (b))
#endif

void ReadKeyInput(void);
void InitOamEntries(void);
void UpdateWorldMapScene(void);
void TransitionWorldMapFadeOut(void);
void VBlankCallback_Gameplay(void);
void SoftResetRom(u32);

void ResetEntityScrollState(s32 arg0);
void SpawnEntityAtPosition(u16, u16, u8, u8);
void EntityHitReaction(u8);
void SetPaletteAnimEntry(s32, u8);
void CopyBGScrollTiles(void);

INCLUDE_ASM("asm/nonmatchings/code_1", EntityUpdateDispatch);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerMainUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerMovementPhysics);
/* Result of a tile-collision probe: unk0 = collision coordinate (-1 = none),
 * unk2 = tile attribute byte. */
struct Unk_08014184 {
    u16 unk0;
    u8 unk2;
    u8 pad3[0x4 - 0x3];
};

/**
 * CheckTileCollisionVertical: find the floor segment under a vertical probe.
 *
 * Scans the active room's collision segment list for a horizontal segment whose
 * span brackets arg1 (X) and whose top edge lies within [arg2-arg3, arg2] (Y),
 * returning its left edge (minus 3) and tile attribute through arg0.
 */
struct Unk_08014184 *CheckTileCollisionVertical(struct Unk_08014184 *arg0, u16 arg1, u16 arg2, u8 arg3) {
    u32 var_r3;
    struct Unk_08014184 var_r4;

    for (var_r3 = gUnk_03004D80->unk2; var_r3 < gUnk_03004D80->unk0; var_r3++) {
        if ((arg2 >= gUnk_03004D80->unk4[var_r3].unk2) && (gUnk_03004D80->unk4[var_r3].unk6 >= (arg2 - arg3))
            && (arg1 < (gUnk_03004D80->unk4[var_r3].unk0 + 3)) && ((gUnk_03004D80->unk4[var_r3].unk0 - 3) < arg1)) {
            var_r4.unk0 = gUnk_03004D80->unk4[var_r3].unk0 - 3;
            var_r4.unk2 = gUnk_03004D80->unk4[var_r3].unk8;
            *arg0 = var_r4;
            goto exit; // FAKE
            return arg0;
        }
    }

    var_r4.unk0 = -1;
    *arg0 = var_r4;
exit:
    return arg0;
}

/**
 * CheckTileCollisionSloped: find the wall/slope segment for a horizontal probe.
 *
 * Scans the collision segment list for a vertical or sloped segment crossing arg1
 * (X); for sloped segments it interpolates the contact Y from the segment's
 * endpoints, returning the contact coordinate and tile attribute through arg0.
 */
struct Unk_08014184 *CheckTileCollisionSloped(struct Unk_08014184 *arg0, u16 arg1, u16 arg2, u8 arg3) {
    s32 temp_r1_2;
    struct Unk_08014184 var_r5;
    u32 var_r3;

    var_r5.unk0 = -1;

    for (var_r3 = 0; var_r3 < gUnk_03004D80->unk2; var_r3++) {
        if (gUnk_03004D80->unk4[var_r3].unk4 >= arg1) {
            if (arg1 >= gUnk_03004D80->unk4[var_r3].unk0) {
                if (gUnk_03004D80->unk4[var_r3].unk2 == gUnk_03004D80->unk4[var_r3].unk6) {
                    if (((arg2 - arg3) <= gUnk_03004D80->unk4[var_r3].unk2) && (gUnk_03004D80->unk4[var_r3].unk2 <= arg2)) {
                        var_r5.unk0 = gUnk_03004D80->unk4[var_r3].unk2;
                        var_r5.unk2 = gUnk_03004D80->unk4[var_r3].unk8;
                        *arg0 = var_r5;
                        goto exit;
                        return arg0;
                    } else {
                        continue;
                    }
                } else {
                    temp_r1_2 = (((gUnk_03004D80->unk4[var_r3].unk6 - gUnk_03004D80->unk4[var_r3].unk2)
                                  * (arg1 - gUnk_03004D80->unk4[var_r3].unk0))
                                 / (gUnk_03004D80->unk4[var_r3].unk4 - gUnk_03004D80->unk4[var_r3].unk0))
                        + gUnk_03004D80->unk4[var_r3].unk2;
                    if ((temp_r1_2 >= (arg2 - arg3)) && (temp_r1_2 <= (arg2 + 3))) {
                        var_r5.unk0 = temp_r1_2;
                        var_r5.unk2 = gUnk_03004D80->unk4[var_r3].unk8;
                    } else {
                        continue;
                    }
                }
            }

            *arg0 = var_r5;
        exit:
            return arg0;
        }
    }

    *arg0 = var_r5;
    return arg0;
}
/**
 * ApplyEntityTileMovement: move the player by the pending scroll delta if clear.
 *
 * Samples the BG2 collision map at the destination (per horizontal/vertical scroll
 * direction); if the highest tile attribute is below the solid threshold, commits
 * the move and copies the carried-entity sub-position nibbles, otherwise cancels
 * the scroll and resets the carry/scroll state.
 */
void ApplyEntityTileMovement(void) {
    u8 var_r3;

    var_r3 = 0;
    if (gUnk_03005220.unk56 > 0) {
        var_r3 = gUnk_03004790
                     .pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2 + 0xC) >> 3)
                                     + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 4) >> 3) * gUnk_03003430.bg2MapWidth)];

        // var_r3 = (var_r3 < gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2 + 0xC) >> 3) +
        // ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0xC) >> 3) * gUnk_03003430.bg2MapWidth)]) ?
        // (gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2 + 0xC) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 0xC) >> 3) * gUnk_03003430.bg2MapWidth)]) : var_r3; var_r3 = (var_r3 <
        // gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2 + 0xC) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 0x14) >> 3) * gUnk_03003430.bg2MapWidth)]) ? (gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56
        // + gUnk_03002920[0].xPosBg2 + 0xC) >> 3) + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0x14) >> 3) *
        // gUnk_03003430.bg2MapWidth)]) : var_r3;

        var_r3 = MAX(
            var_r3,
            gUnk_03004790
                .pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2 + 0xC) >> 3)
                                + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0xC) >> 3) * gUnk_03003430.bg2MapWidth)]);
        var_r3 = MAX(
            var_r3,
            gUnk_03004790
                .pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2 + 0xC) >> 3)
                                + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0x14) >> 3) * gUnk_03003430.bg2MapWidth)]);
    } else if (gUnk_03005220.unk56 < 0) {
        var_r3 = gUnk_03004790
                     .pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3)
                                     + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 4) >> 3) * gUnk_03003430.bg2MapWidth)];

        // var_r3 = (var_r3 < gUnk_03004790.pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3) +
        // ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0xC) >> 3) * gUnk_03003430.bg2MapWidth)]) ?
        // (gUnk_03004790.pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 0xC) >> 3) * gUnk_03003430.bg2MapWidth)]) : var_r3; var_r3 = (var_r3 <
        // gUnk_03004790.pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 0x14) >> 3) * gUnk_03003430.bg2MapWidth)]) ?
        // (gUnk_03004790.pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 0x14) >> 3) * gUnk_03003430.bg2MapWidth)]) : var_r3;

        var_r3 = MAX(
            var_r3,
            gUnk_03004790
                .pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3)
                                + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0xC) >> 3) * gUnk_03003430.bg2MapWidth)]);
        var_r3 = MAX(
            var_r3,
            gUnk_03004790
                .pBufBg2Tilemap[(((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) - 0xD) >> 3)
                                + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0x14) >> 3) * gUnk_03003430.bg2MapWidth)]);
    }

    if (gUnk_03005220.unk57 != 0) {
        // var_r3 = (var_r3 < gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) >> 3) +
        // ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0x1A) >> 3) * gUnk_03003430.bg2MapWidth)]) ?
        // (gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 0x1A) >> 3) * gUnk_03003430.bg2MapWidth)]) : var_r3; var_r3 = (var_r3 <
        // gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) >> 3) + ((((gUnk_03005220.unk57 +
        // gUnk_03002920[0].yPosBg2) - 4) >> 3) * gUnk_03003430.bg2MapWidth)]) ? (gUnk_03004790.pBufBg2Tilemap[((gUnk_03005220.unk56 +
        // gUnk_03002920[0].xPosBg2) >> 3) + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 4) >> 3) *
        // gUnk_03003430.bg2MapWidth)]) : var_r3;

        var_r3 = MAX(
            var_r3,
            gUnk_03004790
                .pBufBg2Tilemap[((gUnk_03002920[0].xPosBg2 + gUnk_03005220.unk56) >> 3)
                                + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 0x1A) >> 3) * gUnk_03003430.bg2MapWidth)]);
        var_r3
            = MAX(var_r3,
                  gUnk_03004790
                      .pBufBg2Tilemap[((gUnk_03005220.unk56 + gUnk_03002920[0].xPosBg2) >> 3)
                                      + ((((gUnk_03005220.unk57 + gUnk_03002920[0].yPosBg2) - 4) >> 3) * gUnk_03003430.bg2MapWidth)]);
    }

    if (var_r3 < gUnk_03004654[0x1A]) {
        gUnk_03002920[0].xPosBg2 += gUnk_03005220.unk56;
        gUnk_03002920[0].yPosBg2 += gUnk_03005220.unk57;
        gUnk_03002920[0].xOffset = gUnk_03002920[gUnk_03005220.unk3F].xOffset;
        gUnk_03002920[0].yOffset = gUnk_03002920[gUnk_03005220.unk3F].yOffset;
        return;
    }

    gUnk_03005220.unk57 = 0;
    gUnk_03005220.unk56 = 0;
    gUnk_03005220.unk3F = 0;
    ResetEntityScrollState(1);
    if ((gUnk_03005220.unk34 | gUnk_03005220.unk39) != 0) {
        SetPaletteAnimEntry(0, 0);
        gUnk_03005220.unk39 = 0;
        gUnk_03005220.unk34 = 0;
    }
}
/**
 * InitScrollState: reset the entire player/scroll state block at level start.
 *
 * Clears the scroll link, the ~40 player-state fields (carry, jump, push,
 * camera-follow, etc.), seeds the few non-zero defaults (unk5C/unk3D/unk16),
 * disables the player's affine flag, and re-seeds the palette animation.
 */
void InitScrollState(void) {
    ResetEntityScrollState(1);

    gUnk_03005220.unk5D = gUnk_03002920[0x9].onScreen = gUnk_03002920[0xA].onScreen = 0;
    gUnk_03005220.unk47 = 0;
    gUnk_03005220.deathSequenceTimer = 0;
    gUnk_03005220.unk38 = 0;
    gUnk_03005220.unk43 = 0;
    gUnk_03005220.unk42 = 0;
    gUnk_03005220.unk48 = 0;
    gUnk_03005220.unk49 = 0;
    gUnk_03005220.unk4A = 0;
    gUnk_03005220.unk4B = 0;
    gUnk_03005220.unk59 = 0;
    gUnk_03005220.unk39 = 0;
    gUnk_03005220.unk5A = 0;
    gUnk_03005220.unk53 = 0;
    gUnk_03005220.unk3B = 0;
    gUnk_03005220.unk3A = 0;
    gUnk_03005220.unk45 = 0;
    gUnk_03005220.unk37 = 0;
    gUnk_03005220.unk36 = 0;
    gUnk_03005220.unk35 = 0;
    gUnk_03005220.unk34 = 0;
    gUnk_03005220.unk31 = 0;
    gUnk_03005220.unk30 = 0;
    gUnk_03005220.unk33 = 0;
    gUnk_03005220.unk41 = 0;
    gUnk_03005220.unk40 = 0;
    gUnk_03005220.unk3F = 0;
    gUnk_03005220.unk3E = 0;
    gUnk_03005220.unk3C = 0;
    gUnk_03005220.unk55 = 0;
    gUnk_03005220.unk54 = 0;
    gUnk_03005220.unk57 = 0;
    gUnk_03005220.unk56 = 0;
    gUnk_03005220.unk2C = 0;
    gUnk_03005220.unk2A = 0;
    gUnk_03005220.unk28 = 0;
    gUnk_03005220.unk26 = 0;
    gUnk_03005220.unk5C = 1;
    gUnk_03005220.unk3D = 1;
    gUnk_03002920[0].onScreen = 1;
    gUnk_03005220.unk16 = 0x230;
    gUnk_03005220.unk18 = 0;
    gUnk_03002920[0].affineEnable = 0;

    SetPaletteAnimEntry(0, 0);
}
/**
 * ResetEntityScrollState: clear the player's "carried entity" scroll link.
 *
 * Disables the affine flag on the currently-linked entity (outside level 8),
 * clears the carry-link fields, and — when arg0 == 1 — flushes any queued
 * palette-animation entries past the base count.
 */
void ResetEntityScrollState(s32 arg0) {
    if (gUnk_03003410.unkB == 0) {
        if (gUnk_03004C20.level != 8) {
            if (gUnk_03002920[gUnk_03005220.unk42].affineEnable != 0) {
                gUnk_03002920[gUnk_03005220.unk42].affineEnable = 0;
            }
        }

        gUnk_03005220.unk38 = 0;
        gUnk_03005220.unk43 = 0;
        gUnk_03005220.unk42 = 0;

        if ((arg0 == 1) && (gUnk_03000830->unk0 >= 0x16)) {
            SetPaletteAnimEntry(0, gUnk_03000830->unk0 - 0x16);
        }
    }
}

/**
 * PlayerRespawnOrDeath: tick the death/respawn timer and trigger the outcome.
 *
 * When arg0 == 1 the player lost a life: decrements the heart count
 * (gUnk_03005220.hearts). If hearts remain, resets the scroll/carry state for the
 * respawn; once hearts reach 0 it plays the game-over sound and queues the fade.
 * Sets up the post-death camera/blend state.
 */
void PlayerRespawnOrDeath(s32 arg0) {
    if ((gUnk_03005220.deathSequenceTimer | gUnk_03003410.unkB | gUnk_030034E4) != 0) {
        return;
    }

    gUnk_03005220.unk5B = 0;
    if (arg0 == 1) {
        gUnk_03005220.hearts -= 1;
        CopyBGScrollTiles();
        gUnk_03005220.unk5B = 1;
    }

    if (gUnk_03005220.hearts == 0) {
        m4aSongNumStart(0x27);

        gUnk_03005220.deathSequenceTimer = 0x46;
        gUnk_03002920[0x9].onScreen = 0;
        gUnk_03002920[0xA].onScreen = 0;
        gUnk_03005220.unk57 = 0;
        gUnk_03005220.unk56 = 0;
        gUnk_03005220.unk3F = 0;
        gUnk_03005220.unk3B = 0;
        gUnk_03005220.unk3A = 0;
        gUnk_03005220.unk39 = 0;
        gUnk_03005220.unk34 = 0;

        if (gUnk_03005220.unk42 != 0) {
            if ((gUnk_03002920[gUnk_03005220.unk42].kind != ENTITY_KIND_BOX) && (gUnk_03002920[gUnk_03005220.unk42].kind != 0x25)) {
                SpawnEntityAtPosition(gUnk_03002920[gUnk_03005220.unk42].xPosBg2, gUnk_03002920[gUnk_03005220.unk42].yPosBg2, 2,
                                      gUnk_03005220.unk42);
            }
        }

        gUnk_03005220.unk38 = 0;
        gUnk_03005220.unk43 = 0;
        gUnk_03005220.unk42 = 0;
    } else {
        m4aSongNumStart(0x25);
        gUnk_03005220.unk3C = 0;
        SetPaletteAnimEntry(0, 0xC);
    }

    gUnk_03005220.unk1C = 0;
    gUnk_03005220.unk3E = 0x87;
    if (gUnk_03005220.unk3D > 1) {
        gUnk_03005220.unk3D = 1;
        m4aSongNumStart(0x8E);
    }

    gUnk_03005220.unk44 = gUnk_03002920[0].flip;
    gUnk_03005220.unk3C = 0;
    gUnk_03005220.unk26 = 0;
    gUnk_03005220.unk28 = 0;
}
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBehaviorMasterUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", EntitySpawnFromLevelData);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerFollowEntityMovement);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerGrabInputCheck);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerEntityCollisionCheck);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityStateSwitch_Carried);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityPairUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityProximityDamageCheck);
INCLUDE_ASM("asm/nonmatchings/code_1", EntitySpriteFrameUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityPositionFromLevelTable);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityGravityAndFloorCheck);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityThrowUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityPlatformRide);
/**
 * Entity struct used by EntityDeathAnimation. 28-byte entries in
 * gEntityArray (0x03002920). Index pattern: (slot * 7) * 4 = slot * 28.
 */
typedef struct EntityDeathStruct {
    u16 x; /* +0x00: screen-space X */
    u16 y; /* +0x02: screen-space Y */
    u8 unk04;
    u8 unk05;
    u8 unk06;
    u8 unk07;
    u8 phase; /* +0x08: death phase counter (decrements each cycle) */
    u8 timer; /* +0x09: frame countdown (resets to 25 on underflow) */
    u8 slotIdx; /* +0x0A */
    u8 unk0B;
    u8 unk0C;
    u8 unk0D;
    u8 unk0E;
    u8 typeId; /* +0x0F: entity type (0x1C = inactive) */
    u8 subState; /* +0x10 */
    u8 behaviorId; /* +0x11 */
    u8 direction; /* +0x12 */
    u8 unk13[9];
} EntityDeathStruct;

extern void SpawnEntityAtPosition(u16 x, u16 y, u8 type, u8 slot);
extern u8 sub_08051A0C(u8 a, u8 b);
extern u8 sub_08051A84(u8 a, u8 b);
extern void LoadSpriteFrame(u8 frame, u8 tilesetIdx);

/**
 * EntityDeathAnimation: multi-frame death/destruction animation.
 *
 * Each frame, decrements a timer. When the timer underflows (wraps to 0xFF),
 * resets it to 25 frames and advances the death phase:
 *   phase > 5: spawns a visual effect entity (type = phase + 0x0C)
 *   phase > 0: plays SFX 0x56, updates sprite frames for the dying
 *              and paired entities via sub_08051A0C / sub_08051A84
 *   phase == 0: spawns a final entity (type 0x02) at the parent position
 *
 * Every frame (regardless of phase), copies position from the parent entity
 * (slot - offset) with a 32-pixel upward Y shift. If phase > 9, also
 * positions a paired entity (slot + offset) with a 3-pixel X offset.
 *
 * Uses gEntityDeathState[0] as the slot offset between linked entity pairs.
 */
void EntityDeathAnimation(u8 slot) {
    u8 phase;
    s16 byteMask;
    int newPhase;
    EntityDeathStruct *entities = (EntityDeathStruct *)gEntityArray;
    EntityDeathStruct *entity = &entities[slot];
    EntityDeathStruct *arr;
    EntityDeathStruct *self;

    entity->timer = entity->timer - 1;
    byteMask = 0xFF;
    if (entity->timer == 0xFF) {
        entity->timer = 25;
        phase = entity->phase;
        if (phase <= 5) {
            SpawnEntityAtPosition(entities[slot - *gEntityDeathState].x, entities[slot - *gEntityDeathState].y, (u8)(phase + 0x0C), 0);
        }
        newPhase = entity->phase - 1;
        entity->phase = newPhase;
        do {
            if ((newPhase & byteMask) == 0) {
                do {
                    SpawnEntityAtPosition(entities[slot - *gEntityDeathState].x, entities[slot - *gEntityDeathState].y, 2,
                                          (u8)(slot - *gEntityDeathState));
                } while (0);
            } else {
                m4aSongNumStart(0x56);
                if (entity->phase > 9) {
                    entities[slot + *gEntityDeathState].typeId = 0;
                    LoadSpriteFrame((u8)(slot + *gEntityDeathState), sub_08051A0C(entity->phase, 0x0A));
                }
                LoadSpriteFrame(slot, sub_08051A84(entity->phase, 0x0A));
                if (entity->phase == 9) {
                    entities[slot + *gEntityDeathState].typeId = 0x1C; /* inactive */
                    entities[slot + *gEntityDeathState].subState = 0;
                }
            }
        } while (0);
    }
    arr = (EntityDeathStruct *)gEntityArray;
    self = &arr[slot];
    byteMask |= 0;
    self->x = arr[slot - *gEntityDeathState].x;
    self->y = arr[slot - *gEntityDeathState].y - 0x20;
    if (self->phase > 9) {
        arr[slot + *gEntityDeathState].x = arr[slot - *gEntityDeathState].x - 3;
        arr[slot + *gEntityDeathState].y = arr[slot - *gEntityDeathState].y - 0x20;
        self->x = self->x + 3;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBounceOffWall);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityFloatPath);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityPickupCollect);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityProjectileUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", SpawnEntityAtPosition);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityHitReaction);
INCLUDE_ASM("asm/nonmatchings/code_1", EntitySpriteFlipAndLoad);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityPositionFromROMTable);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityScrollBoundsCheck);
struct EntityBase {
    u16 unk_0;
    u16 unk_2;
    u8 pad_4[0x8 - 0x4];
    s8 unk_8;
    s8 unk_9;
    u8 pad_A[0xC - 0xA];
    u8 unk_C_0 : 2;
    u8 pad_D[0xF - 0xD];
    u8 unk_F;
    u8 unk_10;
    u8 pad_11[0x14 - 0x11];
    u16 unk_14;
    u8 unk_16;
    u8 pad_17[0x1C - 0x17];
}; /* size = 0x1C */

struct Entity {
    struct EntityBase base[1]; // TODO: size
    u8 pad_C[0x1F8 - 0x1C];
    u16 unk_1F8;
    u16 unk_1FA;
    u8 pad_1FA[0x204 - 0x1FC];
    u32 unk_204_0 : 2;
    u32 unk_204_2 : 2;
};

extern struct Entity gEntity;

void EntityItemDrop(u8 arg0) {
    u8 itemType = arg0 + 0xE4;

    if (gGameFlagsPtr[0x0A] == 1) {
        gEntity.base[arg0].unk_F = 0x1C;
        gEntity.base[arg0].unk_10 = 0;
        return;
    }

    switch (gEntity.base[arg0].unk_F) {
        case 3: {
            gEntity.base[arg0].unk_F = 0;
            gEntity.base[arg0].unk_14 = 0;
            gEntity.base[arg0].unk_10 = 1;
            gEntity.base[arg0].unk_C_0 = 0;

            if (gEntity.unk_204_2 == 0) {
                gEntity.base[arg0].unk_0 = gEntity.unk_1F8 + 0x10;
            } else {
                gEntity.base[arg0].unk_0 = gEntity.unk_1F8 - 0x10;
            }

            gEntity.base[arg0].unk_2 = gEntity.unk_1FA;
            gEntity.base[arg0].unk_8 = gItemDropParamTable.unk_0[itemType][0];
            gEntity.base[arg0].unk_9 = gItemDropParamTable.unk_0[itemType][1];
            gEntity.base[arg0].unk_16 = 4;
            break;
        }

        case 4: {
            gEntity.base[arg0].unk_F = 0;
            gEntity.base[arg0].unk_14 = 0;
            gEntity.base[arg0].unk_10 = 1;
            gEntity.base[arg0].unk_C_0 = 0;

            if (gEntity.unk_204_2 == 0) {
                gEntity.base[arg0].unk_0 = gEntity.unk_1F8 + 0x10;
            } else {
                gEntity.base[arg0].unk_0 = gEntity.unk_1F8 - 0x10;
            }

            gEntity.base[arg0].unk_2 = gEntity.unk_1FA;
            gEntity.base[arg0].unk_8 = gItemDropParamTable.unk_A[itemType][0];
            gEntity.base[arg0].unk_9 = gItemDropParamTable.unk_A[itemType][1];
            gEntity.base[arg0].unk_16 = 2;
            break;
        }

        case 0: {
            gEntity.base[arg0].unk_2 = 0x10C - ((gEntity.base[arg0].unk_9 * gSineTable[gEntity.base[arg0].unk_14]) >> 8);
            gEntity.base[arg0].unk_0 += gEntity.base[arg0].unk_8;

            gEntity.base[arg0].unk_14 += gEntity.base[arg0].unk_16;
            if (gEntity.base[arg0].unk_14 == 0x88) {
                gEntity.base[arg0].unk_F = 0x1C;
                gEntity.base[arg0].unk_10 = 0;
            }
            break;
        }

        default:
            break;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_1", EntityTimerAction);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityComplexBehavior);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityMovingObstacle);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBossPhaseA);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityCrushingBlock);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBossPhaseB);
INCLUDE_ASM("asm/nonmatchings/code_1", EntitySpringBoard);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityCutsceneActor);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBossPhaseC);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBossPhaseD);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityMiniBoss);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityMiniBossAlt);
/**
 * TransitionFadeOutDisableIRQ: per-frame fade-to-black step (every other
 * frame). Darkens all layers via BLDCNT, decrements the blend value, and once
 * it reaches zero removes this callback from the queue.
 */
void TransitionFadeOutDisableIRQ(void) {
    u32 removed;
    u32 i;

    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue -= 1;
    if (gBlendValue == 0) {
        // Remove TransitionFadeOutDisableIRQ from callback queue
        removed = FALSE;
        for (i = 0; i < (gCallbackQueue.currentCount - 1); i++) {
            if ((gCallbackQueue.current[i] == TransitionFadeOutDisableIRQ) || (removed == TRUE)) {
                gCallbackQueue.next[i] = gCallbackQueue.current[i + 1];
                removed = TRUE;
            } else {
                gCallbackQueue.next[i] = gCallbackQueue.current[i];
            }
        }
        if (removed == TRUE) {
            gCallbackQueue.nextCount = gCallbackQueue.currentCount - 1;
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        }

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
        gUnk_030034E4 = 0;
    } else {
        gMosaicSize -= 1;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeInBldAlpha);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionInitLevelMusic);
/**
 * TransitionToWorldMap: fades to black then sets up world map scene.
 */
void TransitionToWorldMap(void) {
    u32 *sceneCtrl;
    u32 isActive;
    u32 *callbackState;
    u32 slotIdx;

    gPauseFlag = 1;

    sceneCtrl = (u32 *)gControlBlock;
    isActive = sceneCtrl[1] & 1;
    if (isActive != 0)
        return;

    REG_BLDCNT = 0xFF;

    gFrameCounter += 1;
    if (gFrameCounter != 16)
        return;

    InitOamEntries();
    sceneCtrl[0] = (u32)-1;

    callbackState = gCallbackStateArray;
    callbackState[0x28 / 4] = (u32)ReadKeyInput;
    callbackState[0x2C / 4] = (u32)UpdateWorldMapScene;
    callbackState[0x30 / 4] = (u32)TransitionWorldMapFadeOut;
    callbackState[0x34 / 4] = (u32)VBlankCallback_Gameplay;
    callbackState[0x38 / 4] = 1;
    slotIdx = *((u8 *)callbackState + 0x78) - 1;
    callbackState[slotIdx] = isActive;
    *((u8 *)callbackState + 0x79) = 5;
}
/**
 * TransitionGameplayInit: kleod TransitionGameplayInit.
 */
void TransitionGameplayInit(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        gUnk_030034E4 = 0;
        FreeAllDecompBuffers();
        gUnk_03004C20.sceneFrameCounter = -1;
        gCallbackQueue.next[0] = ReadKeyInput;
        if (gUnk_03003410.unk7 == 1) {
            gCallbackQueue.next[0] = ReadKeyInput; // Redundant, required to match
            if (gUnk_03004C20.world == 6) {
                gCallbackQueue.next[1] = UpdateAllEntities;
                gCallbackQueue.next[2] = TransitionFadeInRestoreWindows;
                gCallbackQueue.next[3] = VBlankCallback_Gameplay;
                gCallbackQueue.next[4] = NULL + 1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
                gCallbackQueue.nextCount = 5;
            } else {
                gUnk_03004C20.level = 0;
                gUnk_03003410.unk9 = 0;
                gUnk_03003410.unkA = 0;
                gCallbackQueue.next[0] = InitLevelBG;
                gUnk_03003410.unk8 = 1;
                gCallbackQueue.next[1] = ResetVideoRegisters;
                gCallbackQueue.next[2] = NULL + 1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
                gCallbackQueue.nextCount = 3;
            }
            gUnk_03004C20.sceneFrameCounter = -1;
        } else {
            gEntityInfo[0xB].unk10 = 0;
            gMosaicSize = 0;
            gCallbackQueue.next[1] = LoadBGPalette;
            gCallbackQueue.next[2] = VBlankCallback_TitleScreen;
            gCallbackQueue.next[3] = NULL + 1;
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
            gCallbackQueue.nextCount = 4;
            gUnk_03004C20.sceneFrameCounter = -1;
            gUnk_03004D9C = 0;
        }
    } else {
        gMosaicSize += 1;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutWithMusic);
/**
 * TransitionWorldMapFadeOut: kleod TransitionWorldMapFadeOut.
 */
void TransitionWorldMapFadeOut(void) {
    u32 removed;
    u32 i;

    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue -= 1;
    if (gBlendValue == 0) {
        // remove TransitionWorldMapFadeOut from callback queue
        // TODO: do while required to match, callback removal possibly a macro
        do {
            removed = FALSE;
            for (i = 0; i < (gCallbackQueue.currentCount - 1); i++) {
                if ((gCallbackQueue.current[i] == TransitionWorldMapFadeOut) || (removed == TRUE)) {
                    gCallbackQueue.next[i] = gCallbackQueue.current[i + 1];
                    removed = TRUE;
                } else {
                    gCallbackQueue.next[i] = gCallbackQueue.current[i];
                }
            }
            if (removed == TRUE) {
                gCallbackQueue.nextCount = gCallbackQueue.currentCount - 1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
            }
        } while (0);

        gUnk_03004C20.sceneFrameCounter = 0;
        gUnk_030034E4 = 0;
    }
}
/**
 * TransitionToSceneSelect: kleod TransitionToSceneSelect.
 */
void TransitionToSceneSelect(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        gUnk_030034E4 = 0;
        FreeAllDecompBuffers();

        gBg2XMag = gBg2YMag = 0x100;
        gBg2Alpha = 0;

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;

        gUnk_03004658[0xC] = 0;
        gCallbackQueue.next[0] = ReadKeyInput;
        gCallbackQueue.next[1] = UpdateSceneTransition;
        gCallbackQueue.next[2] = VBlankCallback_Gameplay;
        gCallbackQueue.next[3] = NULL + 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 4;
        gUnk_03004C20.sceneFrameCounter = -1;
    } else {
        gMosaicSize += 1;
    }
}
/**
 * TransitionToTitleScreen: kleod TransitionToTitleScreen.
 */
void TransitionToTitleScreen(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        gUnk_030034E4 = 0;
        FreeAllDecompBuffers();

        gBg2XMag = gBg2YMag = 0x100;
        gBg2Alpha = 0;

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;

        gUnk_03004658[0xC] = 0;
        gCallbackQueue.next[0] += 0; // FAKE
        gUnk_03003410.unkA = gUnk_03003410.unk9 = 0;
        gCallbackQueue.next[0] = InitLevelBG;
        gUnk_03003410.unk8 = 1;
        gCallbackQueue.next[1] = ResetVideoRegisters;
        gCallbackQueue.next[2] = NULL + 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 3;
        gUnk_03004C20.sceneFrameCounter = -1;
    } else {
        gMosaicSize += 1;
    }
}
/**
 * TransitionGameOver: per-frame fade-out step (every other frame) run after a game over; once fully darkened, frees decomp buffers,
 * resets BG2 affine/HBlank state, records save progress, and queues either the world-map return or the next-level setup depending on
 * world/level.
 */
void TransitionGameOver(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        gUnk_030034E4 = 0;
        FreeAllDecompBuffers();

        gBg2XMag = gBg2YMag = 0x100;
        gBg2Alpha = 0;

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
        gUnk_03004658[0xC] = 0;

        if (gUnk_03005220.unk37 == 0) {
            if ((gUnk_03004C20.world != 6) && (gUnk_03004C20.level == 8)) {
                gUnk_03005284->unk4 = gUnk_03004C20.world * 3;
                gBlendValue = BLEND_MAX;
                gCallbackQueue.next[0] = InitWorldMapGfx;
                gCallbackQueue.next[1] = NULL + 1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
                gCallbackQueue.nextCount = 2;
            } else {
                ClearVideoState();
                gUnk_03003410.unk9 = 0;
                gUnk_03003410.unkA = 0;
                gCallbackQueue.next[0] = InitLevelBG;
                gUnk_03003410.unk8 = 1;
                gCallbackQueue.next[1] = ResetVideoRegisters;
                gCallbackQueue.next[2] = NULL + 1;
                gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
                gCallbackQueue.nextCount = 3;
            }
        }
        gUnk_03004C20.sceneFrameCounter = -1;
    } else {
        gMosaicSize += 1;
    }
}
#define VCOUNT_SPLIT_LINE   143
#define MUSIC_FADE_STEP     16
#define AUTO_ADVANCE_FRAMES 250
#define SONG_ID_78          0x78

/**
 * sub_08024D84: one frame of the ending/credits hand-off step.
 *
 * Installed as gCallbackQueue.next[1] by VBlankCallback_Credits (code_0), and run
 * once per frame while gUnk_030034E4 (transition-busy) is held at 1.
 *
 * Two independent jobs:
 *  1. A 16-step BLDY darken fade driven by gBlendValue. On step 0 it parks every
 *     entity but the player (unkF = 0x1C, onScreen = 0) and marks entity 0 visible.
 *     On step 1 it arms a VCount=143 raster split (WaitVBlankAndClearMosaic on the
 *     VCount IRQ) and switches BLDCNT to darken. Every step ramps the m4a master
 *     volume down by 16; on step 16 it stops all players, restores full volume,
 *     starts song 0x78 and refreshes the HUD collectible count. Steps 2..15 only
 *     run on every 8th global frame (globalFrameCounter & 7).
 *  2. An auto-advance: A/START, or ~250 frames of gUnk_03003410.unk0, restores the
 *     player to 3 hearts, silences and re-arms the mixer, opens the windows and
 *     queues TransitionFadeOutFull, clearing the blend and mosaic levels.
 *
 * Matching notes (agbcc 2.95):
 *  - the `goto checkAdvance` is load-bearing: the same shape written as
 *    do { ... break; ... } while (0) makes agbcc re-order the whole prologue.
 *  - `dispStat` must be a separate statement; folding it into one expression makes
 *    agbcc narrow 0xFFFF8F00 to a `movs #143; lsls #8` pair instead of a pool word.
 *  - `gBlendValue = gMosaicSize = 0;` (chained) emits both address loads before the
 *    two stores, which two separate statements do not.
 */
void sub_08024D84(void) {
    u32 i;
    s32 dispStat;

    gUnk_030034E4 = 1;

    if (gBlendValue == 0) {
        for (i = 1; i < gUnk_03005428; i++) {
            gUnk_03002920[i].onScreen = 0;
            gUnk_03002920[i].unkF = 0x1C;
        }
        gUnk_03002920[0].onScreen = 1;
    } else {
        if ((gUnk_03004C20.globalFrameCounter & 7) != 0) {
            goto checkAdvance;
        }
        if (gBlendValue >= BLEND_MAX) {
            goto checkAdvance;
        }
    }

    if (gBlendValue == 1) {
        dispStat = REG_DISPSTAT & 0xFF;
        REG_DISPSTAT = dispStat | (VCOUNT_SPLIT_LINE << 8);
        gIntrTable.vCount = WaitVBlankAndClearMosaic;
        REG_IE |= INTR_FLAG_VCOUNT;
        REG_DISPSTAT |= DISPSTAT_VCOUNT_INTR;
        REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BD;
        if (gUnk_03004C20.world == 6) {
            if ((gUnk_03004C20.level == 1) || (gUnk_03004C20.level == 3)) {
                REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_CLR | WININ_WIN1_BG0;
            }
        }
    }

    gUnk_03005210 -= MUSIC_FADE_STEP;
    if (gUnk_03005210 > MUSIC_FADE_STEP) {
        m4aMPlayVolumeControl(&gMPlayInfo_0, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_1, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_2, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_3, 0xFF, gUnk_03005210);
    } else {
        m4aMPlayVolumeControl(&gMPlayInfo_0, 0xFF, MUSIC_FADE_STEP);
        m4aMPlayVolumeControl(&gMPlayInfo_1, 0xFF, MUSIC_FADE_STEP);
        m4aMPlayVolumeControl(&gMPlayInfo_2, 0xFF, MUSIC_FADE_STEP);
        m4aMPlayVolumeControl(&gMPlayInfo_3, 0xFF, MUSIC_FADE_STEP);
    }

    gBlendValue++;
    if (gBlendValue == BLEND_MAX) {
        m4aMPlayAllStop();
        gUnk_03005210 = 0x100;
        m4aMPlayVolumeControl(&gMPlayInfo_0, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_1, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_2, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_3, 0xFF, gUnk_03005210);
        m4aSongNumStart(SONG_ID_78);
        UpdateHUDCollectibleCount();
    } else if (gBlendValue == 9) {
        gUnk_03002920[0].priority = 0;
    }

checkAdvance:
    if ((((gNewKeys & (A_BUTTON | START_BUTTON)) != 0) || (gUnk_03003410.unk0 > AUTO_ADVANCE_FRAMES)) && (gUnk_03003410.unk0 != 0)) {
        gUnk_03005220.hearts = 3;
        m4aMPlayAllStop();
        VBlankIntrWait();
        if (gUnk_03004C20.world == 6) {
            if ((gUnk_03004C20.level == 1) || (gUnk_03004C20.level == 3)) {
                REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_CLR | WININ_WIN1_BG0 | WININ_WIN1_CLR;
            }
        }
        REG_WINOUT = WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR;
        gUnk_03005210 = 0x100;
        m4aMPlayVolumeControl(&gMPlayInfo_0, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_1, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_2, 0xFF, gUnk_03005210);
        m4aMPlayVolumeControl(&gMPlayInfo_3, 0xFF, gUnk_03005210);
        gCallbackQueue.current[1] = TransitionFadeOutFull;
        gBlendValue = gMosaicSize = 0;
        return;
    }
    gUnk_03003410.unk0++;
}
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutFull);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionReturnToWorldMap);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutMusicAndReset);
/**
 * TransitionClearAndRestart: per-frame fade-in step (every other frame). Brightens
 * all layers via BLDCNT until the blend value reaches max, then resets BG2
 * affine/blend state, disables the HBlank interrupt, clears video state, and
 * queues InitLevelBG / ResetVideoRegisters to rebuild the scene.
 */
void TransitionClearAndRestart(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        gUnk_030034E4 = 0;
        gBg2XMag = gBg2YMag = 0x100;
        gBg2Alpha = 0;

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;

        gUnk_03004658[0xC] = 0;
        gUnk_03004C20.sceneFrameCounter = -1;
        ClearVideoState();
        gUnk_03003410.unk9 = 0;
        gUnk_03003410.unkA = 0;
        gCallbackQueue.next[0] = InitLevelBG;
        gUnk_03003410.unk8 = 1;
        gCallbackQueue.next[1] = ResetVideoRegisters;
        gCallbackQueue.next[2] = NULL + 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 3;
        gUnk_03004C20.sceneFrameCounter = -1;
    } else {
        gMosaicSize += 1;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeInRestoreWindows);
/**
 * TransitionToGameplayScreen: kleod TransitionToGameplayScreen.
 */
void TransitionToGameplayScreen(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_LIGHTEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        gUnk_030034E4 = 0;

        gBg2XMag = gBg2YMag = 0x100;
        gBg2Alpha = 0;

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;

        gUnk_03004658[0xC] = 0;
        gUnk_03004C20.sceneFrameCounter = -1;
        InitOamEntries();
        gCallbackQueue.next[0] = ReadKeyInput;
        gCallbackQueue.next[1] = UpdateAllEntities;
        gCallbackQueue.next[2] = TransitionFadeInRestoreWindows;
        gCallbackQueue.next[3] = VBlankCallback_Gameplay;
        gCallbackQueue.next[4] = NULL + 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 5;
        gUnk_03004C20.sceneFrameCounter = -1;
    } else {
        gMosaicSize += 1;
    }
}
/**
 * TransitionSoftReset: fades to black then triggers soft reset after 16 frames.
 */
void TransitionSoftReset(void) {
    u32 *sceneCtrl;

    gPauseFlag = 1;

    sceneCtrl = (u32 *)gControlBlock;
    if (sceneCtrl[1] & 1)
        return;

    REG_BLDCNT = 0xBF;

    gFrameCounter += 1;
    if (gFrameCounter == 16) {
        SoftResetRom(0xFF);
        return;
    }

    gMosaicSize += 1;
}
/**
 * TransitionSelfRemoveFadeIn: kleod TransitionSelfRemoveFadeIn.
 */
void TransitionSelfRemoveFadeIn(void) {
    u32 removed;
    u32 i;

    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue -= 1;
    if (gBlendValue == 0) {
        // remove TransitionFadeInRestoreWindows from callback queue
        removed = FALSE;
        for (i = 0; i < (gCallbackQueue.currentCount - 1); i++) {
            if ((gCallbackQueue.current[i] == TransitionSelfRemoveFadeIn) || (removed == TRUE)) {
                gCallbackQueue.next[i] = gCallbackQueue.current[i + 1];
                removed = TRUE;
            } else {
                gCallbackQueue.next[i] = gCallbackQueue.current[i];
            }
        }
        if (removed == TRUE) {
            gCallbackQueue.nextCount = gCallbackQueue.currentCount - 1;
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        }

        gUnk_030034E4 = 0;
    } else {
        gMosaicSize -= 1;
    }
}
/**
 * TransitionToSaveScreen: kleod TransitionToSaveScreen.
 */
void TransitionToSaveScreen(void) {
    gUnk_030034E4 = 1;
    if ((gUnk_03004C20.globalFrameCounter % 2) != 0) {
        return;
    }

    REG_BLDCNT = BLDCNT_EFFECT_DARKEN | BLDCNT_TGT1_ALL;

    gBlendValue += 1;
    if (gBlendValue == BLEND_MAX) {
        REG_IE &= ~INTR_FLAG_VBLANK;
        REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
        m4aSoundVSyncOff();

        m4aMPlayAllStop();
        gUnk_03005284->unk1 = 6;
        SaveGameWithVerify(0, 7);
        SaveGameWithVerify(1, 0);
        gUnk_030034E4 = 0;
        ClearVideoState();
        FreeAllDecompBuffers();
        gUnk_03004C20.sceneFrameCounter = -1;

        gBg2XMag = gBg2YMag = 0x100;
        gBg2Alpha = 0;

        REG_IE &= ~INTR_FLAG_HBLANK;
        REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;

        gUnk_03004658[0xC] = 0;
        gCallbackQueue.next[0] = ReadKeyInput;
        gCallbackQueue.next[1] = InitVideoAndBG;
        gCallbackQueue.next[2] = TransitionSelfRemoveFadeIn;
        gCallbackQueue.next[3] = VBlankCallback_MinimalHW;
        gCallbackQueue.next[4] = NULL + 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = 5;

        REG_IE |= INTR_FLAG_VBLANK;
        REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
        m4aSoundVSyncOn();
    } else {
        gMosaicSize += 1;
    }
}
/**
 * SetPaletteAnimEntry: (re)starts the palette/sprite animation for entry arg0
 * with animation id arg1, resetting its timer to 1 and frame to 0xFF. Entries
 * above 8 are remapped relative to the current dynamic-entry base (gUnk_030007C4).
 */
void SetPaletteAnimEntry(s32 arg0, u8 arg1) {
    if (arg0 > 8) {
        arg0 = arg0 + (9 - gUnk_030007C4);
    }

    gEntityAnimationInfo[arg0].state = arg1;
    gEntityAnimationInfo[arg0].timer = 1;
    gEntityAnimationInfo[arg0].frame = 0xFF;
}
/**
 * UpdatePaletteAnimations: per-frame driver for all 0x2D palette/sprite animation
 * slots. Decrements each active slot's timer, advances its frame, handles the
 * end/loop/branch sentinels (-1 loop, -2 despawn, -3 hold), DMAs the new frame's
 * palette into its destination, and mirrors the frame's flip flags onto the owner
 * entity in gEntityInfo.
 */
void UpdatePaletteAnimations(void) {
    vu32 sp0;
    struct Unk_03005294_03005418 *var_r5;
    struct Unk_03005294_03005418_0 **temp_r7;
    struct Unk_03005294_03005418_0 *var_r4;

    for (sp0 = 0; sp0 < 0x2D; sp0++) {
        if (sp0 < 9) {
            var_r5 = &gUnk_03005418[sp0];
        } else {
            var_r5 = &gUnk_03005294[sp0] - 9;
        }
        if (var_r5->unk0 == NULL) {
            break;
        }
        if (var_r5->unk0 == NULL + 1) {
            continue;
        }

        if (gEntityAnimationInfo[sp0].timer == 0xFF) {
            continue;
        }

        if (--gEntityAnimationInfo[sp0].timer != 0) {
            continue;
        }

        temp_r7 = var_r5->unk0;
        var_r4 = temp_r7[gEntityAnimationInfo[sp0].state];
        if (var_r4[++gEntityAnimationInfo[sp0].frame].src == -1) {
            gEntityAnimationInfo[sp0].frame = 0;
        } else if (var_r4[gEntityAnimationInfo[sp0].frame].src == -2) {
            gEntityAnimationInfo[sp0].timer |= 0xFF;
            gEntityInfo[var_r5->unkA].unk10 = 0;
            gEntityInfo[var_r5->unkA].unkF = 0x1C;
            gEntityInfo[var_r5->unkA].unk8.split.unk8 = 0;
            continue;
        } else if (var_r4[gEntityAnimationInfo[sp0].frame].src > 9999) {
            if (var_r4[gEntityAnimationInfo[sp0].frame].src == -3) {
                gEntityAnimationInfo[sp0].timer |= 0xFF;
                continue;
            }
        } else {
            gEntityAnimationInfo[sp0].state = var_r4[gEntityAnimationInfo[sp0].frame].src;
            gEntityAnimationInfo[sp0].frame = 0;
            var_r4 = temp_r7[gEntityAnimationInfo[sp0].state];
        }

        gEntityAnimationInfo[sp0].timer = var_r4[gEntityAnimationInfo[sp0].frame].unk4;
        DmaCopy16(3, var_r4[gEntityAnimationInfo[sp0].frame].src, var_r5->dest, var_r5->size);
        gEntityInfo[var_r5->unkA].unkB_0 = var_r4[gEntityAnimationInfo[sp0].frame].unk5_0;
        gEntityInfo[var_r5->unkA].unkB_4 = var_r4[gEntityAnimationInfo[sp0].frame].unk5_4;
    }
}
/**
 * CopyBGScrollTiles: ported from kleod CopyBGScrollTiles.
 */
void CopyBGScrollTiles(void) {
    u32 var_r5;
    u32 var_r6;

    for (var_r6 = 0; var_r6 < 3; var_r6++) {
        if (var_r6 < gUnk_03005220.hearts) {
            var_r5 = 0;
        } else {
            var_r5 = 2;
        }
        gBgTilemapBufs[0][(var_r6 * 2) + 0x241] = gBgTilemapBufs[0][(var_r6 * 2) + ((var_r5 + 0x14) << 5)];
        gBgTilemapBufs[0][(var_r6 * 2) + 0x242] = gBgTilemapBufs[0][(var_r6 * 2 + 1) + ((var_r5 + 0x14) << 5)];
        gBgTilemapBufs[0][(var_r6 * 2) + 0x261] = gBgTilemapBufs[0][(var_r6 * 2) + ((var_r5 + 0x15) << 5)];
        gBgTilemapBufs[0][(var_r6 * 2) + 0x262] = gBgTilemapBufs[0][(var_r6 * 2 + 1) + ((var_r5 + 0x15) << 5)];
    }
}
/**
 * UpdateHUDCounterDisplay: ported from kleod UpdateHUDCounterDisplay.
 */
s32 UpdateHUDCounterDisplay(void) {
    s32 var_r5;
    s32 var_sb;

    var_sb = 0;
    if ((gUnk_03004C20.unkA == 1) || (gUnk_03004C20.level == 6)) {
        var_r5 = 0x64;
    } else {
        var_r5 = 0x1E;
    }
    if (var_r5 == gUnk_03005220.dreamStones) {
        var_sb = 1;
    }

    if ((gUnk_03004C20.unkA == 1) || (gUnk_03004C20.level == 6)) {
        var_r5 = 1;
        if (gUnk_03005220.dreamStones > 0x63) {
            gBgTilemapBufs[0][0x252] += 0;
            gBgTilemapBufs[0][0x252] = gBgTilemapBufs[0][0x293];
            gBgTilemapBufs[0][0x272] += 0;
            gBgTilemapBufs[0][0x272] = gBgTilemapBufs[0][0x2B3];
        }
    } else {
        var_r5 = 0;
    }

    if (gUnk_03005220.dreamStones > 9) {
        gBgTilemapBufs[0][0x254 - var_r5] = gBgTilemapBufs[0][(gUnk_03005220.dreamStones / 10) + 0x292];
        gBgTilemapBufs[0][0x274 - var_r5] = gBgTilemapBufs[0][(gUnk_03005220.dreamStones / 10) + 0x2B2];
    }

    gBgTilemapBufs[0][0x255 - var_r5] = gBgTilemapBufs[0][(gUnk_03005220.dreamStones % 10) + 0x292];
    gBgTilemapBufs[0][0x275 - var_r5] = gBgTilemapBufs[0][(gUnk_03005220.dreamStones % 10) + 0x2B2];
    return var_sb;
}
/**
 * UpdateHUDCollectibleCount: ported from kleod UpdateHUDCollectibleCount.
 */
void UpdateHUDCollectibleCount(void) {
    if ((u8)gUnk_03005220.lives > 9) {
        gBgTilemapBufs[0][0x25B] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives / 10) + 0x292];
        gBgTilemapBufs[0][0x27B] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives / 10) + 0x2B2];
    } else if ((u8)gUnk_03005220.lives == 9) {
        gBgTilemapBufs[0][0x25B] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives / 10) + 0x25E];
        gBgTilemapBufs[0][0x27B] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives / 10) + 0x27E];
    }
    gBgTilemapBufs[0][0x25C] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives % 10) + 0x292];
    gBgTilemapBufs[0][0x27C] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives % 10) + 0x2B2];
}
/**
 * UpdateHUDCollectibleCountAlt: ported from kleod UpdateHUDCollectibleCountAlt.
 */
void UpdateHUDCollectibleCountAlt(void) {
    if ((u8)gUnk_03005220.lives > 9) {
        gBgTilemapBufs[0][0x25B] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives / 10) + 0x293];
        gBgTilemapBufs[0][0x27B] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives / 10) + 0x2B3];
    }
    gBgTilemapBufs[0][0x25C] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives % 10) + 0x293];
    gBgTilemapBufs[0][0x27C] = gBgTilemapBufs[0][((u8)gUnk_03005220.lives % 10) + 0x2B3];
}
INCLUDE_ASM("asm/nonmatchings/code_1", UpdateHUDTimerAndLives);
INCLUDE_ASM("asm/nonmatchings/code_1", IntroScrollAnimation);
INCLUDE_ASM("asm/nonmatchings/code_1", IntroSequenceUpdate);

/**
 * VBlankDMA_Level2: streams the OBJ palette and sprite tile graphics for
 * level 2 into OBJ palette RAM / OBJ VRAM via a fixed sequence of DMA copies.
 * Advances gVramWriteCursor (OBJ palette RAM) and gPaletteVramCursor (OBJ VRAM)
 * as each block is transferred.
 */
/**
 * VBlankDMA_Level1: streams level-1 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level1).
 */
void VBlankDMA_Level1(void) {
    DmaCopy16Wait(3, &gUnk_08077E68, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805D9E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    SetPaletteAnimEntry(0xD, 0);

    DmaCopy16Wait(3, &gUnk_08077E88, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805DBE8, gPaletteVramCursor, 0x300);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x300;

    DmaCopy16Wait(3, &gUnk_08077EA8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805DEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805E0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077EC8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805E2E8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_0805E6E8, gPaletteVramCursor, 0x400);
    gPaletteVramCursor += 0x400;

    DmaCopy16Wait(3, &gUnk_08077EE8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077F08, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02000904, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, (void *)0x02001104, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, (void *)0x02001904, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02001B04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02001D04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077F28, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002704, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02002904, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02002B04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02002D04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077F48, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002104, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02002304, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x02002504, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077F68, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001F04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
}
void VBlankDMA_Level2(void) {
    DmaCopy16Wait(3, &gUnk_08077EE8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077F88, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02000904, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08077FA8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001104, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08077FC8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001904, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077FE8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001B04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078008, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001D04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078028, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001F04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
}
/**
 * VBlankDMA_Level3: streams level-3 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level3).
 */
void VBlankDMA_Level3(void) {
    DmaCopy16Wait(3, &gUnk_08077EE8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078048, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02000904, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078068, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001104, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    SetPaletteAnimEntry(0x16, 0);

    DmaCopy16Wait(3, &gUnk_08078088, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001D04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080780A8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001F04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080780C8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001904, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080780E8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001704, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078108, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001B04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
}
/**
 * VBlankDMA_Level4: per-level OBJ palette/sprite tile DMA loader (kleod VBlankDMA_Level4).
 */
void VBlankDMA_Level4(void) {
    DmaCopy16Wait(3, &gUnk_08077EE8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078128, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02000904, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    SetPaletteAnimEntry(0x15, 0);

    DmaCopy16Wait(3, &gUnk_08078148, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002304, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078168, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002104, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078188, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002504, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080781A8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002704, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080781C8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002904, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080781E8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002B04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078208, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002D04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078228, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02002F04, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
}
/**
 * VBlankDMA_Level4Extra: per-level OBJ palette/sprite tile DMA loader (kleod VBlankDMA_Level4Extra).
 */
void VBlankDMA_Level4Extra(void) {
    DmaCopy16Wait(3, &gUnk_08077EE8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EAE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078248, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02004704, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    SetPaletteAnimEntry(0x15, 0);

    DmaCopy16Wait(3, &gUnk_08078268, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02001104, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    SetPaletteAnimEntry(0x16, 0);

    DmaCopy16Wait(3, &gUnk_08078288, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02000904, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_080782A8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02004104, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080782C8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02004304, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080782E8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x02004504, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
}
/**
 * VBlankDMA_LevelNoop: no-op entry in the per-level VBlank DMA loader table (kleod VBlankDMA_LevelNoop).
 */
void VBlankDMA_LevelNoop(void) {
    return;
}
/**
 * VBlankDMA_Level5: streams level-5 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level5).
 */
void VBlankDMA_Level5(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level6: streams level-6 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level6).
 */
void VBlankDMA_Level6(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level7: streams level-7 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level7).
 */
void VBlankDMA_Level7(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level8: streams level-8 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level8).
 */
void VBlankDMA_Level8(void) {
    DmaCopy16Wait(3, &gUnk_0805FA08, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078428, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FB08, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level9: streams level-9 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod sub_08028E8C).
 */
void VBlankDMA_Level9(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level10: streams level-10 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level10).
 */
void VBlankDMA_Level10(void) {
    DmaCopy16Wait(3, &gUnk_08078468, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FE08, gPaletteVramCursor, 0x800);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08060708, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level11: streams level-11 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level11).
 */
void VBlankDMA_Level11(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078488, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level12: streams level-12 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod sub_08029EAC).
 */
void VBlankDMA_Level12(void) {
    DmaCopy16Wait(3, &gUnk_080784A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060A08, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080784C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060A88, gPaletteVramCursor, 0x600);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x600;

    DmaCopy16Wait(3, &gUnk_080784E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061088, gPaletteVramCursor, 0x800);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078508, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    SetPaletteAnimEntry(0x16, 0);

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    SetPaletteAnimEntry(0x17, 0);
    SetPaletteAnimEntry(0x18, 0);

    DmaCopy16Wait(3, &gUnk_08078528, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk60, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk60, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, &gUnk_08061888, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078548, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061988, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08061A08, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
}
/**
 * VBlankDMA_Level13: streams level-13 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level13).
 */
void VBlankDMA_Level13(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08060708, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061C28, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level14: streams level-14 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level14).
 */
void VBlankDMA_Level14(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078588, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061C28, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level15: streams level-15 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level15).
 */
void VBlankDMA_Level15(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078588, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080785A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062248, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level16: streams level-16 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level16).
 */
void VBlankDMA_Level16(void) {
    DmaCopy16Wait(3, &gUnk_0805FA08, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078428, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FB08, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level17: streams level-17 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level17).
 */
void VBlankDMA_Level17(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078488, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062348, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080785C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080623C8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08060708, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080627C8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level18: streams level-18 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod sub_0802C1F8).
 */
void VBlankDMA_Level18(void) {
    DmaCopy16Wait(3, &gUnk_08078468, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FE08, gPaletteVramCursor, 0x800);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078488, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062248, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level19: streams level-19 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod sub_0802C8B0).
 */
void VBlankDMA_Level19(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080785E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062348, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080785C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080623C8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08060708, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078608, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062248, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062848, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080627C8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level20: streams level-20 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod sub_0802D028).
 */
void VBlankDMA_Level20(void) {
    DmaCopy16Wait(3, &gUnk_080784A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060A08, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080784C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060A88, gPaletteVramCursor, 0x600);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x600;

    DmaCopy16Wait(3, &gUnk_080784E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061088, gPaletteVramCursor, 0x800);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078628, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078648, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk60, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk60, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078668, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk78, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk90[1], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078688, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level21: streams level-21 OBJ palettes and sprite tiles (kleod VBlankDMA_Level21).
 */
void VBlankDMA_Level21(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level22: streams level-22 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level22).
 */
void VBlankDMA_Level22(void) {

    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062EE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062248, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_080630E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level23: streams level-23 OBJ palettes and sprite tiles into OBJ palette
 * RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod sub_0802E374).
 */
void VBlankDMA_Level23(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078488, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062EE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063168, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080627C8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078368, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level24: streams level-24 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level24).
 */
void VBlankDMA_Level24(void) {
    DmaCopy16Wait(3, &gUnk_0805FA08, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078428, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FB08, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level25: streams level-25 OBJ palettes and sprite tiles (kleod VBlankDMA_Level25).
 */
void VBlankDMA_Level25(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063368, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080786E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080633E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062348, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080785C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080623C8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080635E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080635E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080635E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_080630E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level26: streams level-26 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level26).
 */
void VBlankDMA_Level26(void) {
    DmaCopy16Wait(3, &gUnk_08078468, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FE08, gPaletteVramCursor, 0x800);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078588, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08060708, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08062248, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * VBlankDMA_Level27: streams level-27 OBJ palettes and sprite tiles into OBJ
 * palette RAM / OBJ VRAM via a fixed sequence of DMA copies (kleod VBlankDMA_Level27).
 */
void VBlankDMA_Level27(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062EE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063668, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_080627C8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080786E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080633E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062348, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080785C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080623C8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;

    DmaCopy16Wait(3, &gUnk_08078708, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061D28, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D48, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D68, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061D88, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08061DA8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F2E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078388, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F368, gPaletteVramCursor, 0x20);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_0805F388, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
