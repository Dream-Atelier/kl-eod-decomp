#include "global.h"
#include "gba.h"
#include "globals.h"
#include "include_asm.h"

void ReadKeyInput(void);
void InitOamEntries(void);
void UpdateWorldMapScene(void);
void TransitionWorldMapFadeOut(void);
void VBlankCallback_Gameplay(void);
void SoftReset(u32);

INCLUDE_ASM("asm/nonmatchings/code_1", EntityUpdateDispatch);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerMainUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerMovementPhysics);
INCLUDE_ASM("asm/nonmatchings/code_1", CheckTileCollisionVertical);
INCLUDE_ASM("asm/nonmatchings/code_1", CheckTileCollisionSloped);
INCLUDE_ASM("asm/nonmatchings/code_1", ApplyEntityTileMovement);
INCLUDE_ASM("asm/nonmatchings/code_1", InitScrollState);
INCLUDE_ASM("asm/nonmatchings/code_1", ResetEntityScrollState);
INCLUDE_ASM("asm/nonmatchings/code_1", PlayerRespawnOrDeath);
INCLUDE_ASM("asm/nonmatchings/code_1", EntityBehaviorMasterUpdate);
INCLUDE_ASM("asm/nonmatchings/code_1", FUN_080158ac);
INCLUDE_ASM("asm/nonmatchings/code_1", EntitySpawnFromLevelData);
INCLUDE_ASM("asm/nonmatchings/code_1", FUN_0801af28);
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
extern u8 FUN_08051a0c(u8 a, u8 b);
extern u8 FUN_08051a84(u8 a, u8 b);
extern void m4aSongNumStart(u16 n);
extern void LoadSpriteFrame(u8 frame, u8 tilesetIdx);

/**
 * EntityDeathAnimation: multi-frame death/destruction animation.
 *
 * Each frame, decrements a timer. When the timer underflows (wraps to 0xFF),
 * resets it to 25 frames and advances the death phase:
 *   phase > 5: spawns a visual effect entity (type = phase + 0x0C)
 *   phase > 0: plays SFX 0x56, updates sprite frames for the dying
 *              and paired entities via FUN_08051a0c / FUN_08051a84
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
    register int mask asm("r4");
    u8 pairedSlot;
    int val;
    EntityDeathStruct *entities = (EntityDeathStruct *)gEntityArray;
    EntityDeathStruct *entity;
    register int ents asm("r3") = (int)gEntityArray;
    EntityDeathStruct *ent;
    int tmp;

    entity = &entities[slot];
    entity->timer = entity->timer - 1;
    mask = 0xFF;
    if ((u8)entity->timer == 0xFF) {
        entity->timer = 25;
        phase = entity->phase;
        if (phase <= 5) {
            SpawnEntityAtPosition(entities[slot - *gEntityDeathState].x, entities[slot - *gEntityDeathState].y, (u8)(phase + 0x0C), 0);
        }
        val = entity->phase - 1;
        entity->phase = val;
        if ((val & mask) == 0) {
            SpawnEntityAtPosition(entities[slot - *gEntityDeathState].x, entities[slot - *gEntityDeathState].y, 2,
                                  (u8)(slot - *gEntityDeathState));
        } else {
            m4aSongNumStart(0x56);
            if (entity->phase > 9) {
                entities[slot + *gEntityDeathState].typeId = 0;
                pairedSlot = (u8)(slot + *gEntityDeathState);
                LoadSpriteFrame(pairedSlot, FUN_08051a0c(entity->phase, 0x0A));
            }
            LoadSpriteFrame(slot, FUN_08051a84(entity->phase, 0x0A));
            if (entity->phase == 9) {
                entities[slot + *gEntityDeathState].typeId = 0x1C; /* inactive */
                entities[slot + *gEntityDeathState].subState = 0;
            }
        }
    }
    ents = (int)gEntityArray;
    tmp = slot * sizeof(EntityDeathStruct);
    ent = (EntityDeathStruct *)(tmp + ents);
    ent->x = ((EntityDeathStruct *)ents)[slot - *gEntityDeathState].x;
    ent->y = ((EntityDeathStruct *)ents)[slot - *gEntityDeathState].y - 0x20;
    if (ent->phase > 9) {
        ((EntityDeathStruct *)ents)[slot + *gEntityDeathState].x = ((EntityDeathStruct *)ents)[slot - *gEntityDeathState].x - 3;
        ((EntityDeathStruct *)ents)[slot + *gEntityDeathState].y = ((EntityDeathStruct *)ents)[slot - *gEntityDeathState].y - 0x20;
        ent->x = ent->x + 3;
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
INCLUDE_ASM("asm/nonmatchings/code_1", EntityItemDrop);
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

    sceneCtrl = (u32 *)0x03004C20;
    isActive = sceneCtrl[1] & 1;
    if (isActive != 0)
        return;

    REG_BLDCNT = 0xFF;

    gFrameCounter += 1;
    if (gFrameCounter != 16)
        return;

    InitOamEntries();
    sceneCtrl[0] = (u32)-1;

    callbackState = (u32 *)0x03003510;
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

    sceneCtrl = (u32 *)0x03004C20;
    if (sceneCtrl[1] & 1)
        return;

    REG_BLDCNT = 0xBF;

    gFrameCounter += 1;
    if (gFrameCounter == 16) {
        SoftReset(0xFF);
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
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level1);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level2);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level3);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level4);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level5);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level6);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level7);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level8);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level9);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level10);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level11);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level12);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level13);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level14);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level15);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level16);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level17);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level18);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level19);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level20);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level21);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level22);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level23);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level24);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level25);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level26);
INCLUDE_ASM("asm/nonmatchings/code_1", VBlankDMA_Level27);
