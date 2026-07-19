#include "global.h"
#include "gba.h"
#include "globals.h"
#include "include_asm.h"
#include "structs/variables.h"

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
void SetPaletteAnimEntry(u32, u8);
void CopyBGScrollTiles(void);
void m4aSongNumStart(u16);

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
INCLUDE_ASM("asm/nonmatchings/code_1", sub_080158AC);
INCLUDE_ASM("asm/nonmatchings/code_1", EntitySpawnFromLevelData);
INCLUDE_ASM("asm/nonmatchings/code_1", sub_0801AF28);
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
extern void m4aSongNumStart(u16 n);
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
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutDisableIRQ);
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
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionGameplayInit);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutWithMusic);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionWorldMapFadeOut);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionToSceneSelect);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionToTitleScreen);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionGameOver);
INCLUDE_ASM("asm/nonmatchings/code_1", GameplayFrameInit);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutFull);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionReturnToWorldMap);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeOutMusicAndReset);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionClearAndRestart);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionFadeInRestoreWindows);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionToGameplayScreen);
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

    gBldyFadeLevel += 1;
}
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionSelfRemoveFadeIn);
INCLUDE_ASM("asm/nonmatchings/code_1", TransitionToSaveScreen);
INCLUDE_ASM("asm/nonmatchings/code_1", SetPaletteAnimEntry);
INCLUDE_ASM("asm/nonmatchings/code_1", UpdatePaletteAnimations);
INCLUDE_ASM("asm/nonmatchings/code_1", CopyBGScrollTiles);
INCLUDE_ASM("asm/nonmatchings/code_1", UpdateHUDCounterDisplay);
INCLUDE_ASM("asm/nonmatchings/code_1", UpdateHUDCollectibleCount);
INCLUDE_ASM("asm/nonmatchings/code_1", UpdateHUDCollectibleCountAlt);
INCLUDE_ASM("asm/nonmatchings/code_1", UpdateHUDTimerAndLives);
INCLUDE_ASM("asm/nonmatchings/code_1", IntroScrollAnimation);
INCLUDE_ASM("asm/nonmatchings/code_1", IntroSequenceUpdate);
/* Per-level animated-tile pointer table (kleod-canonical), indexed
 * [world-1][level]; the ->unk3C etc members point at frame data streamed
 * into OBJ VRAM by some level loaders. */
struct Unk_08189A24 {
    u8 pad0[0x3C - 0x0];
    void ***unk3C;
    u8 pad40[0x60 - 0x40];
    void ***unk60;
    u8 pad64[0x6C - 0x64];
    void ***unk6C;
    u8 pad70[0x78 - 0x70];
    void ***unk78;
    u8 pad7C[0x90 - 0x7C];
    void ***unk90;
};
extern struct Unk_08189A24 *gUnk_08189A24[6][9];
/* Per-level sprite tile graphics (0x200 bytes each) and OBJ palettes (0x20
 * bytes each) in ROM, streamed by the VBlankDMA_Level* loaders. */
extern u8 gUnk_0805EAE8[0x200];
extern u8 gUnk_08077EE8[0x20];
extern u8 gUnk_08077F88[0x20];
extern u8 gUnk_08077FA8[0x20];
extern u8 gUnk_08077FC8[0x20];
extern u8 gUnk_08077FE8[0x20];
extern u8 gUnk_08078008[0x20];
extern u8 gUnk_08078028[0x20];
extern u8 gUnk_08060808[0x200];
extern u8 gUnk_08060A08[0x80];
extern u8 gUnk_08060A88[0x600];
extern u8 gUnk_08061088[0x800];
extern u8 gUnk_08061888[0x100];
extern u8 gUnk_08061988[0x80];
extern u8 gUnk_08061A08[0x20];
extern u8 gUnk_08062848[0x80];
extern u8 gUnk_08062CE8[0x200];
extern u8 gUnk_08063168[0x200];
extern u8 gUnk_08078488[0x20];
extern u8 gUnk_080784A8[0x20];
extern u8 gUnk_080784C8[0x20];
extern u8 gUnk_080784E8[0x20];
extern u8 gUnk_08078508[0x20];
extern u8 gUnk_08078528[0x20];
extern u8 gUnk_08078548[0x20];
extern u8 gUnk_080785E8[0x20];
extern u8 gUnk_08078608[0x20];
extern u8 gUnk_08078628[0x20];
extern u8 gUnk_08078648[0x20];
extern u8 gUnk_08078668[0x20];
extern u8 gUnk_08078688[0x20];
extern u8 gUnk_0805D9E8[0x200];
extern u8 gUnk_0805DBE8[0x300];
extern u8 gUnk_0805DEE8[0x200];
extern u8 gUnk_0805E0E8[0x200];
extern u8 gUnk_0805E2E8[0x400];
extern u8 gUnk_0805E6E8[0x400];
extern u8 gUnk_08077E68[0x20];
extern u8 gUnk_08077E88[0x20];
extern u8 gUnk_08077EA8[0x20];
extern u8 gUnk_08077EC8[0x20];
extern u8 gUnk_08077F08[0x20];
extern u8 gUnk_08077F28[0x20];
extern u8 gUnk_08077F48[0x20];
extern u8 gUnk_08077F68[0x20];
extern u8 gUnk_08078128[0x20];
extern u8 gUnk_08078148[0x20];
extern u8 gUnk_08078168[0x20];
extern u8 gUnk_08078188[0x20];
extern u8 gUnk_080781A8[0x20];
extern u8 gUnk_080781C8[0x20];
extern u8 gUnk_080781E8[0x20];
extern u8 gUnk_08078208[0x20];
extern u8 gUnk_08078228[0x20];
extern u8 gUnk_08078248[0x20];
extern u8 gUnk_08078268[0x20];
extern u8 gUnk_08078288[0x20];
extern u8 gUnk_080782A8[0x20];
extern u8 gUnk_080782C8[0x20];
extern u8 gUnk_080782E8[0x20];
extern u8 gUnk_0805ECE8[0x200];
extern u8 gUnk_0805EEE8[0x200];
extern u8 gUnk_0805F0E8[0x200];
extern u8 gUnk_0805F2E8[0x80];
extern u8 gUnk_0805F368[0x20];
extern u8 gUnk_0805F388[0x80];
extern u8 gUnk_0805F408[0x80];
extern u8 gUnk_0805F488[0x80];
extern u8 gUnk_0805F508[0x200];
extern u8 gUnk_0805F708[0x80];
extern u8 gUnk_0805F788[0x80];
extern u8 gUnk_0805F808[0x200];
extern u8 gUnk_0805FA08[0x100];
extern u8 gUnk_0805FB08[0x100];
extern u8 gUnk_0805FC08[0x200];
extern u8 gUnk_0805FE08[0x800];
extern u8 gUnk_08060608[0x100];
extern u8 gUnk_08060708[0x100];
extern u8 gUnk_08061A28[0x200];
extern u8 gUnk_08061C28[0x100];
extern u8 gUnk_08061D28[0x20];
extern u8 gUnk_08061D48[0x20];
extern u8 gUnk_08061D68[0x20];
extern u8 gUnk_08061D88[0x20];
extern u8 gUnk_08061DA8[0x20];
extern u8 gUnk_08061DC8[0x200];
extern u8 gUnk_08061FC8[0x80];
extern u8 gUnk_08062048[0x100];
extern u8 gUnk_08062148[0x100];
extern u8 gUnk_08062248[0x100];
extern u8 gUnk_08062348[0x80];
extern u8 gUnk_080623C8[0x400];
extern u8 gUnk_080627C8[0x80];
extern u8 gUnk_080628C8[0x200];
extern u8 gUnk_08062AC8[0x20];
extern u8 gUnk_08062AE8[0x200];
extern u8 gUnk_08062EE8[0x200];
extern u8 gUnk_080630E8[0x80];
extern u8 gUnk_08063368[0x80];
extern u8 gUnk_080633E8[0x200];
extern u8 gUnk_080635E8[0x80];
extern u8 gUnk_08063668[0x200];
extern u8 gUnk_08078048[0x20];
extern u8 gUnk_08078068[0x20];
extern u8 gUnk_08078088[0x20];
extern u8 gUnk_080780A8[0x20];
extern u8 gUnk_080780C8[0x20];
extern u8 gUnk_080780E8[0x20];
extern u8 gUnk_08078108[0x20];
extern u8 gUnk_08078308[0x20];
extern u8 gUnk_08078328[0x20];
extern u8 gUnk_08078348[0x20];
extern u8 gUnk_08078368[0x20];
extern u8 gUnk_08078388[0x20];
extern u8 gUnk_080783A8[0x20];
extern u8 gUnk_080783C8[0x20];
extern u8 gUnk_080783E8[0x20];
extern u8 gUnk_08078408[0x20];
extern u8 gUnk_08078428[0x20];
extern u8 gUnk_08078448[0x20];
extern u8 gUnk_08078468[0x20];
extern u8 gUnk_08078568[0x20];
extern u8 gUnk_08078588[0x20];
extern u8 gUnk_080785A8[0x20];
extern u8 gUnk_080785C8[0x20];
extern u8 gUnk_080786A8[0x20];
extern u8 gUnk_080786C8[0x20];
extern u8 gUnk_080786E8[0x20];
extern u8 gUnk_08078708[0x20];

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
