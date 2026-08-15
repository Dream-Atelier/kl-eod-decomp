#include "global.h"
#include "globals.h"
#include "include_asm.h"
#include "structs/variables.h"
#include "data/trig.h"
extern struct EntityAnimationInfo gEntityAnimationInfo[];
extern void SetupBG3WindowOverlay(void);
extern void UpdateScrollPosition(void);
void SetPaletteAnimEntry(s32, u8);
extern s16 MultiplyQ8(s16, s16);
extern s16 ReciprocalQ8(s16);
extern u8 thunk_sub_080002A0();
extern u8 thunk_sub_080002D0();
extern void VBlankCallback_Dialog(void);
/* Cross-module function prototypes referenced by the decompiled functions below.
 * (ROM data symbols live in structs/variables.h.) */
void HBlankScrollUpdate(void);
s32 Abs(s32 n);
void IntroSequenceUpdate(void);
void TransitionReturnToWorldMap(void);
void TransitionClearAndRestart(void);
void TransitionSoftReset(void);
void TransitionToGameplayScreen(void);
void TransitionGameOver(void);
void TransitionFadeOutFull(void);
void UpdatePlayerFinalBoss(u8 arg0);
void UpdatePlayerAlternate(u8 arg0);
void UpdatePlayerMinigame(u8 arg0);
void VBlankHandler_ModeB(void);
void ReadKeyInput(void);
void UpdateVisionStarIcons(void);
u8 FindNextUnlockedVision(void);
void UpdateWorldMapNodeState(void);
void VBlankCallback_Gameplay(void);
void AnimatePaletteEffects(void);
void IntroScrollAnimation(void);

/* ── kleod code_08039D8C.c shared scaffolding ─────────────────────────────
 * Common globals, cross-module externs, and helper inlines for the logic
 * functions ported from kleod's code_08039D8C.c. */

extern struct BgDataPtrs gBgDataPtrs; /* 0x03004790 */
extern u8 gBlendValue; /* 0x03005498 */

/* Tile-collision query result (kleod CheckTileCollisionSloped). */
struct Unk_08014184 {
    u16 unk0;
    u8 unk2;
    u8 pad3[0x4 - 0x3];
};

/* Cross-module functions (defined in code_1). */
extern struct Unk_08014184 *CheckTileCollisionSloped(struct Unk_08014184 *, u16, u16, u8);
static inline struct Unk_08014184 Call_CheckTileCollisionSloped(u16 arg1, u16 arg2, u8 arg3) {
    struct Unk_08014184 sp;
    CheckTileCollisionSloped(&sp, arg1, arg2, arg3);
    return sp;
}
extern void PlayerRespawnOrDeath(s32);
extern void SpawnEntityAtPosition(u16, u16, u8, u8);

/* Forward decls (code_08039D8C.c cluster). */
void InitGameplayState(void);
void UpdateOamSortOrder(void);
void ProcessInputAndUpdateEntities(void);
void UpdateWorldMapInput(void);
u8 CheckWorldCompletion(u8 arg0);
void CopyWorldMapTiles(u8 arg0);
void SetWorldMapTilePalette(u8 arg0, u8 arg1);
void UpdateWorldMapNodeTile(u8 arg0);
void UpdateEntities(void);
void CountCollectedGems(void);
void UpdateWorldMapNodeAnim(void);
void GameplayMainLoop(void);
void InitLevelState(void);
void SpawnEntitiesForVision(u8 arg0);
void GetEntityLookupData(u8 arg0);
void ComputeScrollLimits(void);
void ApplyPlayerMovement(u8 arg0, struct Unk_0803D4AC arg1);
void UpdatePlayerNormal(u8 arg0);
void SetupEntitySpawnTable(u8 arg0);
void RollRandomLevelVariant(void);
void UpdatePlayerBoss(u8 arg0);
void ConfigureEntityBehavior(u8 arg0, u8 arg1, u8 arg2);
void TransitionLevelVariant(u8 arg0);
void UpdateLevelProgression(void);
void SetEntityVisibility(u8 arg0);
void UpdatePlayerSpecial(u8 arg0);

/**
 * LoadLevel_World1_Vision1: streams the tile and palette graphics data for World 1, Vision 1 from ROM into VRAM and palette RAM via
 * DMA (including the level-specific palette tables indexed through gUnk_08189A24), advancing the write cursors.
 */
void LoadLevel_World1_Vision1(void) {
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

    DmaCopy16Wait(3, &gUnk_08078728, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078348, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078748, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk6C, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, (void *)0x0200A984, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, (void *)0x0200A904, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078728, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x24 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078788, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x28 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x2C / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World1_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", sub_08030D3A);
/**
 * LoadLevel_World2_Vision1: streams the tile and palette graphics data for World 2, Vision 1 from ROM into VRAM and palette RAM via
 * DMA, advancing the gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_World2_Vision1(void) {
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

    DmaCopy16Wait(3, &gUnk_08078588, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078448, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805FC08, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080787A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08063868, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063A68, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08063868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063368, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080633E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

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
 * LoadLevel_World2_Vision2: streams the tile and palette graphics data for World 2, Vision 2 from ROM into VRAM and palette RAM via
 * DMA, advancing the gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_World2_Vision2(void) {
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
 * LoadLevel_World3_Vision1: ported from kleod sub_08031E7C.
 */
void LoadLevel_World3_Vision1(void) {
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

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062EE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063168, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
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
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080787C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08063AE8, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_080787E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08063BE8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08063FE8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08064068, gPaletteVramCursor, 0x400);
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078848, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
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
 * LoadLevel_World3_Vision2: streams the tile and palette graphics data for World 3, Vision 2 from ROM into VRAM and palette RAM via
 * DMA, advancing the gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_World3_Vision2(void) {
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

    DmaCopy16Wait(3, &gUnk_08078588, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08061DC8, gPaletteVramCursor, 0x200);
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
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060708, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
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
 * LoadLevel_World4_Vision1: ported from kleod sub_08032D3C.
 */
void LoadLevel_World4_Vision1(void) {
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
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078568, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061A28, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078868, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08061C28, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;

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
    DmaCopy16Wait(3, &gUnk_08063FE8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078888, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064068, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08063AE8, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08063BE8, gPaletteVramCursor, 0x400);
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08064468, gPaletteVramCursor, 0x400);
    gPaletteVramCursor += 0x400;

    DmaCopy16Wait(3, &gUnk_08078848, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
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
    DmaCopy16Wait(3, &gUnk_080630E8, gPaletteVramCursor, 0x80);
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

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * LoadLevel_World4_Vision2: streams the tile and palette graphics data for World 4, Vision 2 from ROM into VRAM and palette RAM via
 * DMA (including the level-specific palette tables indexed through gUnk_08189A24), advancing the write cursors.
 */
void LoadLevel_World4_Vision2(void) {
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

    DmaCopy16Wait(3, &gUnk_080788A8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    SetPaletteAnimEntry(0x12, 0);

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080788C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064C68, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064C68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064E68, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078848, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * LoadLevel_World5_Vision1: streams the tile and palette graphics data for World 5, Vision 1 from ROM into VRAM and palette RAM via
 * DMA, advancing the gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_World5_Vision1(void) {
    DmaCopy16Wait(3, &gUnk_08078308, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805ECE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080788E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065168, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065368, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060608, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08063368, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080633E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_080627C8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
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
 * LoadLevel_World5_Vision2: ported from kleod sub_08034078.
 */
void LoadLevel_World5_Vision2(void) {
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
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F488, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078488, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_080788E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065168, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065368, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063368, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080633E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062348, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080785C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080623C8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08063168, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078408, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F708, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805F788, gPaletteVramCursor, 0x80);
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
 * LoadLevel_World6_Vision1: streams the tile and palette graphics data for World 6, Vision 1 from ROM into VRAM and palette RAM via
 * DMA, advancing the gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_World6_Vision1(void) {
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

    DmaCopy16Wait(3, &gUnk_080788E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064F68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065168, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065368, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080786A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080628C8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_080635E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080635E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_080635E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_08078908, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062048, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08061FC8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08065568, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080783E8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F508, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062348, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    DmaCopy16Wait(3, &gUnk_080785C8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_080623C8, gPaletteVramCursor, 0x400);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, &gUnk_08062148, gPaletteVramCursor, 0x100);
    gPaletteVramCursor += 0x100;

    DmaCopy16Wait(3, &gUnk_08078848, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
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
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World6_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", sub_080356E6);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World7_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", sub_08036566);
/**
 * LoadLevel_World7_Vision2: streams the tile and palette graphics data for World 7, Vision 2 from ROM into VRAM and palette RAM via
 * DMA (including the level-specific palette tables indexed through gUnk_08189A24), advancing the write cursors.
 */
void LoadLevel_World7_Vision2(void) {
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

    DmaCopy16Wait(3, &gUnk_08078948, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x20 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    SetPaletteAnimEntry(0x17, 0);
    SetPaletteAnimEntry(0x18, 0);

    DmaCopy16Wait(3, &gUnk_08078968, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, **gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C, gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078988, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x38 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x44 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x44 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x4 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x8 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World8_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World8_Vision2);
/**
 * LoadLevel_World9_Vision1: streams the tile and palette graphics data for World 9, Vision 1 from ROM into VRAM and palette RAM via
 * DMA, advancing the gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_World9_Vision1(void) {
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
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805F0E8, gPaletteVramCursor, 0x200);
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

    DmaCopy16Wait(3, &gUnk_08078488, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08060808, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080789A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062CE8, gPaletteVramCursor, 0x200);
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
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08062AC8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078608, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_08062248, gPaletteVramCursor, 0x100);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x100;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064868, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08064A68, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_08062AE8, gPaletteVramCursor, 0x200);
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
 * LoadLevel_World9_Vision2: streams the tile and palette graphics data for World 9, Vision 2 from ROM into VRAM and palette RAM via
 * DMA (including the level-specific palette tables indexed through gUnk_08189A24), advancing the write cursors.
 */
void LoadLevel_World9_Vision2(void) {
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

    DmaCopy16Wait(3, &gUnk_080789C8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x60 / 4], gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_080789E8, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x8 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078328, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805EEE8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08078A28, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x4C / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x54 / 4], gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x54 / 4], gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x50 / 4], gPaletteVramCursor, 0x400);
    gPaletteVramCursor += 0x400;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x1C / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x20 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x24 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_08078A48, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08065768, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_08065788, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;
    DmaCopy16Wait(3, &gUnk_080657A8, gPaletteVramCursor, 0x20);
    gPaletteVramCursor += 0x20;

    DmaCopy16Wait(3, &gUnk_08078A68, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    DmaCopy16Wait(3, *gUnk_08189A24[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk3C[0x8 / 4], gPaletteVramCursor, 0x800);
    gPaletteVramCursor += 0x800;

    DmaCopy16Wait(3, &gUnk_080783A8, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805F408, gPaletteVramCursor, 0x80);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x80;
}
/**
 * LoadLevel_BossArena: streams the boss-arena tile and palette graphics data from ROM into VRAM and palette RAM via DMA, advancing the
 * gVramWriteCursor/gPaletteVramCursor write cursors.
 */
void LoadLevel_BossArena(void) {
    DmaCopy16Wait(3, &gUnk_08077E28, gVramWriteCursor, 0x20);
    gVramWriteCursor += 0x20;
    SetPaletteAnimEntry(0, 0);
    gEntityInfo[0].unk10 = 1;
    gPaletteVramCursor += 0x200;

    DmaCopy16Wait(3, &gUnk_08077E48, gVramWriteCursor, 0x20);
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gVramWriteCursor += 0x20;
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C6E8, gPaletteVramCursor, 0x200);
    gPaletteVramCursor += 0x200;
    DmaCopy16Wait(3, &gUnk_0805C8E8, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;
    DmaCopy16Wait(3, &gUnk_0805C968, gPaletteVramCursor, 0x80);
    gPaletteVramCursor += 0x80;

    if (gUnk_03004C20.level != 8) {
        if (gUnk_03003410.unkA == 0) {
            DmaCopy16Wait(3, &gUnk_0805C9E8, gPaletteVramCursor, 0x800);
            gPaletteVramCursor += 0x800;

            if (gUnk_03004C20.level != 0) {
                if (gCallbackQueue.next[4] == IntroScrollAnimation) {
                    gEntityInfo[0xB].unk10 = 1;
                } else {
                    gEntityInfo[0xB].unk10 = 0;
                }
            }
        } else {
            DmaCopy16Wait(3, &gUnk_080A5888, gPaletteVramCursor, 0x800);
            gPaletteVramCursor += 0x800;

            gEntityInfo[0xB].xPosScreen = 0x48;
            gEntityInfo[0xB].yPosScreen = 0x20;
            gEntityInfo[0xB].unk10 = 1;
        }
    } else {
        gEntityInfo[0xB].yPosScreen = 0x50;

        DmaCopy16Wait(3, &gUnk_080A4888, gPaletteVramCursor, 0x800);
        gPaletteVramCursor += 0x800;

        gEntityInfo[0xB].unk10 = 1;
    }

    if ((gUnk_03004C20.level - 1) >= 0 && (gUnk_03004C20.level - 1) <= 6) {
        DmaCopy16Wait(3, gUnk_0818B800[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1], gPaletteVramCursor, 0x800);
        gPaletteVramCursor += 0x800;

        if (gCallbackQueue.next[4] == IntroScrollAnimation) {
            gEntityInfo[0xC].unk10 = 1;
        } else {
            gEntityInfo[0xC].unk10 = 0;
        }
    } else {
        DmaCopy16Wait(3, &gUnk_0805D1E8, gPaletteVramCursor, 0x800);
        gPaletteVramCursor += 0x800;
    }

    gUnk_030034F4 = gVramWriteCursor;
    gUnk_030052AC = gPaletteVramCursor;
}
/**
 * InitGameplayState: sets up a level for play — saves the current display registers into gUnk_030051F0, allocates and decompresses the
 * BG3 tiles/tilemap, and installs the gameplay VBlank/main callbacks.
 */
void InitGameplayState(void) {
    s32 var_r4;
    s32 var_r4_2; // TODO: can be merged with var_r4
    s32 var_r5;
    s32 var_sl;

    if (gUnk_030034BC == 0) {
        var_sl = gUnk_03000800;
    } else {
        var_sl = 0;
    }

    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();
    m4aMPlayAllStop();

    if (gUnk_03003410.unk4 != 0) {
        if (gUnk_03004C20.level == 8) {
            gUnk_030034C0 = 3;
        } else if (gUnk_03004C20.level == 0) {
            gUnk_030034C0 = 1;
        } else if (gUnk_03004C20.world == 6) {
            gUnk_030034C0 = 2;
        } else {
            gUnk_030034C0 = 0;
        }
    }

    if (gUnk_030034C0 == 3) {
        gBgDataPtrs.pBufBg3Tiles = thunk_HeapAlloc(gUnk_082EAF8C & 0x7FFFFFFF, 0);
        gBgDataPtrs.pBufBg3Tilemap = thunk_HeapAlloc(gUnk_082EB488 & 0x7FFFFFFF, 0);
        Decompress(gBgDataPtrs.pBufBg3Tiles, &gUnk_082EAF8C);
        Decompress(gBgDataPtrs.pBufBg3Tilemap, &gUnk_082EB488);
    } else if (gUnk_030034C0 == 0) {
        gBgDataPtrs.pBufBg3Tiles = thunk_HeapAlloc(gUnk_082EB5B8 & 0x7FFFFFFF, 0);
        gBgDataPtrs.pBufBg3Tilemap = thunk_HeapAlloc(gUnk_082EBB20 & 0x7FFFFFFF, 0);
        Decompress(gBgDataPtrs.pBufBg3Tiles, &gUnk_082EB5B8);
        Decompress(gBgDataPtrs.pBufBg3Tilemap, &gUnk_082EBB20);
    } else if (gUnk_030034C0 == 2) {
        gBgDataPtrs.pBufBg3Tiles = thunk_HeapAlloc(gUnk_082EBC68 & 0x7FFFFFFF, 0);
        gBgDataPtrs.pBufBg3Tilemap = thunk_HeapAlloc(gUnk_082EC1A4 & 0x7FFFFFFF, 0);
        Decompress(gBgDataPtrs.pBufBg3Tiles, &gUnk_082EBC68);
        Decompress(gBgDataPtrs.pBufBg3Tilemap, &gUnk_082EC1A4);
    } else {
        gBgDataPtrs.pBufBg3Tiles = thunk_HeapAlloc(gUnk_082EC2E4 & 0x7FFFFFFF, 0);
        gBgDataPtrs.pBufBg3Tilemap = thunk_HeapAlloc(gUnk_082EC7C8 & 0x7FFFFFFF, 0);
        Decompress(gBgDataPtrs.pBufBg3Tiles, &gUnk_082EC2E4);
        Decompress(gBgDataPtrs.pBufBg3Tilemap, &gUnk_082EC7C8);
    }

    REG_DISPSTAT &= 0xFF;

    for (var_r4 = 0, var_r5 = 0; var_r4 <= 0x21B; var_r5++, var_r4++) {
        if (((var_r4 % 30) == 0) && (var_r4 != 0)) {
            var_r5 += 2;
        }
        gBgTilemapBufs[gUnk_030034BC][var_r5] = gBgDataPtrs.pBufBg3Tilemap[var_r4 + 2] + var_sl;
    }

    DmaCopy16(3, &gBgTilemapBufs[gUnk_030034BC], gBgInfo[gUnk_030034BC].pTilemap, 0x800);

    REG_DISPCNT &= ~DISPCNT_WIN1_ON;

    if (gUnk_03003410.unk4 != 0) {
        gUnk_030051F0.unkE = gBlendValue;
        gUnk_030051F0.unk4 = REG_BLDCNT;
        gUnk_030051F0.unk6 = REG_BG0CNT;
        gUnk_030051F0.unk8 = REG_BG1CNT;
        gUnk_030051F0.unkA = REG_BG2CNT;
        gUnk_030051F0.unkC = REG_BG3CNT;
        gUnk_030051F0.unk0 = gUnk_03004C20.sceneFrameCounter;

        for (var_r4_2 = 0; var_r4_2 < gUnk_03005428; var_r4_2++) {
            gEntityInfo[var_r4_2].priority += 1;
        }
        gBgInfo[gUnk_030034BC].hOfs = 0;
    }

    gBlendValue = 9;
    gUnk_03004658[0xC] = 0;
    gCallbackQueue.next[0] = ReadKeyInput;
    gCallbackQueue.next[1] = ProcessInputAndUpdateEntities;
    gCallbackQueue.next[3] = NULL + 1;
    if (gUnk_03004C20.level == 8) {
        gCallbackQueue.next[2] = AnimatePaletteEffects;
    } else if (gUnk_03004C20.level == 0) {
        gCallbackQueue.next[2] = VBlankCallback_Dialog;
    } else {
        gCallbackQueue.next[2] = VBlankCallback_Gameplay;
    }
    gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
    gCallbackQueue.nextCount = 4;

    if (gUnk_030034C0 == 1) {
        DmaCopy16(3, gBgDataPtrs.pBufBg3Tiles + 4, BG_VRAM + (var_sl * 0x20), 0xB60);
    } else if (gUnk_030034C0 == 2) {
        DmaCopy16(3, gBgDataPtrs.pBufBg3Tiles + 4, gBgInfo[gUnk_030034BC].pTiles + (var_sl * 0x20), 0xC60);
    } else {
        DmaCopy16(3, gBgDataPtrs.pBufBg3Tiles + 4, gBgInfo[gUnk_030034BC].pTiles + (var_sl * 0x20), 0xCE0);
    }

    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOn();
    m4aSongNumStart(0x55);

    if (gUnk_03004C20.level == 8) {
        AnimatePaletteEffects();
    } else if (gUnk_03004C20.level == 0) {
        VBlankCallback_Dialog();
    } else {
        VBlankCallback_Gameplay();
    }

    if (gUnk_03003410.unk4 != 0) {
        if (gUnk_030034BC == 0) {
            REG_BG0CNT &= ~3; // set priority to 0
            REG_BG0CNT += 0;
            REG_BG1CNT += 1; // increment priority
            REG_BG2CNT += 1; // increment priority
            REG_BG3CNT += 1; // increment priority
            REG_BLDCNT = BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_OBJ | BLDCNT_EFFECT_DARKEN;
        } else {
            REG_BG1CNT &= ~3; // set priority to 0
            REG_BG1CNT += 0;
            REG_BLDCNT = BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_OBJ | BLDCNT_EFFECT_DARKEN;
        }
    }
}
/**
 * UpdateOamSortOrder: level teardown/restore counterpart to InitGameplayState. Decrements entity priorities, renders the HUD, frees
 * the BG3 buffers, restores the BG0/BG1 tiles+tilemap from the decompress buffers, and restores the saved display registers
 * (BLDCNT/BGxCNT and blend) from gUnk_030051F0.
 */
void UpdateOamSortOrder(void) {
    s32 var_r4;

    for (var_r4 = 0; var_r4 < gUnk_03005428; var_r4++) {
        gEntityInfo[var_r4].priority -= 1;
    }

    RenderHUDTop();

    VBlankIntrWait();
    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();
    m4aMPlayAllStop();

    thunk_HeapFree(gBgDataPtrs.pBufBg3Tilemap);
    thunk_HeapFree(gBgDataPtrs.pBufBg3Tiles);
    if (gUnk_030034BC == 0) {
        DmaCopy16(3, gBgDataPtrs.pBufBg0Tiles, gBgInfo[0].pTiles, gBgInfo[0].unk18 * gBgInfo[0].unk16);
        DmaCopy16(3, gBgDataPtrs.pBufBg0Tilemap, &gBgTilemapBufs[0], 0x480);
    } else {
        DmaCopy16(3, gBgDataPtrs.pBufBg1Tiles, gBgInfo[1].pTiles, gBgInfo[1].unk18 * gBgInfo[1].unk16);
        DmaCopy16(3, gBgDataPtrs.pBufBg1Tilemap, &gBgTilemapBufs[1], 0x480);
    }

    gBlendValue = gUnk_030051F0.unkE;
    REG_BLDCNT = gUnk_030051F0.unk4;
    REG_BG0CNT = gUnk_030051F0.unk6;
    REG_BG1CNT = gUnk_030051F0.unk8;
    REG_BG2CNT = gUnk_030051F0.unkA;
    REG_BG3CNT = gUnk_030051F0.unkC;
    gUnk_03004C20.sceneFrameCounter = gUnk_030051F0.unk0;

    if ((gUnk_03004C20.world == 6) && ((gUnk_03004C20.level == 1) || (gUnk_03004C20.level == 3))) {
        REG_WIN1H = WIN_RANGE(0xA0, 0xF0);
        REG_WIN1V = WIN_RANGE(0, 0x10);
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_CLR | WININ_WIN1_BG0 | WININ_WIN1_CLR;
        REG_WINOUT = WINOUT_WIN01_BG_ALL | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR;
        REG_DISPCNT = DISPCNT_MODE_1 | DISPCNT_OBJ_1D_MAP | DISPCNT_BG0_ON | DISPCNT_BG1_ON | DISPCNT_BG2_ON | DISPCNT_OBJ_ON
            | DISPCNT_WIN0_ON | DISPCNT_WIN1_ON;
        UpdateHUDTimePanel();
    }

    DmaCopy16(3, &gBgTilemapBufs[gUnk_030034BC], gBgInfo[gUnk_030034BC].pTilemap, 0x800);

    if (gUnk_03004C20.unk10 == 1) {
        UpdateHUDTimePanel();
    }

    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
}
/**
 * ProcessInputAndUpdateEntities: reads the pause/confirm/D-pad input on the pause or vision-select overlay, updating the menu cursor
 * tilemap and dispatching the selected action (resume, restart, quit).
 */
void ProcessInputAndUpdateEntities(void) {
    u8 sp4;
    u32 var_r1;
    u8 var_r4;
    u8 var_r5;

    sp4 = (gNewKeys & (START_BUTTON | B_BUTTON)) != 0;
    if (gNewKeys & A_BUTTON) {
        sp4 = gUnk_03004658[0xC] + 1;
    }

    if (gNewKeys & (DPAD_DOWN | DPAD_UP)) {
        for (var_r5 = 0; var_r5 < 2; var_r5++) {
            DmaFill16(3, 0, &gBgTilemapBufs[gUnk_030034BC][((var_r5 + (gUnk_03004658[0xC] * 3)) << 5) + 0xA5], 0x4);
            DmaFill16(3, 0, &gBgTilemapBufs[gUnk_030034BC][((var_r5 + (gUnk_03004658[0xC] * 3)) << 5) + 0xB7], 0x4);
        }

        if ((gUnk_030034C0 == 0) || (gUnk_030034C0 == 2)) {
            if (gNewKeys & DPAD_DOWN) {
                m4aSongNumStart(0x51);
                gUnk_03004658[0xC] += 1;
                if (gUnk_03004658[0xC] > 3) {
                    gUnk_03004658[0xC] = 0;
                }
            }

            if (gNewKeys & DPAD_UP) {
                m4aSongNumStart(0x51);
                gUnk_03004658[0xC] -= 1;
                if (gUnk_03004658[0xC] & 0x80) {
                    gUnk_03004658[0xC] = 3;
                }
            }
        } else {
            if (gNewKeys & DPAD_DOWN) {
                m4aSongNumStart(0x51);
                gUnk_03004658[0xC] += 1;
                if (gUnk_03004658[0xC] > 2) {
                    gUnk_03004658[0xC] = 0;
                }
            }

            if (gNewKeys & DPAD_UP) {
                m4aSongNumStart(0x51);
                gUnk_03004658[0xC] -= 1;
                if (gUnk_03004658[0xC] & 0x80) {
                    gUnk_03004658[0xC] = 2;
                }
            }
        }

        for (var_r5 = 0; var_r5 < 2; var_r5++) {
            for (var_r4 = 0; var_r4 < 2; var_r4++) {
                if (gUnk_030034BC == 0) {
                    gBgTilemapBufs[gUnk_030034BC][((var_r5 + (gUnk_03004658[0xC] * 3)) << 5) + 0xA5 + var_r4]
                        = gBgDataPtrs.pBufBg3Tilemap[(var_r5 * 0x1E) + 0x9D + var_r4] + gUnk_03000800;
                    gBgTilemapBufs[gUnk_030034BC][((var_r5 + (gUnk_03004658[0xC] * 3)) << 5) + 0xB7 + var_r4]
                        = gBgDataPtrs.pBufBg3Tilemap[(var_r5 * 0x1E) + 0xAF + var_r4] + gUnk_03000800;
                } else {
                    gBgTilemapBufs[gUnk_030034BC][((var_r5 + (gUnk_03004658[0xC] * 3)) << 5) + 0xA5 + var_r4]
                        = gBgDataPtrs.pBufBg3Tilemap[(var_r5 * 0x1E) + 0x9D + var_r4];
                    gBgTilemapBufs[gUnk_030034BC][((var_r5 + (gUnk_03004658[0xC] * 3)) << 5) + 0xB7 + var_r4]
                        = gBgDataPtrs.pBufBg3Tilemap[(var_r5 * 0x1E) + 0xAF + var_r4];
                }
            }
        }
    }

    if (sp4 > 0) {
        sp4 = gUnk_081166F8[gUnk_030034C0][sp4 - 1];
    }

    switch (sp4 - 1) {
        case 0:
            gCallbackQueue.next[0] = ReadKeyInput;
            for (var_r1 = 0; var_r1 < 10; var_r1++) {
                gCallbackQueue.next[var_r1] = gCallbackQueue.previous[var_r1];
            }
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
            gCallbackQueue.nextCount = gCallbackQueue.previousCount;

            UpdateOamSortOrder();
            m4aSoundVSyncOn();
            m4aMPlayAllContinue();
            break;

        case 1:
            UpdateOamSortOrder();
            gCallbackQueue.current[1] = TransitionFadeOutFull;
            gUnk_03005220.unk4 = gUnk_03005284->unk18;
            gUnk_03005220.lives = gUnk_03005284->unk0;
            gUnk_03005220.hearts = gUnk_03005284->unk8_0;
            REG_BLDCNT = 0;
            gBlendValue = 0;
            break;

        case 2:
            gUnk_03005284->unk0 = gUnk_03005220.lives = gUnk_03005284->unk1E;
            UpdateOamSortOrder();
            gBlendValue = 0;
            if (gUnk_03004C20.world == 6 && gUnk_03004C20.level == 8) {
                gUnk_03004C20.world = 5;
            }
            gUnk_030034B0.unk6_4 = gUnk_03004C20.level;
            gUnk_03004C20.level = 0;
            gCallbackQueue.current[1] = TransitionGameOver;
            break;

        case 3:
            gCallbackQueue.current[1] = SetupBG3WindowOverlay;
            thunk_HeapFree(gBgDataPtrs.pBufBg3Tilemap);
            thunk_HeapFree(gBgDataPtrs.pBufBg3Tiles);
            break;

        case 4:
            if (gUnk_03004C20.level != 0) {
                gUnk_03005284->unk0 = gUnk_03005220.lives = gUnk_03005284->unk1E;
            }
            UpdateOamSortOrder();
            REG_BLDCNT = 0;
            gBlendValue = 0;
            FreeAllDecompBuffers();
            gCallbackQueue.current[1] = TransitionToGameplayScreen;
            break;

        case 5:
            gBlendValue = 0;
            gUnk_03004C20.level = 9;
            gUnk_03004D9C = 0;
            InitOamEntries();
            gCallbackQueue.current[1] = TransitionSoftReset;
            thunk_HeapFree(gBgDataPtrs.pBufBg3Tilemap);
            thunk_HeapFree(gBgDataPtrs.pBufBg3Tiles);
            break;
    }
}
/**
 * SetupBG3WindowOverlay: allocates and decompresses the BG3 tiles/tilemap, fills the BG3 tilemap buffer (per-scanline stride), and
 * configures the WIN1 window registers for the level text/overlay layer.
 */
void SetupBG3WindowOverlay(void) {
    s32 var_r5;
    s32 var_r6;
    s32 var_r7;

    if (gUnk_030034BC == 0) {
        var_r7 = gUnk_03000800;
    } else {
        var_r7 = 0;
    }

    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOff();

    gBgDataPtrs.pBufBg3Tiles = thunk_HeapAlloc(gUnk_082EC8F4 & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg3Tilemap = thunk_HeapAlloc(gUnk_082ECD74 & 0x7FFFFFFF, 0);
    Decompress(gBgDataPtrs.pBufBg3Tiles, &gUnk_082EC8F4);
    Decompress(gBgDataPtrs.pBufBg3Tilemap, &gUnk_082ECD74);

    for (var_r5 = 0, var_r6 = 0; var_r5 < 0x21C; var_r6++, var_r5++) {
        if (((var_r5 % 30) == 0) && (var_r5 != 0)) {
            var_r6 += 2;
        }
        gBgTilemapBufs[gUnk_030034BC][var_r6] = gBgDataPtrs.pBufBg3Tilemap[var_r5 + 2] + var_r7;
    }

    REG_WIN1H = WIN_RANGE(0, 0xF0);
    REG_WIN1V = WIN_RANGE(0x1E, 0x90);

    if (gUnk_030034BC == 0) {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_CLR | WININ_WIN1_BG1 | WININ_WIN1_BG2 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
    } else {
        REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_CLR | WININ_WIN1_BG0 | WININ_WIN1_BG2 | WININ_WIN1_OBJ | WININ_WIN1_CLR;
    }

    if (gUnk_03004C20.level == 8) {
        gCallbackQueue.next[2] = AnimatePaletteEffects;
    } else {
        gCallbackQueue.next[2] = VBlankCallback_Gameplay;
    }
    gCallbackQueue.next[0] = ReadKeyInput;
    gCallbackQueue.next[1] = UpdateWorldMapInput;
    gCallbackQueue.next[3] = NULL + 1;
    gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
    gCallbackQueue.nextCount = 4;

    DmaCopy16(3, gBgDataPtrs.pBufBg3Tiles + 4, gBgInfo[gUnk_030034BC].pTiles + (var_r7 << 5), 0xB80);

    REG_DISPCNT |= DISPCNT_WIN1_ON;
    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOn();

    gUnk_03004C20.sceneFrameCounter = 0;
}
/**
 * UpdateWorldMapInput: per-frame input handler for the world-map/vision-select screen. Advances the scene timer, reads D-pad to move
 * the selection (with SFX), and A/B to confirm, writing the chosen slot into the save struct (gUnk_03005284) and triggering the
 * level-entry or palette-effect callbacks.
 */
void UpdateWorldMapInput(void) {
    gUnk_03004C20.sceneFrameCounter += 5;
    if (gUnk_03004C20.sceneFrameCounter > 96) {
        if (gUnk_03004C20.sceneFrameCounter < 200) {
            gUnk_03004C20.sceneFrameCounter = 96;
            if (gNewKeys & DPAD_RIGHT) {
                gUnk_03005284->unk1C = 1;
                m4aSongNumStart(0x51);
                gUnk_03004C20.sceneFrameCounter = 0;
            } else if (gNewKeys & DPAD_LEFT) {
                gUnk_03005284->unk1C = 2;
                m4aSongNumStart(0x51);
                gUnk_03004C20.sceneFrameCounter = 0;
            }

            if (gNewKeys & (B_BUTTON | A_BUTTON)) {
                if (gNewKeys & A_BUTTON) {
                    m4aSongNumStart(0x52);
                } else {
                    m4aSongNumStart(0x54);
                }

                gUnk_03004C20.sceneFrameCounter = 200;
                if ((gNewKeys & A_BUTTON) && (gUnk_03005284->unk1C == gUnk_03005284->unk1D)) {
                    gUnk_03005284->unk1D = 3 ^ gUnk_03005284->unk1D;
                } else {
                    gUnk_03005284->unk1C = 3 - gUnk_03005284->unk1D;
                }
            }
        } else if (gUnk_03004C20.sceneFrameCounter > 350) {
            gUnk_03003410.unk4 = 0;

            if (gUnk_03004C20.level == 8) {
                gCallbackQueue.next[2] = AnimatePaletteEffects;
            } else {
                gCallbackQueue.next[2] = VBlankCallback_Gameplay;
            }
            gCallbackQueue.next[0] = ReadKeyInput;
            gCallbackQueue.next[1] = InitGameplayState;
            gCallbackQueue.next[3] = NULL + 1;
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
            gCallbackQueue.nextCount = 4;

            thunk_HeapFree(gBgDataPtrs.pBufBg3Tilemap);
            thunk_HeapFree(gBgDataPtrs.pBufBg3Tiles);
        }
    }

    if (gUnk_03004C20.sceneFrameCounter < 200) {
        if (gUnk_03005284->unk1C == 1) {
            REG_WIN1H = WIN_RANGE(0, 0xE0 - gUnk_03004C20.sceneFrameCounter);
        } else {
            REG_WIN1H = WIN_RANGE(gUnk_03004C20.sceneFrameCounter + 0x18, 0xE0);
        }
    }
}
u8 CheckWorldCompletion(u8 arg0) {
    u32 var_r0;
    u32 var_r2;
    u8 var_sl;
    u8 var_ip;
    u8 var_r6;
    u8 var_r8;
    u8 v50;
    if (arg0 < 4) {
        if (((gUnk_03004670->unk8[arg0][7] & 0x80) != 0) && ((gUnk_03004670->unk8[arg0 + 1][0] & 0x7F) != 0x7F))
            return 1;
    } else if ((gUnk_03004670->unk8[5][7] & 0x80) != 0) {
        var_r8 = 0;
        var_ip = 0;
        var_r6 = 0;
        var_sl = 0;
        for (var_r0 = 0; var_r0 < 5; var_r0++) {
            for (var_r2 = 0; var_r2 < 7; var_r2++) {
                if ((var_r2 == 3 || var_r2 == 5) && (gUnk_03004670->unk8[var_r0][var_r2] & 0x7F) == 0x64)
                    var_r8 += 1;
                else if ((var_r2 != 7) && ((gUnk_03004670->unk8[var_r0][var_r2] & 0x7F) == 0x1E))
                    var_ip += 1;
                if (gUnk_03004670->unk8[var_r0][var_r2] & 0x80)
                    var_r6 += 1;
            }
        }
        v50 = gUnk_03004670->unk8[5][0] & 0x7F;
        if (v50 == 0x1E)
            var_sl += 1;
        if ((gUnk_03004670->unk8[5][1] & 0x7F) == 0x1E)
            var_sl += 1;
        if ((arg0 == 4) && (v50 != 0x7F) && (var_r6 == 0x23))
            return 1;
        if ((arg0 == 5) && ((gUnk_03004670->unk8[5][1] & 0x7F) != 0x7F) && ((var_ip + var_r8) > 0x18))
            return 1;
        if ((arg0 == 6) && ((gUnk_03004670->unk8[5][2] & 0x7F) != 0x7F) && ((var_ip + var_r8 + var_sl) == 0x25))
            return 1;
    }
    return 0;
}

/**
 * CopyWorldMapTiles: ported from kleod CopyWorldMapTiles.
 */
void CopyWorldMapTiles(u8 arg0) {
    u8 var_r0;
    u8 var_r3;

    arg0 += 1;
    if (arg0 < 5) {
        // Variables must be declared in this scope to match
        u16 *var_r4 = &gBgTilemapBufs[1][gUnk_08116708[arg0][2] + (gUnk_08116708[arg0][3] << 5)];
        u16 *var_r2 = &gBgTilemapBufs[1][arg0 * 0x5 + 0x280];
        for (var_r0 = 0; var_r0 < 4; var_r0++) {
            for (var_r3 = 0; var_r3 < 5; var_r3++) {
                var_r4[var_r3] = var_r2[var_r3];
            }
            var_r4 += 0x20;
            var_r2 += 0x20;
        }
    } else {
        gBgTilemapBufs[1][gUnk_08116708[arg0][2] + ((gUnk_08116708[arg0][3] + 0) << 5) + 0]
            = gBgTilemapBufs[1][((arg0 - 5) * 2) + 0x340];
        gBgTilemapBufs[1][gUnk_08116708[arg0][2] + ((gUnk_08116708[arg0][3] + 0) << 5) + 1]
            = gBgTilemapBufs[1][((arg0 - 5) * 2) + 0x341];
        gBgTilemapBufs[1][gUnk_08116708[arg0][2] + ((gUnk_08116708[arg0][3] + 1) << 5) + 0]
            = gBgTilemapBufs[1][((arg0 - 5) * 2) + 0x360];
        gBgTilemapBufs[1][gUnk_08116708[arg0][2] + ((gUnk_08116708[arg0][3] + 1) << 5) + 1]
            = gBgTilemapBufs[1][((arg0 - 5) * 2) + 0x361];
    }
}
/**
 * SetWorldMapTilePalette: ported from kleod SetWorldMapTilePalette.
 */
void SetWorldMapTilePalette(u8 arg0, u8 arg1) {
    u16 temp_sl;
    u8 var_r0;
    u8 var_r3;
    u16 *var_r5; // Must be declared last to match

    arg0 += 1;
    var_r5 = &gBgTilemapBufs[1][gUnk_08116708[arg0][2] + (gUnk_08116708[arg0][3] << 5)];
    temp_sl = var_r5[0x20];
    for (var_r0 = 0; var_r0 < 4; var_r0++) {
        for (var_r3 = 0; var_r3 < 5; var_r3++) {
            var_r5[var_r3] = (var_r5[var_r3] & 0xFFF) | (arg1 << 0xC);
        }
        var_r5 += 0x20;
    }

    if (arg0 == 4) {
        gBgTilemapBufs[1][gUnk_08116708[4][2] + (gUnk_08116708[4][3] << 5) + 0x20] = temp_sl;
    }
}
/**
 * UpdateWorldMapNodeTile: ported from kleod UpdateWorldMapNodeTile.
 */
void UpdateWorldMapNodeTile(u8 arg0) {
    vu32 a; // Required to match
    if (arg0 < 4) {
        gBgTilemapBufs[1][gUnk_08116748[arg0][0] + (gUnk_08116748[arg0][1] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 0];
        gBgTilemapBufs[1][gUnk_08116748[arg0][2] + (gUnk_08116748[arg0][3] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 1];
        gBgTilemapBufs[1][gUnk_08116748[arg0][4] + (gUnk_08116748[arg0][5] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 2];
        gBgTilemapBufs[1][gUnk_08116748[arg0][6] + (gUnk_08116748[arg0][7] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 3];
    } else {
        gBgTilemapBufs[1][gUnk_08116748[arg0][0] + (gUnk_08116748[arg0][1] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 4];
        gBgTilemapBufs[1][gUnk_08116748[arg0][2] + (gUnk_08116748[arg0][3] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 5];
        gBgTilemapBufs[1][gUnk_08116748[arg0][4] + (gUnk_08116748[arg0][5] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 6];
        gBgTilemapBufs[1][gUnk_08116748[arg0][6] + (gUnk_08116748[arg0][7] << 5)]
            = gBgTilemapBufs[1][((gUnk_08116880[arg0] + 0x1C) * 0x20) + 7];
    }
}
/*
 * Iterates over entity slots 0-6 and updates active ones.
 * For each slot, checks if the entity is active via CheckWorldCompletion.
 * If active, calls CopyWorldMapTiles and UpdateWorldMapNodeTile to update it.
 *   no parameters
 *   no return value
 */
void UpdateEntities(void) {
    u8 i;
    for (i = 0; i <= 6; i++) {
        if ((u8)CheckWorldCompletion(i)) {
            CopyWorldMapTiles(i);
            UpdateWorldMapNodeTile(i);
        }
    }
}
/**
 * CountCollectedGems: counts collected gems/dream-stones for the current world by scanning the per-level progress bytes in
 * gUnk_03004670, returning whether the world is complete.
 */
void CountCollectedGems(void) {
    u8 sp0;
    u8 var_ip;
    u8 var_r0;
    u8 var_r3;
    u8 var_r6;
    u8 var_r8;

    if ((gUnk_03004670->unk8[5][7] & 0x80) != 0) {
        var_ip = 0;
        var_r8 = 0;
        var_r6 = 0;
        sp0 = 0;
        for (var_r0 = 0; var_r0 < 5; var_r0++) {
            for (var_r3 = 0; var_r3 < 7; var_r3++) {
                if ((var_r3 == 3 || var_r3 == 5) && (gUnk_03004670->unk8[var_r0][var_r3] & 0x7F) == 0x64) {
                    var_ip += 1;
                } else if ((var_r3 != 7) && ((gUnk_03004670->unk8[var_r0][var_r3] & 0x7F) == 0x1E)) {
                    var_r8 += 1;
                }
                if (gUnk_03004670->unk8[var_r0][var_r3] & 0x80) {
                    var_r6 += 1;
                }
            }
        }

        if ((gUnk_03004670->unk8[5][0] & 0x7F) == 0x1E) {
            sp0 += 1;
        }
        if ((gUnk_03004670->unk8[5][1] & 0x7F) == 0x1E) {
            sp0 += 1;
        }

        if (((gUnk_03004670->unk8[5][0] & 0x7F) == 0x7F) && (var_r6 == 0x23)) {
            gUnk_03004C08.unk0_0 = 4;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[5][0] = 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else if (((gUnk_03004670->unk8[5][1] & 0x7F) == 0x7F) && ((var_r8 + var_ip) > 0x18)) {
            gUnk_03004C08.unk0_0 = 5;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[5][1] = 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else if (((gUnk_03004670->unk8[5][2] & 0x7F) == 0x7F) && ((sp0 + var_ip + var_r8) == 0x25)) {
            gUnk_03004C08.unk0_0 = 6;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[5][2] = 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else {
            // gCallbackQueue.current[1] = GameplayMainLoop;
        }
    } else {
        if (((gUnk_03004670->unk8[0][7] & 0x80) != 0) && ((gUnk_03004670->unk8[1][0] & 0x7F) == 0x7F)) {
            gUnk_03004C08.unk0_0 = 0;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[1][0] &= 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else if (((gUnk_03004670->unk8[1][7] & 0x80) != 0) && ((gUnk_03004670->unk8[2][0] & 0x7F) == 0x7F)) {
            gUnk_03004C08.unk0_0 = 1;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[2][0] &= 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else if (((gUnk_03004670->unk8[2][7] & 0x80) != 0) && ((gUnk_03004670->unk8[3][0] & 0x7F) == 0x7F)) {
            gUnk_03004C08.unk0_0 = 2;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[3][0] &= 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else if (((gUnk_03004670->unk8[3][7] & 0x80) != 0) && ((gUnk_03004670->unk8[4][0] & 0x7F) == 0x7F)) {
            gUnk_03004C08.unk0_0 = 3;
            gUnk_03004C08.unk2 = 0;
            gUnk_03004670->unk8[4][0] &= 0x80;
            gCallbackQueue.current[1] = UpdateWorldMapNodeAnim;
            return;
        } else {
            // gCallbackQueue.current[1] = GameplayMainLoop;
        }
    }
    gCallbackQueue.current[1] = GameplayMainLoop;
}
/**
 * UpdateWorldMapNodeAnim: drives the world-map node reveal animation across its 8 phases (tile/palette cycling with a random palette
 * pick) as level completion advances, then hands off to the next map callback.
 */
void UpdateWorldMapNodeAnim(void) {
    s32 var_r7;
    s32 temp_r6;

    if (gUnk_03004C08.unk0_0 < 4) {
        var_r7 = 0;
        temp_r6 = gUnk_08116880[gUnk_03004C08.unk0_0];
    } else {
        var_r7 = 4;
        temp_r6 = gUnk_08116880[gUnk_03004C08.unk0_0];
    }

    switch ((gUnk_03004C08.unk2 / 30) - 1) {
        case 0:
            if ((gUnk_03004C08.unk2 % 30) == 0) {
                m4aSongNumStart(0x89);
            }
            gBgTilemapBufs[1][gUnk_08116748[gUnk_03004C08.unk0_0][0] + (gUnk_08116748[gUnk_03004C08.unk0_0][1] << 5)]
                = gBgTilemapBufs[1][0 + var_r7 + ((temp_r6 + 0x1C) << 5)];
            break;

        case 1:
            if ((gUnk_03004C08.unk2 % 30) == 0) {
                m4aSongNumStart(0x89);
            }
            gBgTilemapBufs[1][gUnk_08116748[gUnk_03004C08.unk0_0][2] + (gUnk_08116748[gUnk_03004C08.unk0_0][3] << 5)]
                = gBgTilemapBufs[1][1 + var_r7 + ((temp_r6 + 0x1C) << 5)];
            break;

        case 2:
            if (gUnk_03004C08.unk0_0 == 6) {
                gUnk_03004C08.unk2 = 0x95;
            } else {
                if ((gUnk_03004C08.unk2 % 30) == 0) {
                    m4aSongNumStart(0x89);
                }
                gBgTilemapBufs[1][gUnk_08116748[gUnk_03004C08.unk0_0][4] + (gUnk_08116748[gUnk_03004C08.unk0_0][5] << 5)]
                    = gBgTilemapBufs[1][2 + var_r7 + ((temp_r6 + 0x1C) << 5)];
            }
            break;

        case 3:
            if ((gUnk_03004C08.unk0_0 != 4) && (gUnk_03004C08.unk0_0 != 6)) {
                if ((gUnk_03004C08.unk2 % 30) == 0) {
                    m4aSongNumStart(0x89);
                }
                gBgTilemapBufs[1][gUnk_08116748[gUnk_03004C08.unk0_0][6] + (gUnk_08116748[gUnk_03004C08.unk0_0][7] << 5)]
                    = gBgTilemapBufs[1][3 + var_r7 + ((temp_r6 + 0x1C) << 5)];
            }
            break;

        case 4:
            if ((gUnk_03004C08.unk2 % 30) == 0) {
                m4aSongNumStart(0x8A);
            }
            /* fallthrough */
        case 5:
        case 6:
            if (gUnk_03004C08.unk0_0 < 4) {
                if ((gUnk_03004C20.sceneFrameCounter % 4) == 0) {
                    DmaCopy16(3, &gUnk_08116780[thunk_sub_080002A0() % 8], BG_PLTT + 0x160, 0x20);
                    SetWorldMapTilePalette(gUnk_03004C08.unk0_0, 0xB);
                }
            } else {
                CopyWorldMapTiles(gUnk_03004C08.unk0_0);
            }
            break;

        case 7:
            CopyWorldMapTiles(gUnk_03004C08.unk0_0);
            gCallbackQueue.current[1] = CountCollectedGems;
            break;
    }

    gUnk_03004C08.unk2 += 1;
}
/**
 * UpdateAllEntities: per-frame world-map/level entity driver. Selects the active world/level slot, flushes the OAM buffer to hardware,
 * and iterates the per-level progress table in gUnk_03004670 to drive each entity/node state.
 */
void UpdateAllEntities(void) {
    s32 var_r4;
    s32 var_r5;
    s32 temp_r6;
    s32 temp_r8;

    if (gUnk_03004C20.world == 6) {
        gUnk_03004C08.unk0_4 = (gUnk_03004C20.world - 1) + (gUnk_03004C20.level - 1);
    } else {
        gUnk_03004C08.unk0_4 = gUnk_03004C20.world - 1;
    }
    gUnk_03004C08.unk1 = 0;

    temp_r6 = gUnk_03004C20.world;
    temp_r8 = gUnk_03004C20.level;
    ClearVideoState();
    DmaCopy32(3, gOamBuffer, OAM, OAM_SIZE);
    gUnk_03003410.unk8 = 0;
    gUnk_03004C20.world = 1;
    gUnk_03004C20.level = 1;
    gUnk_03004C20.unkA = 0;
    ResetVideoRegisters();
    gUnk_03004C20.world = temp_r6;
    gUnk_03004C20.level = temp_r8;

    gOamAffineMatrix[0].pa = 0x100;
    gOamAffineMatrix[0].pb = 0;
    gOamAffineMatrix[0].pc = 0;
    gOamAffineMatrix[0].pd = 0x100;

    gEntityInfo[0].unk10 = 1;
    for (var_r4 = 1; var_r4 < 0x13; var_r4++) {
        gEntityInfo[var_r4].unk10 = 0;
    }

    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    REG_IE &= ~INTR_FLAG_HBLANK;
    REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
    m4aSoundVSyncOff();
    m4aMPlayAllStop();
    REG_DISPCNT = 0;

    gBgInfo[0].pTiles = BG_VRAM;
    gBgInfo[1].pTiles = BG_VRAM + 0x4000;
    gBgInfo[2].pTiles = BG_VRAM + 0x8000;
    gBgInfo[3].pTiles = BG_VRAM + 0xC000;
    gBgInfo[0].pTilemap = BG_VRAM + 0xE000;
    gBgInfo[1].pTilemap = BG_VRAM + 0xE800;
    gBgInfo[2].pTilemap = BG_VRAM + 0xF000;
    gBgInfo[3].pTilemap = BG_VRAM + 0xF800;
    DecompressDma(&gUnk_083128F8, BG_PLTT, BG_PLTT_SIZE);

    gBgDataPtrs.pBufBg0Tiles = thunk_HeapAlloc(gUnk_08312A58 & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg0Tilemap = thunk_HeapAlloc(gUnk_08312B70 & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg1Tiles = thunk_HeapAlloc(gUnk_08312BD8 & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg1Tilemap = thunk_HeapAlloc(gUnk_08313C34 & 0x7FFFFFFF, 0);
    Decompress(gBgDataPtrs.pBufBg0Tiles, &gUnk_08312A58);
    Decompress(gBgDataPtrs.pBufBg0Tilemap, &gUnk_08312B70);
    Decompress(gBgDataPtrs.pBufBg1Tiles, &gUnk_08312BD8);
    Decompress(gBgDataPtrs.pBufBg1Tilemap, &gUnk_08313C34);
    gBgDataPtrs.pBufBg0Tiles += 4;
    gBgDataPtrs.pBufBg0Tilemap += 2;
    gBgDataPtrs.pBufBg1Tiles += 4;
    gBgDataPtrs.pBufBg1Tilemap += 2;
    DmaCopy16Wait(3, gBgDataPtrs.pBufBg0Tiles, gBgInfo[0].pTiles, 0x260);
    DmaCopy16Wait(3, gBgDataPtrs.pBufBg1Tiles, gBgInfo[1].pTiles, 0x1BC0);
    DmaFill16(3, 0, &gBgTilemapBufs[0], 0x800);

    for (var_r4 = 0, var_r5 = 0; var_r4 < 0x258; var_r5++, var_r4++) {
        if (((var_r4 % 30) == 0) && (var_r4 != 0)) {
            var_r5 += 2;
        }
        gBgTilemapBufs[0][var_r5] = gBgDataPtrs.pBufBg0Tilemap[var_r4];
    }

    for (var_r4 = 0; var_r4 < 0x400; var_r4++) {
        gBgTilemapBufs[1][var_r4] = gBgDataPtrs.pBufBg1Tilemap[var_r4];
    }

    thunk_HeapFree(gBgDataPtrs.pBufBg1Tilemap - 2);
    thunk_HeapFree(gBgDataPtrs.pBufBg1Tiles - 4);
    thunk_HeapFree(gBgDataPtrs.pBufBg0Tilemap - 2);
    thunk_HeapFree(gBgDataPtrs.pBufBg0Tiles - 4);

    gBgDataPtrs.pBufBg0Tiles = thunk_HeapAlloc(gUnk_08313F24 & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg0Tilemap = thunk_HeapAlloc(gUnk_083141F0 & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg1Tiles = thunk_HeapAlloc(gUnk_083142EC & 0x7FFFFFFF, 0);
    gBgDataPtrs.pBufBg1Tilemap = thunk_HeapAlloc(gUnk_083155C4 & 0x7FFFFFFF, 0);
    Decompress(gBgDataPtrs.pBufBg0Tiles, &gUnk_08313F24);
    Decompress(gBgDataPtrs.pBufBg0Tilemap, &gUnk_083141F0);
    Decompress(gBgDataPtrs.pBufBg1Tiles, &gUnk_083142EC);
    Decompress(gBgDataPtrs.pBufBg1Tilemap, &gUnk_083155C4);
    gBgDataPtrs.pBufBg0Tiles += 4;
    gBgDataPtrs.pBufBg0Tilemap += 2;
    gBgDataPtrs.pBufBg1Tiles += 4;
    gBgDataPtrs.pBufBg1Tilemap += 2;
    DmaCopy16Wait(3, gBgDataPtrs.pBufBg0Tiles, gBgInfo[2].pTiles, 0x820);
    DmaCopy16Wait(3, gBgDataPtrs.pBufBg1Tiles, gBgInfo[3].pTiles, 0x1A80);

    for (var_r4 = 0, var_r5 = 0; var_r4 <= 0x257; var_r5++, var_r4++) {
        if (((var_r4 % 30) == 0) && (var_r4 != 0)) {
            var_r5 += 2;
        }
        gBgTilemapBufs[2][var_r5] = gBgDataPtrs.pBufBg0Tilemap[var_r4];
        gBgTilemapBufs[3][var_r5] = gBgDataPtrs.pBufBg1Tilemap[var_r4];
    }

    thunk_HeapFree(gBgDataPtrs.pBufBg1Tilemap - 2);
    thunk_HeapFree(gBgDataPtrs.pBufBg1Tiles - 4);
    thunk_HeapFree(gBgDataPtrs.pBufBg0Tilemap - 2);
    thunk_HeapFree(gBgDataPtrs.pBufBg0Tiles - 4);

    REG_DISPCNT = 0x3F40;
    REG_BG0CNT = 0x1C41;
    REG_BG1CNT = 0x1D46;
    REG_BG2CNT = 0x1E48;
    REG_BG3CNT = 0x1F4F;

    REG_WININ = WININ_WIN0_BG0 | WININ_WIN0_BG1 | WININ_WIN0_BG3 | WININ_WIN0_OBJ | WININ_WIN0_CLR;
    REG_WINOUT = WINOUT_WIN01_BG0 | WINOUT_WIN01_BG1 | WINOUT_WIN01_BG3 | WINOUT_WIN01_OBJ | WINOUT_WIN01_CLR;

    REG_WIN0H = gUnk_08116728[gUnk_03004C08.unk0_4][0];
    REG_WIN0V = gUnk_08116728[gUnk_03004C08.unk0_4][1];

    gEntityInfo[0].xPosBg2 = gUnk_08116708[gUnk_03004C08.unk0_4][0];
    gEntityInfo[0].yPosBg2 = gUnk_08116708[gUnk_03004C08.unk0_4][1];

    gBgInfo[3].vOfs = 0;
    gBgInfo[3].hOfs = 0;
    gBgInfo[2].hOfs = 0;
    gBgInfo[1].vOfs = 0;
    gBgInfo[1].hOfs = 0;
    gBgInfo[0].hOfs = 0;
    gBgInfo[2].vOfs = 4;
    gBgInfo[0].vOfs = 0x400;

    if ((gUnk_03004670->unk1 == 0) && (gUnk_03004670->unk2 == 0) && (gUnk_03004670->unk3 == 0)) {
        gUnk_03004670->unk1 = 0x63;
        gUnk_03004670->unk2 = 0x3B;
        gUnk_03004670->unk3 = 0x63;
    }
    if ((gUnk_03004670->unk4 == 0) && (gUnk_03004670->unk5 == 0) && (gUnk_03004670->unk6 == 0)) {
        gUnk_03004670->unk4 = 0x63;
        gUnk_03004670->unk5 = 0x3B;
        gUnk_03004670->unk6 = 0x63;
    }

    if (gUnk_03004C08.unk0_4 == 5) {
        gBgTilemapBufs[0][0x4C] = 0xA00C;
        gBgTilemapBufs[0][0x4D] = 0xA00D;
        gBgTilemapBufs[0][0x4E] = 0xA00E;
        gBgTilemapBufs[0][0x4F] = 0xA00F;
        gBgTilemapBufs[0][0x50] = 0xA010;

        gBgTilemapBufs[0][0x6D] = 0xA000 | ((gUnk_03004670->unk1 / 10) + 1);
        gBgTilemapBufs[0][0x6E] = 0xA000 | ((gUnk_03004670->unk1 % 10) + 1);
        gBgTilemapBufs[0][0x6F] = 0xA00B;
        gBgTilemapBufs[0][0x70] = 0xA000 | ((gUnk_03004670->unk2 / 10) + 1);
        gBgTilemapBufs[0][0x71] = 0xA000 | ((gUnk_03004670->unk2 % 10) + 1);
        gBgTilemapBufs[0][0x72] = 0xA00B;
        gBgTilemapBufs[0][0x73] = 0xA000 | ((gUnk_03004670->unk3 / 10) + 1);
        gBgTilemapBufs[0][0x74] = 0xA000 | ((gUnk_03004670->unk3 % 10) + 1);

        gBgTilemapBufs[0][0x2C] = 0xA011;
        gBgTilemapBufs[0][0x30] = 0xA010;
        gBgTilemapBufs[0][0x31] = 0xA004;
        gBgTilemapBufs[0][0x32] = 0xA001;

        gBgTilemapBufs[0][0x2E] = 0xA000 | (((gUnk_03004670->unk8[5][0] & 0x7F) / 10) + 1);
        gBgTilemapBufs[0][0x2F] = 0xA000 | (((gUnk_03004670->unk8[5][0] & 0x7F) % 10) + 1);
    } else if (gUnk_03004C08.unk0_4 == 6) {
        gBgTilemapBufs[0][0x4C] = 0xA000;
        gBgTilemapBufs[0][0x4D] = 0xA000;
        gBgTilemapBufs[0][0x4E] = 0xA000;
        gBgTilemapBufs[0][0x4F] = 0xA000;
        gBgTilemapBufs[0][0x50] = 0xA000;

        gBgTilemapBufs[0][0x6D] = 0xA000;
        gBgTilemapBufs[0][0x6E] = 0xA000;
        gBgTilemapBufs[0][0x6F] = 0xA000;
        gBgTilemapBufs[0][0x70] = 0xA000;
        gBgTilemapBufs[0][0x71] = 0xA000;
        gBgTilemapBufs[0][0x72] = 0xA000;
        gBgTilemapBufs[0][0x73] = 0xA000;
        gBgTilemapBufs[0][0x74] = 0xA000;

        gBgTilemapBufs[0][0x2C] = 0xA011;
        gBgTilemapBufs[0][0x30] = 0xA010;
        gBgTilemapBufs[0][0x31] = 0xA004;
        gBgTilemapBufs[0][0x32] = 0xA001;

        gBgTilemapBufs[0][0x2E] = 0xA000 | (((gUnk_03004670->unk8[5][1] & 0x7F) / 10) + 1);
        gBgTilemapBufs[0][0x2F] = 0xA000 | (((gUnk_03004670->unk8[5][1] & 0x7F) % 10) + 1);
    } else if (gUnk_03004C08.unk0_4 == 7) {
        gBgTilemapBufs[0][0x4C] = 0xA00C;
        gBgTilemapBufs[0][0x4D] = 0xA00D;
        gBgTilemapBufs[0][0x4E] = 0xA00E;
        gBgTilemapBufs[0][0x4F] = 0xA00F;
        gBgTilemapBufs[0][0x50] = 0xA010;

        gBgTilemapBufs[0][0x6D] = 0xA000 | ((gUnk_03004670->unk4 / 10) + 1);
        gBgTilemapBufs[0][0x6E] = 0xA000 | ((gUnk_03004670->unk4 % 10) + 1);
        gBgTilemapBufs[0][0x6F] = 0xA00B;
        gBgTilemapBufs[0][0x70] = 0xA000 | ((gUnk_03004670->unk5 / 10) + 1);
        gBgTilemapBufs[0][0x71] = 0xA000 | ((gUnk_03004670->unk5 % 10) + 1);
        gBgTilemapBufs[0][0x72] = 0xA00B;
        gBgTilemapBufs[0][0x73] = 0xA000 | ((gUnk_03004670->unk6 / 10) + 1);
        gBgTilemapBufs[0][0x74] = 0xA000 | ((gUnk_03004670->unk6 % 10) + 1);

        gBgTilemapBufs[0][0x2C] = 0xA000;
        gBgTilemapBufs[0][0x30] = 0xA000;
        gBgTilemapBufs[0][0x31] = 0xA000;
        gBgTilemapBufs[0][0x32] = 0xA000;

        gBgTilemapBufs[0][0x2E] = 0xA000;
        gBgTilemapBufs[0][0x2F] = 0xA000;
    }

    REG_BG0HOFS = 0;
    REG_BG0VOFS = 0;
    REG_BG1HOFS = 0;
    REG_BG1VOFS = 0;
    REG_BG2HOFS = 0;
    REG_BG2VOFS = 0;
    REG_BG3HOFS = 0;
    REG_BG3VOFS = 0;

    gBg2X = gBg2Y = 0;
    SetPaletteAnimEntry(0, 0);
    UpdatePaletteAnimations();
    gIntrTable.vBlank = VBlankHandler_ModeB;
    gCallbackQueue.current[1] = CountCollectedGems;
    UpdateEntities();
    gUnk_03005284->unk1 = gUnk_03004C20.world;
    SaveGameWithVerify(0, 7);
    SaveGameWithVerify(1, 0);

    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOn();
    m4aSongNumStart(3);
}
/**
 * GameplayMainLoop: top-level per-frame gameplay tick — advances the fade blend, runs palette animations, updates the camera/scroll
 * from the player position, and steps all entity behaviors unless the scene is paused.
 */
void GameplayMainLoop(void) {
    struct Unk_0800BEF0 sp0;
    u32 spC;
    u8 var_r4;

    if (gUnk_030034E4 == 1) {
        return;
    }

    if ((gBlendValue < BLEND_MAX) && ((gUnk_03004C20.sceneFrameCounter % 2) != 0)) {
        gBlendValue += 1;
    }

    UpdatePaletteAnimations();

    if (gUnk_03004C08.unk1 == 0) {
        if (gUnk_03004C08.unk0_4 > 4) {
            if (gBgInfo[0].vOfs != 0) {
                gBgInfo[0].vOfs -= 0x80;
            }
        } else if (gBgInfo[0].vOfs < 0x400) {
            gBgInfo[0].vOfs += 0x80;
        }

        if (gNewKeys & (START_BUTTON | A_BUTTON)) {
            gBlendValue = 0;
            gUnk_03004C20.world = gUnk_03004C08.unk0_4 + 1;
            gUnk_03004C20.level = 0;
            gUnk_030034B0.unk6_4 = 1;
            m4aSongNumStart(0x52);
            gUnk_03005284->unk1E = gUnk_03005284->unk0 = gUnk_03005220.lives;

            if (gUnk_03004C08.unk0_4 > 4) {
                gUnk_03004C20.room = 0;
                if (gUnk_03004C08.unk0_4 == 5) {
                    gUnk_03004C20.world = 6;
                    gUnk_03004C20.level = 1;
                } else if (gUnk_03004C08.unk0_4 == 6) {
                    gUnk_03004C20.world = 6;
                    gUnk_03004C20.level = 2;
                } else if (gUnk_03004C08.unk0_4 == 7) {
                    gUnk_03004C20.world = 6;
                    gUnk_03004C20.level = 3;
                }
                gCallbackQueue.current[1] = TransitionClearAndRestart;
            } else {
                gUnk_03005284->unk4 = (gUnk_03004C20.world * 3) - 1;
                gUnk_03003410.unkC = 0;
                gCallbackQueue.current[1] = TransitionReturnToWorldMap;
            }
            return;
        }

        switch (gUnk_03004C08.unk0_4) {
            case 0:
                if ((gHeldKeys & (DPAD_DOWN | DPAD_RIGHT)) && (CheckWorldCompletion(gUnk_03004C08.unk0_4) != 0)) {
                    gUnk_03004C08.unk1 = 1;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 1);
                }

                if ((gHeldKeys & DPAD_UP) && (CheckWorldCompletion(6) != 0)) {
                    gUnk_03004C08.unk1 = 7;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x25);

                    gBgTilemapBufs[0][0x4C] = 0xA00C;
                    gBgTilemapBufs[0][0x4D] = 0xA00D;
                    gBgTilemapBufs[0][0x4E] = 0xA00E;
                    gBgTilemapBufs[0][0x4F] = 0xA00F;
                    gBgTilemapBufs[0][0x50] = 0xA010;

                    gBgTilemapBufs[0][0x6D] = 0xA000 | ((gUnk_03004670->unk4 / 10) + 1);
                    gBgTilemapBufs[0][0x6E] = 0xA000 | ((gUnk_03004670->unk4 % 10) + 1);
                    gBgTilemapBufs[0][0x6F] = 0xA00B;
                    gBgTilemapBufs[0][0x70] = 0xA000 | ((gUnk_03004670->unk5 / 10) + 1);
                    gBgTilemapBufs[0][0x71] = 0xA000 | ((gUnk_03004670->unk5 % 10) + 1);
                    gBgTilemapBufs[0][0x72] = 0xA00B;
                    gBgTilemapBufs[0][0x73] = 0xA000 | ((gUnk_03004670->unk6 / 10) + 1);
                    gBgTilemapBufs[0][0x74] = 0xA000 | ((gUnk_03004670->unk6 % 10) + 1);

                    gBgTilemapBufs[0][0x2C] = 0xA000;
                    gBgTilemapBufs[0][0x30] = 0xA000;
                    gBgTilemapBufs[0][0x31] = 0xA000;
                    gBgTilemapBufs[0][0x32] = 0xA000;

                    gBgTilemapBufs[0][0x2E] = 0xA000;
                    gBgTilemapBufs[0][0x2F] = 0xA000;
                }
                break;

            case 1:
                if ((gHeldKeys & DPAD_UP) && (CheckWorldCompletion(gUnk_03004C08.unk0_4) != 0)) {
                    gUnk_03004C08.unk1 = 1;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x25);
                }

                if (gHeldKeys & DPAD_LEFT) {
                    gUnk_03004C08.unk1 = 0xFF;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 1);
                }
                break;

            case 2:
                if ((gHeldKeys & DPAD_RIGHT) && (CheckWorldCompletion(gUnk_03004C08.unk0_4) != 0)) {
                    gUnk_03004C08.unk1 = 1;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 1);
                }

                if (gHeldKeys & DPAD_DOWN) {
                    gUnk_03004C08.unk1 = 0xFF;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x24);
                }
                break;

            case 3:
                if ((gHeldKeys & (DPAD_DOWN | DPAD_RIGHT)) && (CheckWorldCompletion(gUnk_03004C08.unk0_4) != 0)) {
                    gUnk_03004C08.unk1 = 1;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 1);
                }

                if (gHeldKeys & DPAD_LEFT) {
                    gUnk_03004C08.unk1 = 0xFF;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 1);
                }
                break;

            case 4:
                if ((gHeldKeys & DPAD_UP) && (CheckWorldCompletion(gUnk_03004C08.unk0_4 + 1) != 0)) {
                    gUnk_03004C08.unk1 = 2;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x25);

                    gBgTilemapBufs[0][0x4C] = 0xA000;
                    gBgTilemapBufs[0][0x4D] = 0xA000;
                    gBgTilemapBufs[0][0x4E] = 0xA000;
                    gBgTilemapBufs[0][0x4F] = 0xA000;
                    gBgTilemapBufs[0][0x50] = 0xA000;

                    gBgTilemapBufs[0][0x6D] = 0xA000;
                    gBgTilemapBufs[0][0x6E] = 0xA000;
                    gBgTilemapBufs[0][0x6F] = 0xA000;
                    gBgTilemapBufs[0][0x70] = 0xA000;
                    gBgTilemapBufs[0][0x71] = 0xA000;
                    gBgTilemapBufs[0][0x72] = 0xA000;
                    gBgTilemapBufs[0][0x73] = 0xA000;
                    gBgTilemapBufs[0][0x74] = 0xA000;

                    gBgTilemapBufs[0][0x2C] = 0xA011;
                    gBgTilemapBufs[0][0x30] = 0xA010;
                    gBgTilemapBufs[0][0x31] = 0xA004;
                    gBgTilemapBufs[0][0x32] = 0xA001;

                    gBgTilemapBufs[0][0x2E] = 0xA000 | (((gUnk_03004670->unk8[5][1] & 0x7F) / 10) + 1);
                    gBgTilemapBufs[0][0x2F] = 0xA000 | (((gUnk_03004670->unk8[5][1] & 0x7F) % 10) + 1);
                }

                if ((gHeldKeys & DPAD_DOWN) && (CheckWorldCompletion(gUnk_03004C08.unk0_4) != 0)) {
                    gUnk_03004C08.unk1 = 1;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x24);

                    gBgTilemapBufs[0][0x4C] = 0xA00C;
                    gBgTilemapBufs[0][0x4D] = 0xA00D;
                    gBgTilemapBufs[0][0x4E] = 0xA00E;
                    gBgTilemapBufs[0][0x4F] = 0xA00F;
                    gBgTilemapBufs[0][0x50] = 0xA010;

                    gBgTilemapBufs[0][0x6D] = 0xA000 | ((gUnk_03004670->unk1 / 10) + 1);
                    gBgTilemapBufs[0][0x6E] = 0xA000 | ((gUnk_03004670->unk1 % 10) + 1);
                    gBgTilemapBufs[0][0x6F] = 0xA00B;
                    gBgTilemapBufs[0][0x70] = 0xA000 | ((gUnk_03004670->unk2 / 10) + 1);
                    gBgTilemapBufs[0][0x71] = 0xA000 | ((gUnk_03004670->unk2 % 10) + 1);
                    gBgTilemapBufs[0][0x72] = 0xA00B;
                    gBgTilemapBufs[0][0x73] = 0xA000 | ((gUnk_03004670->unk3 / 10) + 1);
                    gBgTilemapBufs[0][0x74] = 0xA000 | ((gUnk_03004670->unk3 % 10) + 1);

                    gBgTilemapBufs[0][0x2C] = 0xA011;
                    gBgTilemapBufs[0][0x30] = 0xA010;
                    gBgTilemapBufs[0][0x31] = 0xA004;
                    gBgTilemapBufs[0][0x32] = 0xA001;

                    gBgTilemapBufs[0][0x2E] = 0xA000 | (((gUnk_03004670->unk8[5][0] & 0x7F) / 10) + 1);
                    gBgTilemapBufs[0][0x2F] = 0xA000 | (((gUnk_03004670->unk8[5][0] & 0x7F) % 10) + 1);
                }

                if (gHeldKeys & DPAD_LEFT) {
                    gUnk_03004C08.unk1 = 0xFF;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 1);
                }
                break;

            case 5:
                if (gHeldKeys & (DPAD_UP | DPAD_RIGHT)) {
                    gUnk_03004C08.unk1 = 0xFF;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x25);
                }
                break;

            case 6:
                if (gHeldKeys & DPAD_DOWN) {
                    gUnk_03004C08.unk1 = 0xFE;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x24);
                }
                break;

            case 7:
                if (gHeldKeys & DPAD_DOWN) {
                    gUnk_03004C08.unk1 = 0xF9;
                    m4aSongNumStart(0x51);
                    SetPaletteAnimEntry(0, 0x24);
                }
                break;
        }

        if (gUnk_03004C08.unk1 != 0) {
            if (gUnk_08116708[gUnk_03004C08.unk0_4][0] >= gUnk_08116708[gUnk_03004C08.unk0_4 + gUnk_03004C08.unk1][0]) {
                gEntityInfo[0].unkC_2 = 1;
            } else {
                gEntityInfo[0].unkC_2 = 0;
            }
            gUnk_030034DC = 0;
        }
    } else {
        sp0.unk0 = gEntityInfo[0].xPosBg2;
        sp0.unk2 = gEntityInfo[0].yPosBg2;
        sp0.unk4 = gUnk_08116708[gUnk_03004C08.unk0_4 + gUnk_03004C08.unk1][0];
        sp0.unk6 = gUnk_08116708[gUnk_03004C08.unk0_4 + gUnk_03004C08.unk1][1];
        sp0.unk8 = sp0.unk9 = 2;
        UpdateTextScroll(&spC, sp0);
        gEntityInfo[0].xPosBg2 = spC;
        gEntityInfo[0].yPosBg2 = spC >> 0x10;

        if ((gUnk_03004C08.unk0_4 + gUnk_03004C08.unk1) > 4) {
            if (gBgInfo[0].vOfs != 0) {
                gBgInfo[0].vOfs -= 0x80;
            }
        } else if (gBgInfo[0].vOfs < 0x400) {
            gBgInfo[0].vOfs += 0x80;
        }

        if ((gEntityInfo[0].xPosBg2 == gUnk_08116708[gUnk_03004C08.unk0_4 + gUnk_03004C08.unk1][0])
            && (gEntityInfo[0].yPosBg2 == gUnk_08116708[gUnk_03004C08.unk0_4 + gUnk_03004C08.unk1][1])) {
            gUnk_03004C08.unk0_4 += gUnk_03004C08.unk1;
            gUnk_03004C08.unk1 = 0;
            if (gEntityAnimationInfo[0].state == 1) {
                SetPaletteAnimEntry(0, 0);
            }
            if (gEntityAnimationInfo[0].state == 0x25) {
                SetPaletteAnimEntry(0, 0x23);
            }
            if (gEntityAnimationInfo[0].state == 0x24) {
                SetPaletteAnimEntry(0, 0x22);
            }
        }

        for (var_r4 = 0; var_r4 < 8; var_r4++) {
            if ((gEntityInfo[0].xPosBg2 >= (gUnk_08116708[var_r4][0] - 0x10))
                && (gEntityInfo[0].xPosBg2 <= (gUnk_08116708[var_r4][0] + 0x10))
                && (gEntityInfo[0].yPosBg2 >= (gUnk_08116708[var_r4][1] - 0x10))
                && (gEntityInfo[0].yPosBg2 <= (gUnk_08116708[var_r4][1] + 0x10))) {
                REG_WIN0H = gUnk_08116728[var_r4][0];
                REG_WIN0V = gUnk_08116728[var_r4][1];
                break;
            }
        }
    }
}
/**
 * InitLevelState: resets the per-level game state at level start — clears the gUnk_03005400 status flags, initializes the world/level
 * lookup pointers, and seeds the level-config counters (gUnk_03005440) for the current world/level.
 */
void InitLevelState(void) {
    u32 var_r4_4;
    u32 var_r3;
    u32 var_r4;

    gUnk_030007CC = gUnk_081168DC[gUnk_03004C20.world - 1];

    gUnk_03005400.unk8_6 = 0;
    gUnk_03005400.unk8_0 = 0;
    gUnk_03005400.unk8_1 = 0;
    gUnk_03005400.unk8_2 = 0;
    gUnk_03005400.unk8_3 = 0;
    gUnk_03005400.unk8_4 = 0;

    gUnk_03005400.unk9 = 0;

    gUnk_03005400.unkE_7 = 0;
    gUnk_03005400.unkE_3 = 0;
    gUnk_03005400.unkE_4 = 0;
    gUnk_03005400.unkE_0 = 0;
    gUnk_03005400.unkE_1 = 0;

    gUnk_03005400.unkA = 0;
    gUnk_03005400.unkB = 0;
    gUnk_03005400.unk13 = 0;
    gUnk_03005400.unk14 = 0;
    gUnk_03005400.unk15 = 0;
    gUnk_03005400.unk16 = 0;
    gUnk_03005400.unkC = 3;
    gUnk_03005400.unkD = 0;
    gUnk_03005400.unk0 = 0;
    gUnk_03005400.unkF = 0;
    gUnk_03005400.unk10 = 0;
    gUnk_03005400.unk11 = 0;
    gUnk_03005400.unk12 = 0;
    gUnk_03005400.unk2 = 0;

    REG_IE &= ~INTR_FLAG_VBLANK;
    REG_DISPSTAT &= ~DISPSTAT_VBLANK_INTR;
    REG_IE &= ~INTR_FLAG_HBLANK;
    REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
    m4aSoundVSyncOff();
    m4aMPlayAllStop();

    DmaFill16(3, 0, &gUnk_03003590, 0x80);

    for (var_r4 = 0; var_r4 < 0xB; var_r4++) {
        gEntityInfo[var_r4].affineEnable = 1;
        gEntityInfo[var_r4].affineHFlip_matrixNum = 0;
        gEntityInfo[var_r4].priority = 1;
    }

    for (var_r4 = 0xD; var_r4 < gUnk_03005428; var_r4++) {
        gEntityInfo[var_r4].affineEnable = 1;
        gEntityInfo[var_r4].affineHFlip_matrixNum = 0;

        switch (gEntityInfo[var_r4].unk11) {
            case 0x0:
                gEntityInfo[var_r4].affineEnable = 0;
                gEntityInfo[var_r4].priority = 0;
                break;

            case 0x20:
                gEntityInfo[var_r4].affineEnable = 0;
                gEntityInfo[var_r4].priority = 0;
                break;

            case 0xB:
            case 0x16:
            case 0x76:
            case 0x77:
            case 0x78:
            case 0x79:
            case 0x7A:
            case 0x7B:
            case 0x7C:
            case 0x7D:
                gEntityInfo[var_r4].priority = 0;
                break;

            case 0x36:
                gEntityInfo[var_r4].priority = 2;
                break;

            default:
                gEntityInfo[var_r4].priority = 1;
                break;
        }

        if ((gEntityInfo[var_r4].unk11 == 0) && (gEntityInfo[var_r4].unkF != 0x1C)) {
            gEntityInfo[var_r4].unk10 = 1;
        }
    }

    switch (gUnk_03004C20.world - 1) {
        case 0:
            gUnk_030034A8 = UpdatePlayerNormal;
            gUnk_03005288 = 5;
            break;

        case 1:
            gUnk_030034A8 = UpdatePlayerBoss;
            gUnk_03005288 = 5;

            gEntityInfo[0x1A].priority = 1;
            gEntityInfo[0x19].priority = 1;
            gEntityInfo[0x18].priority = 1;
            gEntityInfo[0x17].priority = 1;

            gEntityInfo[0x1A].affineEnable = 1;
            gEntityInfo[0x19].affineEnable = 1;
            gEntityInfo[0x18].affineEnable = 1;
            gEntityInfo[0x17].affineEnable = 1;

            gEntityInfo[0x19].unkC_2 = 1;
            gEntityInfo[0x17].unkC_2 = 1;
            gEntityInfo[0x19].affineHFlip_matrixNum = 1;
            gEntityInfo[0x17].affineHFlip_matrixNum = 1;

            gEntityInfo[0x1A].unkC_2 = 0;
            gEntityInfo[0x18].unkC_2 = 0;
            gEntityInfo[0x1A].affineHFlip_matrixNum = 0;
            gEntityInfo[0x18].affineHFlip_matrixNum = 0;
            break;

        case 2:
            gUnk_030034A8 = UpdatePlayerMinigame;
            gUnk_03005288 = 0xF;

            for (var_r4 = 0; var_r4 < gUnk_03005428; var_r4++) {
                gEntityInfo[var_r4].priority = 0;
            }

            REG_BG0CNT = BGCNT_PRIORITY(3) | gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][0] | 0x1C40;
            REG_BG1CNT = BGCNT_PRIORITY(1) | gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][1] | 0x1D44;
            REG_BG2CNT = BGCNT_PRIORITY(0) | gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][2] | 0x9E48;

            ConfigureEntityBehavior(0, 0, 0);
            ConfigureEntityBehavior(1, 0, 0);
            ConfigureEntityBehavior(2, 0, 0);
            ConfigureEntityBehavior(3, 0, 0);
            ConfigureEntityBehavior(4, 0, 0);
            ConfigureEntityBehavior(5, 0, 0);
            break;

        case 3:
            gUnk_03005288 = 0xF;
            gUnk_030034A8 = UpdatePlayerAlternate;

            REG_BG0CNT = BGCNT_PRIORITY(3) | gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][0] | 0x1C40;
            REG_BG1CNT = BGCNT_PRIORITY(0) | gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][1] | 0x1D44;
            REG_BG2CNT = BGCNT_PRIORITY(0) | gUnk_08051BD4[gUnk_03004C20.world - 1][gUnk_03004C20.level][2] | 0x9E49;

            gUnk_03005440.unkC = 0x30;
            gUnk_03005440.unkE = 0;
            gUnk_03005440.unk10 = 0x30;
            gUnk_03005440.unk12 = 0x200;
            gUnk_03005440.unk18 = 0x1B0;
            gUnk_03005440.unk1A = 0;
            gUnk_03005440.unk1C = 0x1B0;
            gUnk_03005440.unk1E = 0x200;
            break;

        case 4:
            gUnk_03005288 = 0xF;
            gUnk_030034A8 = UpdatePlayerSpecial;
            break;

        case 5:
            gUnk_03005288 = 0xF;
            gUnk_030034A8 = UpdatePlayerFinalBoss;
            gEntityInfo[0x12].unk8.split.unk9 = 0x10;

            gUnk_03005400.unkE_7 = 1;
            gUnk_030007E0.unkC_0 = 3;
            gUnk_030007E0.unkC_4 = 0;
            gUnk_030007E0.unk6 = 0x78;
            gUnk_030007E0.unk0 = 0x78;
            gUnk_030007E0.unk8 = 0x80;
            gUnk_030007E0.unk2 = 0x80;
            gUnk_030007E0.unkA = 0;
            break;
    }

    var_r4_4 = 0;
    for (var_r3 = 0; var_r3 < (gCallbackQueue.currentCount - 1); var_r3++) {
        if ((gCallbackQueue.current[var_r3] == InitLevelState) || (var_r4_4 == 1)) {
            gCallbackQueue.next[var_r3] = gCallbackQueue.current[var_r3 + 1];
            var_r4_4 = 1;
        } else {
            gCallbackQueue.next[var_r3] = gCallbackQueue.current[var_r3];
        }
    }
    if (var_r4_4 == 1) {
        gCallbackQueue.nextCount = gCallbackQueue.currentCount - 1;
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
    }

    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
    m4aSoundVSyncOn();
}
/**
 * UpdateEntitySpawnState: advances an entity's spawn state machine (gEntityInfo[i].unkF), positioning it or clearing it once it
 * scrolls past the per-room spawn threshold.
 */
void UpdateEntitySpawnState(u8 arg0) {
    switch (gEntityInfo[arg0].unkF) {
        case 24:
            gEntityInfo[arg0].xPosBg2 = 24;
            gEntityInfo[arg0].yPosBg2 = 24;
            gEntityInfo[arg0].priority = 0;
            gEntityInfo[arg0].unkF = 0x19;
            break;

        case 25:
            if (gEntityInfo[arg0].xPosScreen
                >= gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[gUnk_03004C20.room - 1].unk0) {
                gEntityInfo[arg0].unkF = 0;
            } else if ((gUnk_03004C20.sceneFrameCounter % 2) != 0) {
                gEntityInfo[arg0].xPosBg2 += 1;
            }
            break;

        case 17:
            if (gEntityInfo[arg0].xPosBg2
                <= (gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[gUnk_03004C20.room - 1].unk0
                    - 0x10)) {
                gEntityInfo[arg0].unk10 = 0;
                gEntityInfo[arg0].unkF = 0x1C;
            } else if ((gUnk_03004C20.sceneFrameCounter % 2) != 0) {
                gEntityInfo[arg0].xPosBg2 -= 1;
            }
            break;

        // Can be any case between 0 and 16, required to match
        case 0:
            break;
    }
}
/**
 * SpawnEntitiesForVision: initializes the entity table for the current vision — reads the per-level entity list from ROM,
 * positions/enables each entity, and seeds their palette-animation entries.
 */
void SpawnEntitiesForVision(u8 arg0) {
    u32 var_r4;
    u32 var_r6;
    u32 temp_r1;

    gEntityInfo[arg0].affineDouble = 1;
    if (gUnk_03005400.unkE_4 != 0) {
        temp_r1 = gUnk_0811710A[gUnk_03004C20.world - 1];
        for (var_r6 = 0; var_r6 < 2; var_r6++) {
            if ((gEntityInfo[temp_r1 + var_r6].unkF == 0) || (gEntityInfo[temp_r1 + var_r6].unkF == 0x19)) {
                SpawnEntityAtPosition(gEntityInfo[temp_r1 + var_r6].xPosBg2, gEntityInfo[temp_r1 + var_r6].yPosBg2, 2,
                                      temp_r1 + var_r6);
                gEntityInfo[temp_r1 + var_r6].unkF = 0x1C;
                gEntityInfo[temp_r1 + var_r6].unk10 = 0;
            }
        }

        if (gUnk_03005220.unk42 != 0) {
            gEntityInfo[gUnk_03005220.unk42].unkF = 0x1C;
            gEntityInfo[gUnk_03005220.unk42].unk10 = 0;
            gUnk_03005220.unk38 = 0;
            gUnk_03005220.unk43 = 0;
            gUnk_03005220.unk42 = 0;
            if (gEntityAnimationInfo[0].state >= 0x16) {
                SetPaletteAnimEntry(0, gEntityAnimationInfo[0].state - 0x16);
            }
        }

        gEntityInfo[0].priority = 0;
        gUnk_03005400.unkE_4 = 0;
        REG_BLDCNT = BLDCNT_TGT2_BG0 | BLDCNT_TGT2_BG1 | BLDCNT_TGT2_BG2 | BLDCNT_EFFECT_BLEND;
        gBlendValue = BLEND_MAX;

        for (var_r6 = 0x12; var_r6 < gUnk_03005428; var_r6++) {
            gEntityInfo[var_r6].objMode = 1;
        }
    } else {
        if ((gUnk_03004C20.globalFrameCounter % 8) == 0) {
            if (gBlendValue != 0) {
                gBlendValue -= 1;
            }
        }
        if (gBlendValue != 0) {
            return;
        }

        if ((gUnk_03005220.unk31 == 0) && (gUnk_03005220.unk35 == 0)) {
            return;
        }

        gEntityInfo[arg0].unkF = 4;
        gUnk_03003410.unkB = 2;

        for (var_r4 = 0; var_r4 < (gCallbackQueue.currentCount + 1); var_r4++) {
            if (var_r4 == 4) {
                gCallbackQueue.next[4] = IntroSequenceUpdate;
            } else if (var_r4 > 4) {
                gCallbackQueue.next[var_r4] = gCallbackQueue.current[var_r4 - 1];
            } else {
                gCallbackQueue.next[var_r4] = gCallbackQueue.current[var_r4];
            }
        }
        if (var_r4 > 3) {
            gCallbackQueue.nextCount = gCallbackQueue.currentCount + 1;
            gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        }

        gEntityInfo[0x9].unk10 = 0;
        gEntityInfo[0xA].unk10 = 0;
        if (gUnk_03005220.unk31 != 0) {
            SetPaletteAnimEntry(0, 0);
        } else if (gUnk_03005220.unk35 != 0) {
            SetPaletteAnimEntry(0, 0x10);
        }
    }
}
/**
 * GetEntityLookupData: loads entity parameters from ROM table into state.
 *
 * Indexes into the entity data table at 0x081168E8 with stride 8,
 * copies bytes at offsets +5 and +6 to the entity state block.
 */
void GetEntityLookupData(u8 idx) {
    u8 *flags = gGameFlagsPtr;
    const u8 *table = gEntityDataTable;
    const u8 *entry = &table[(u32)idx * 8];
    flags[0x11] = entry[5];
    flags[0x12] = entry[6];
}
/**
 * ComputeScrollLimits: recomputes the BG2 affine matrix (from magnification/angle) and the per-level camera scroll bounds.
 */
void ComputeScrollLimits(void) {
    u8 sp0;
    s16 temp_r4;
    s16 temp_r7;
    s16 temp_sb;
    s16 temp_sl;
    u8 var_r4;

    temp_sl = MultiplyQ8(COS(0), ReciprocalQ8(gBg2XMag));
    temp_sb = MultiplyQ8(SIN(0), ReciprocalQ8(gBg2XMag));
    temp_r7 = MultiplyQ8(-SIN(0), ReciprocalQ8(gBg2YMag));
    temp_r4 = MultiplyQ8(COS(0), ReciprocalQ8(gBg2YMag));

    gOamAffineMatrix[0].pa = temp_sl;
    gOamAffineMatrix[0].pb = temp_sb;
    gOamAffineMatrix[0].pc = temp_r7;
    gOamAffineMatrix[0].pd = temp_r4;

    gOamAffineMatrix[1].pa = -temp_sl;
    gOamAffineMatrix[1].pb = temp_sb;
    gOamAffineMatrix[1].pc = temp_r7;
    gOamAffineMatrix[1].pd = temp_r4;

    gOamAffineMatrix[2].pa = temp_sl;
    gOamAffineMatrix[2].pb = temp_sb;
    gOamAffineMatrix[2].pc = temp_r7;
    gOamAffineMatrix[2].pd = -temp_r4;

    temp_sl = MultiplyQ8(COS(gUnk_03003590[0].unk4), ReciprocalQ8(gBg2XMag + gUnk_03003590[0].unk0 + gUnk_030007CC));
    temp_sb = MultiplyQ8(SIN(gUnk_03003590[0].unk4), ReciprocalQ8(gBg2XMag + gUnk_03003590[0].unk0 + gUnk_030007CC));
    temp_r7 = MultiplyQ8(-SIN(gUnk_03003590[0].unk4), ReciprocalQ8(gBg2YMag + gUnk_03003590[0].unk2 + gUnk_030007CC));
    temp_r4 = MultiplyQ8(COS(gUnk_03003590[0].unk4), ReciprocalQ8(gBg2YMag + gUnk_03003590[0].unk2 + gUnk_030007CC));

    if (gUnk_03003590[0].unk5_0 == 0) {
        gOamAffineMatrix[3].pa = temp_sl;
        gOamAffineMatrix[3].pc = temp_r7;
    } else {
        gOamAffineMatrix[3].pa = -temp_sl;
        gOamAffineMatrix[3].pc = -temp_r7;
    }
    gOamAffineMatrix[3].pb = temp_sb;
    gOamAffineMatrix[3].pd = temp_r4;

    for (sp0 = 1; sp0 < 0x10; sp0++) {
        if (gUnk_03003590[sp0].unk5_0 == 0) {
            var_r4 = gUnk_03003590[sp0].unk4;
        } else {
            var_r4 = -gUnk_03003590[sp0].unk4;
        }

        temp_sl = MultiplyQ8(COS(var_r4), ReciprocalQ8(gBg2XMag + gUnk_03003590[sp0].unk0));
        temp_sb = MultiplyQ8(SIN(var_r4), ReciprocalQ8(gBg2XMag + gUnk_03003590[sp0].unk0));
        temp_r7 = MultiplyQ8(-SIN(var_r4), ReciprocalQ8(gBg2YMag + gUnk_03003590[sp0].unk2));
        temp_r4 = MultiplyQ8(COS(var_r4), ReciprocalQ8(gBg2YMag + gUnk_03003590[sp0].unk2));

        if (gUnk_03003590[sp0].unk5_0 == 0) {
            gOamAffineMatrix[sp0 + 3].pa = temp_sl;
        } else {
            gOamAffineMatrix[sp0 + 3].pa = -temp_sl;
        }
        if ((gUnk_03003590[sp0].unk5_0 != 0) && (gUnk_03003590[sp0].unk4 != 0)) {
            gOamAffineMatrix[sp0 + 3].pc = -temp_r7;
        } else {
            gOamAffineMatrix[sp0 + 3].pc = temp_r7;
        }
        gOamAffineMatrix[sp0 + 3].pb = temp_sb;
        gOamAffineMatrix[sp0 + 3].pd = temp_r4;
    }
}
/**
 * x
 */
void ApplyPlayerMovement(u8 arg0, struct Unk_0803D4AC arg1) {
    struct Unk_08014184 temp_r0;
    struct Unk_08014184 temp_r0_1;
    u16 sp10;
    u16 temp_sl;
    u32 var_r3;

    sp10 = gEntityInfo[arg0].xPosBg2;
    temp_sl = gEntityInfo[arg0].yPosBg2;

    if (gUnk_03005400.unk8_4) {
        gUnk_03005400.unk10 += 2;
        if ((gUnk_03005400.unk10 >> 4) > 2) {
            gUnk_03005400.unk10 = 0x20;
        }
        gEntityInfo[arg0].yPosBg2 += (gUnk_03005400.unk10 >> 0x4);

        if (gUnk_03004C20.unkA == 0) {
            var_r3 = gBgDataPtrs.pBufBg2Tilemap[(gEntityInfo[arg0].xPosBg2 >> 3)
                                                + (((gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1]) >> 3)
                                                   * gBgInfo[2].hLength)];
        } else {
            temp_r0 = Call_CheckTileCollisionSloped(gEntityInfo[arg0].xPosBg2,
                                                    gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1], 0x18);
            if (temp_r0.unk0 != 0xFFFF) {
                var_r3 = gUnk_03004654[0x1B];
            } else {
                var_r3 = gBgDataPtrs.pBufBg2Tilemap[(gEntityInfo[arg0].xPosBg2 >> 3)
                                                    + (((gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1]) >> 3)
                                                       * gBgInfo[2].hLength)];
            }
        }

        if (gUnk_03004654[0x1B] <= var_r3) {
            gEntityInfo[arg0].yPosBg2 = temp_sl;
            gUnk_03005400.unk8_2 = 1;
        } else {
            gUnk_03005400.unk8_2 = 0;
        }
    }

    if (arg1.unk4 != 0) {
        temp_sl = gEntityInfo[arg0].yPosBg2;
        if ((gUnk_03004C20.globalFrameCounter & arg1.unk2) == arg1.unk2) {
            gUnk_03005400.unk10 += gUnk_03005400.unk12;
            if ((gUnk_03005400.unk10 >> 4) > arg1.unk4) {
                gUnk_03005400.unk10 = arg1.unk4 << 4;
            }
            gEntityInfo[arg0].yPosBg2 += (gUnk_03005400.unk10 >> 4);
        }

        if (gUnk_03004C20.unkA == 0) {
            var_r3 = gBgDataPtrs.pBufBg2Tilemap[(gEntityInfo[arg0].xPosBg2 >> 3)
                                                + (((gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1]) >> 3)
                                                   * gBgInfo[2].hLength)];
        } else {
            temp_r0_1 = Call_CheckTileCollisionSloped(gEntityInfo[arg0].xPosBg2 + 8,
                                                      gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1], 0x18);
            if (temp_r0_1.unk0 != 0xFFFF) {
                var_r3 = gUnk_03004654[0x1B];
            } else {
                var_r3 = gBgDataPtrs.pBufBg2Tilemap[(gEntityInfo[arg0].xPosBg2 >> 3)
                                                    + (((gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1]) >> 3)
                                                       * gBgInfo[2].hLength)];
            }
        }

        if (gUnk_03004654[0x1B] <= var_r3) {
            gEntityInfo[arg0].yPosBg2 = temp_sl & 0xFFF8;
            gUnk_03005400.unk8_2 = 1;
        } else {
            gUnk_03005400.unk8_2 = 0;
        }
    }

    var_r3 = 0;
    if ((gUnk_03004C20.globalFrameCounter & arg1.unk1) == arg1.unk1) {
        if (gEntityInfo[arg0].unkC_2 == 0) {
            gUnk_03005400.unkF += gUnk_03005400.unk11;
            if ((gUnk_03005400.unkF >> 4) > arg1.unk3) {
                gUnk_03005400.unkF = arg1.unk3 << 4;
            }
            gEntityInfo[arg0].xPosBg2 += (gUnk_03005400.unkF >> 4);

            var_r3 = gBgDataPtrs.pBufBg2Tilemap[((gEntityInfo[arg0].xPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][0]) >> 3)
                                                + (((gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1]) >> 3)
                                                   * gBgInfo[2].hLength)];
            if ((gEntityInfo[arg0].xPosBg2 > 0x158) && (gUnk_03004C20.world == 1)) {
                var_r3 = gUnk_03004654[0x1B];
            }
        } else {
            gUnk_03005400.unkF += gUnk_03005400.unk11;
            if ((gUnk_03005400.unkF >> 4) > arg1.unk3) {
                gUnk_03005400.unkF = arg1.unk3 << 4;
            }
            gEntityInfo[arg0].xPosBg2 -= (gUnk_03005400.unkF >> 4);

            var_r3 = gBgDataPtrs.pBufBg2Tilemap[((gEntityInfo[arg0].xPosBg2 - gUnk_08116888[gUnk_03004C20.world - 1][0]) >> 3)
                                                + (((gEntityInfo[arg0].yPosBg2 + gUnk_08116888[gUnk_03004C20.world - 1][1]) >> 3)
                                                   * gBgInfo[2].hLength)];
            if ((gEntityInfo[arg0].xPosBg2 <= 0x4FU) && (gUnk_03004C20.world == 1)) {
                var_r3 = gUnk_03004654[0x1B];
            }
        }
    }

    if (gUnk_03004654[0x1B] <= var_r3) {
        gEntityInfo[arg0].xPosBg2 = sp10;
        gUnk_03005400.unk8_3 = 1;
    } else {
        gUnk_03005400.unk8_3 = 0;
    }
}
/**
 * UpdatePlayerNormal: per-frame update of the player in normal gameplay.
 */
void UpdatePlayerNormal(u8 arg0) {
    s32 var_r0_3;
    u8 var_r2;
    s32 var_r9;
    u8 temp_r3_4;

    do {
    } while (0); // Required to match
    gEntityInfo[arg0].affineHFlip_matrixNum = 3;
    gEntityInfo[arg0].affineDouble = 1;
    gUnk_03003590[0].unk5_0 = gEntityInfo[0x12].unkC_2;
    if (gUnk_03005400.unk0 != 0) {
        gUnk_03005400.unk0 -= 1;
    }
    gUnk_03003620 = gUnk_081168E8[gUnk_03005400.unk9];
    ApplyPlayerMovement(arg0, gUnk_03003620);

    if (gEntityInfo[arg0].unkF == 14) {
        switch (gUnk_03005400.unkA) {
            case 0:
                gUnk_030007E0.unkC_0 = 5;
                gUnk_030007E0.unkC_4 = 1;
                gUnk_030007E0.unk6 = gEntityInfo[0].xPosBg2;
                gUnk_030007E0.unk0 = gEntityInfo[0].xPosBg2;
                gUnk_030007E0.unk8 = gEntityInfo[0].yPosBg2;
                gUnk_030007E0.unk2 = gEntityInfo[0].yPosBg2;
                gUnk_030007E0.unkA = 0;
                gUnk_030007E0.unk4 = 0;

                gEntityInfo[0].xPosBg2 = 0;
                gEntityInfo[0].yPosBg2 = 0x12A;
                SetPaletteAnimEntry(0, 1);
                SetPaletteAnimEntry(arg0, 0);
                gUnk_03005400.unk8_0 = 0;
                gUnk_03005400.unkA = 1;
                break;

            case 1:
                gEntityInfo[0].xPosBg2 += 2;
                if (gEntityInfo[0].xPosBg2 > 0x77) {
                    gEntityInfo[0x1A].unkF = 0x1C;
                    SetPaletteAnimEntry(0, 2);
                    gUnk_03005400.unkA = 2;
                }
                break;

            case 2:
                gEntityInfo[0].xPosBg2 += 1;
                if (gEntityInfo[0].yPosBg2 > 0xD1) {
                    gEntityInfo[0].yPosBg2 -= 2;
                } else {
                    gEntityInfo[0].yPosBg2 -= 1;
                }
                if (gEntityInfo[0].yPosBg2 <= 0xF0) {
                    SetPaletteAnimEntry(0, 3);
                    gUnk_03005400.unkA = 3;
                    gEntityInfo[0x1A].unkF = 0;
                }
                break;

            case 3:
                gEntityInfo[0].xPosBg2 += 1;
                gEntityInfo[0].yPosBg2 += 1;
                if (gEntityInfo[0].yPosBg2 > 0x0111) {
                    m4aSongNumStart(0x2E);
                    m4aSongNumStart(0x64);
                    SetPaletteAnimEntry(arg0, 0xC);
                    gUnk_03005400.unkE_7 = 1;
                    gUnk_03005400.unkA = 4;
                    gUnk_03005400.unk0 = 0xA0;
                }
                break;

            case 4:
                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unk8_4 = 1;
                    gUnk_03005400.unkA = 0;
                    gEntityInfo[arg0].unkF = 0;
                    gEntityInfo[0x13].unkF = 0x19;
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 26) {
        gEntityInfo[arg0].unkF = 0;
    } else if (gEntityInfo[arg0].unkF == 0) {
        switch (gUnk_03005400.unkA) {
            case 1:
                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unk0 = 0x3C;
                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 += 0x10;
                    } else {
                        gEntityInfo[arg0].xPosBg2 -= 0x10;
                    }

                    if (gEntityInfo[arg0].xPosBg2 > (0x190 - gUnk_08116888[gUnk_03004C20.world - 1][0])) {
                        gEntityInfo[arg0].xPosBg2 = 0x190 - gUnk_08116888[gUnk_03004C20.world - 1][0];
                    }
                    if (gEntityInfo[arg0].xPosBg2 < (gUnk_08116888[gUnk_03004C20.world - 1][0] + 0x40)) {
                        gEntityInfo[arg0].xPosBg2 = gUnk_08116888[gUnk_03004C20.world - 1][0] + 0x40;
                    }
                    gUnk_03005400.unk8_0 = 1;
                }

                if (gUnk_03005400.unk0 == 1) {
                    DmaCopy16Wait(
                        3, &gUnk_08078508,
                        OBJ_PLTT + gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20,
                        0x20);
                    if (gUnk_03005400.unkC == 0) {
                        SpawnEntityAtPosition(gEntityInfo[0x13].xPosBg2, gEntityInfo[0x13].yPosBg2, 2, 0x13);
                        gEntityInfo[0x13].unkF = 0x1C;
                        gEntityInfo[0x14].unkF = 0x1C;

                        gUnk_03005400.unkA = 0xC;
                        gUnk_03005400.unk0 = 0x100;
                        gUnk_03005400.unk8_0 = 1;
                        gUnk_03005400.unk8_6 = 1;
                    } else {
                        gUnk_03005400.unkA = 5;
                    }
                } else {
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 5) {
                        DmaCopy16Wait(
                            3, &gUnk_08078508,
                            OBJ_PLTT
                                + gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20,
                            0x20);
                    }
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 0) {
                        DmaFill16(3, 0xFFFF,
                                  OBJ_PLTT
                                      + gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                          * 0x20,
                                  0x20);
                    }
                }
                break;

            case 14:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    if (gUnk_03005400.unk8_2) {
                        gUnk_03005400.unkA = 9;
                    }
                } else if ((gEntityAnimationInfo[arg0 - gUnk_0300363C].timer > 0x10)
                           || (gEntityAnimationInfo[arg0 - gUnk_0300363C].frame <= 2)) {
                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 -= 1;
                    } else {
                        gEntityInfo[arg0].xPosBg2 += 1;
                    }

                    if (gEntityInfo[arg0].xPosBg2 > (0x190 - gUnk_08116888[gUnk_03004C20.world - 1][0])) {
                        gEntityInfo[arg0].xPosBg2 = 0x190 - gUnk_08116888[gUnk_03004C20.world - 1][0];
                    }
                    if (gEntityInfo[arg0].xPosBg2 < (gUnk_08116888[gUnk_03004C20.world - 1][0] + 0x40)) {
                        gEntityInfo[arg0].xPosBg2 = gUnk_08116888[gUnk_03004C20.world - 1][0] + 0x40;
                    }
                }
                break;

            case 10:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 1) {
                    SetPaletteAnimEntry(arg0, 1);
                    gUnk_03005400.unk9 = 0;
                    GetEntityLookupData(0);
                    gUnk_03005400.unk0 = gUnk_081169F9[gUnk_03005400.unkC - 1][1];
                    gUnk_03005400.unkE_0 = 1;
                    gUnk_03005400.unkD = 3;
                } else if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unkA = 2;
                }
                break;

            case 11:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_03005400.unkA = gUnk_03005400.unkB;
                }
                break;

            case 9:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 4) {
                    SetPaletteAnimEntry(arg0, 4);
                } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    if (((gEntityInfo[0].xPosBg2 < gEntityInfo[arg0].xPosBg2) && (gEntityInfo[arg0].unkC_2 == 0))
                        || ((gEntityInfo[0].xPosBg2 > gEntityInfo[arg0].xPosBg2) && (gEntityInfo[arg0].unkC_2 == 1))) {
                        gUnk_03005400.unkA = 2;
                    } else {
                        gUnk_03005400.unkA = 0;
                    }
                }
                break;

            case 2:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0xE) {
                    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0xF) {
                        SetPaletteAnimEntry(arg0, 0xE);
                    } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                        gUnk_03005400.unkA = 0;
                    }
                } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gEntityInfo[arg0].unkC_2 ^= 1;
                    SetPaletteAnimEntry(arg0, 0xF);
                }
                break;

            case 0:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0) {
                    SetPaletteAnimEntry(arg0, 0);
                    gUnk_03005400.unk0 = 0x20;
                }
                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unkA = 0xD;
                }
                break;

            case 13:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0xA) {
                    SetPaletteAnimEntry(arg0, 0xA);
                    gUnk_03005400.unk0 = gUnk_081169F9[gUnk_03005400.unkC - 1][0];
                }

                if (gUnk_03005400.unk0 != 0) {
                    break;
                }

                var_r0_3 = Abs(gEntityInfo[arg0].xPosBg2 - gEntityInfo[0].xPosBg2);
                if (var_r0_3 < 0) {
                    var_r0_3 += 7;
                }
                temp_r3_4 = var_r0_3 >> 3;

                if ((temp_r3_4 > gUnk_081169F9[gUnk_03005400.unkC - 1][2]) && (gUnk_03005220.unk43 != 0)
                    && (gUnk_03005400.unk14 != 3)) {
                    gUnk_03005400.unkA = 3;
                    gUnk_03005400.unk14 = 3;
                } else {
                    gUnk_03005400.unkA = 4;
                    gUnk_03005400.unk13 = temp_r3_4 * 4;
                    gUnk_03005400.unk6 = gEntityInfo[arg0].yPosBg2;

                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gUnk_03005400.unk4 = gEntityInfo[arg0].xPosBg2 + (temp_r3_4 * 4);
                        gEntityInfo[arg0].unk8.split.unk8 = 0x80;
                    } else {
                        gUnk_03005400.unk4 = gEntityInfo[arg0].xPosBg2 - (temp_r3_4 * 4);
                        gEntityInfo[arg0].unk8.split.unk8 = 0;
                    }
                }
                break;

            case 3:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 6) {
                    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 5) {
                        SetPaletteAnimEntry(arg0, 6);
                        break;
                    }

                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 += 1;
                        if ((gUnk_03004C20.sceneFrameCounter % 2) != 0) {
                            gEntityInfo[arg0].xPosBg2 += 1;
                        }
                    } else {
                        gEntityInfo[arg0].xPosBg2 -= 1;
                        if ((gUnk_03004C20.sceneFrameCounter % 2) != 0) {
                            gEntityInfo[arg0].xPosBg2 -= 1;
                        }
                    }

                    if ((gEntityInfo[arg0].xPosBg2 >= (gUnk_08116888[gUnk_03004C20.world - 1][0] + 0x40))
                        && (gEntityInfo[arg0].xPosBg2 <= (0x190 - gUnk_08116888[gUnk_03004C20.world - 1][0]))) {
                        break;
                    }

                    m4aSongNumStart(0x43);
                    if (gUnk_03005400.unkC == 1) {
                        gUnk_03005400.unkA = 5;
                    } else {
                        gUnk_03005400.unkA = 0xA;
                    }
                    break;
                }

                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    SetPaletteAnimEntry(arg0, 5);
                }
                break;

            case 4:
                switch (gEntityAnimationInfo[arg0 - gUnk_0300363C].state) {
                    default:
                        SetPaletteAnimEntry(arg0, 7);
                        gUnk_03005400.unk8_4 = 1;
                        break;

                    case 8:
                        var_r9 = gEntityInfo[arg0].xPosBg2;
                        if (((u32)(u8)(gEntityInfo[arg0].unk8.split.unk8 - 0x31) <= 0x2E)
                            || ((u32)(u8)(gEntityInfo[arg0].unk8.split.unk8 + 0x4F) <= 0x2E)) {
                            var_r2 = 1;
                        } else {
                            var_r2 = 2;
                        }
                        if (gEntityInfo[arg0].unkC_2 == 0) {
                            gEntityInfo[arg0].unk8.split.unk8 += var_r2;
                        } else {
                            gEntityInfo[arg0].unk8.split.unk8 -= var_r2;
                        }

                        gEntityInfo[arg0].xPosBg2
                            = ((gUnk_03005400.unk13 * COS(gEntityInfo[arg0].unk8.split.unk8)) >> 8) + gUnk_03005400.unk4;
                        gEntityInfo[arg0].yPosBg2 = ((SIN(gEntityInfo[arg0].unk8.split.unk8) * 5) >> 4) + gUnk_03005400.unk6;
                        if ((gEntityInfo[arg0].xPosBg2 < (gUnk_08116888[gUnk_03004C20.world - 1][0] + 0x40))
                            || (gEntityInfo[arg0].xPosBg2 > (0x190 - gUnk_08116888[gUnk_03004C20.world - 1][0]))) {
                            gEntityInfo[arg0].xPosBg2 = var_r9;
                        }

                        if ((gEntityInfo[arg0].unk8.split.unk8 != 0) && (gEntityInfo[arg0].unk8.split.unk8 != 0x80)) {
                            return;
                        }

                        SetPaletteAnimEntry(arg0, 9);
                        m4aSongNumStart(0x43);
                        gUnk_03005400.unkE_1 = 1;
                        gUnk_03005400.unkD = 2;
                        gUnk_03005400.unkB = 9;
                        gUnk_03005400.unkA = 0xB;
                        gUnk_03005400.unk14 = 0;

                        gEntityInfo[0x20].unkF = 3;
                        gEntityInfo[0x1F].unkF = 3;
                        gEntityInfo[0x1E].unkF = 3;
                        gEntityInfo[0x1D].unkF = 3;
                        gEntityInfo[0x1C].unkF = 3;
                        break;

                    case 7:
                        if ((gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 1)
                            && (gEntityAnimationInfo[arg0 - gUnk_0300363C].frame == 1)) {
                            m4aSongNumStart(0x65);
                        }
                        if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                            SetPaletteAnimEntry(arg0, 8);
                        }
                        break;
                }
                break;

            case 5:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 2) {
                    SetPaletteAnimEntry(arg0, 2);
                } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_03005400.unkE_1 = 1;
                    gUnk_03005400.unkD = 3;
                    gUnk_03005400.unk8_4 = 0;
                    gUnk_03005400.unkA = 6;
                    m4aSongNumStart(0x65);
                }
                break;

            case 6:
                gUnk_03003590[0].unk4 += 8;
                gEntityInfo[arg0].yPosBg2 -= 4;
                if ((s16)gEntityInfo[arg0].yPosBg2 < 0x30) {
                    gUnk_03005400.unkA = 7;
                    gUnk_030007E0.unkC_0 = 3;
                    gUnk_030007E0.unk8 = 0x8C;
                }
                break;

            case 7:
                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unk9 = 0x1E;
                    GetEntityLookupData(0x1E);
                    gUnk_03005400.unk0 = 0x46;
                    gUnk_03005400.unk10 = 0;
                }

                if (gUnk_03005400.unk0 > 0xA) {
                    if ((gEntityInfo[0].xPosBg2 < gEntityInfo[arg0].xPosBg2) && (gEntityInfo[arg0].unkC_2 == 0)) {
                        s32 tmp; // FAKE
                        gEntityInfo[arg0].unkC_2 = 1;
                        gUnk_03005400.unkF = tmp = -gUnk_03005400.unkF;
                    } else if ((gEntityInfo[0].xPosBg2 > gEntityInfo[arg0].xPosBg2) && (gEntityInfo[arg0].unkC_2 == 1)) {
                        s32 tmp; // FAKE
                        gEntityInfo[arg0].unkC_2 = 0;
                        gUnk_03005400.unkF = tmp = -gUnk_03005400.unkF;
                    }
                } else {
                    gUnk_03005400.unkF = 0;
                    gUnk_03005400.unk11 = 0;
                }

                if (gUnk_03005400.unk0 == 1) {
                    gUnk_03005400.unk8_0 = 0;
                    gUnk_03005400.unk0 = 0;
                    gUnk_03005400.unkA = 8;
                    gUnk_03005400.unk13 = 0;
                    gUnk_03005400.unk8_4 = 1;
                    gUnk_03003590[0].unk4 = 4;
                    m4aSongNumStart(0x42);
                }
                break;

            case 8:
                if ((gUnk_03005400.unk8_2 != 0) && (gUnk_03005400.unk13 == 0)) {
                    m4aSongNumStart(0x43);
                    gEntityInfo[0x20].unkF = 4;
                    gEntityInfo[0x1F].unkF = 4;
                    gEntityInfo[0x1E].unkF = 4;
                    gEntityInfo[0x1D].unkF = 4;
                    gEntityInfo[0x1C].unkF = 4;

                    gUnk_03003590[0].unk4 = 0;
                    gUnk_03005400.unk9 = 0;
                    GetEntityLookupData(0);
                    gUnk_03005400.unk0 = 0x14;
                    gUnk_03005400.unk13 = 1;
                    gUnk_03005400.unkE_1 = 1;
                    gUnk_03005400.unkD = 5;
                    gUnk_03005400.unk8_6 = 0;

                    if ((gUnk_03005220.unk31 != 0) && (gUnk_03005220.unk3E == 0)) {
                        gEntityInfo[0].yPosBg2 -= 0x30;
                    }
                    gUnk_030007E0.unkC_0 = 5;
                    gUnk_030007E0.unk8 = 0x78;

                    gEntityInfo[0x15].unkC_2 = 0;
                    gEntityInfo[0x16].unkC_2 = 1;
                    gEntityInfo[0x15].xPosBg2 = gEntityInfo[arg0].xPosBg2 + 0x20;
                    gEntityInfo[0x16].xPosBg2 = gEntityInfo[arg0].xPosBg2 - 0x20;
                    gEntityInfo[0x15].yPosBg2 = gEntityInfo[arg0].yPosBg2 + 0xC;
                    gEntityInfo[0x16].yPosBg2 = gEntityInfo[arg0].yPosBg2 + 0xC;
                    gEntityInfo[0x15].unkF = 0x19;
                    gEntityInfo[0x16].unkF = 0x19;
                } else {
                    if ((gUnk_03005400.unk0 == 0) && (gUnk_03005400.unk13 == 1)) {
                        gUnk_03003590[0].unk4 = 0;
                        gUnk_03005400.unkA = 9;
                        gUnk_03005400.unk0 = 0x3C;
                        SetPaletteAnimEntry(arg0, 9);
                    } else if (gUnk_03003590[0].unk4 == 0) {
                        gUnk_03003590[0].unk4 = 0;
                    } else {
                        gUnk_03003590[0].unk4 += 4;
                    }
                }
                return;

            case 12:
                gEntityInfo[0x13].unkF = 0x1C;
                gEntityInfo[0x14].unkF = 0x1C;

                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0x10) {
                    SetPaletteAnimEntry(arg0, 0x10);
                } else if ((gUnk_03005220.unk31 != 0) && (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF)) {
                    gUnk_03005400.unkE_4 = 1;
                    gEntityInfo[arg0].unkF = 3;
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 3) {
        SpawnEntitiesForVision(arg0);
    }
}
/**
 * SetupEntitySpawnTable: ported from kleod SetupEntitySpawnTable.
 */
void SetupEntitySpawnTable(u8 arg0) {
    u32 var_r3;

    gUnk_03005400.unk14 = 0;

    for (var_r3 = 0; var_r3 < 9; var_r3++) {
        gEntityInfo[0x1B + var_r3].unkF = gEntityInfo[0x24 + var_r3].unkF = arg0;
    }

    if (arg0 == 0) {
        switch (gUnk_03004C20.room - 1) {
            case 0:
                gUnk_03000790[0].unk8 = 0x48;
                gUnk_03000790[0].unk4 = 0x48;
                gUnk_03000790[1].unk8 = 0x48;
                gUnk_03000790[1].unk4 = 0x48;
                break;

            case 1:
                gUnk_03000790[0].unk8 = 0x68;
                gUnk_03000790[0].unk4 = 0x68;
                gUnk_03000790[1].unk8 = 0x48;
                gUnk_03000790[1].unk4 = 0x48;
                break;

            case 2:
                gUnk_03000790[0].unk8 = 0x68;
                gUnk_03000790[0].unk4 = 0x68;
                gUnk_03000790[1].unk8 = 0x68;
                gUnk_03000790[1].unk4 = 0x68;
                break;

            case 3:
                gUnk_03000790[0].unk8 = 0x88;
                gUnk_03000790[0].unk4 = 0x88;
                gUnk_03000790[1].unk8 = 0x48;
                gUnk_03000790[1].unk4 = 0x48;
                break;

            case 4:
                gUnk_03000790[0].unk8 = 0x48;
                gUnk_03000790[0].unk4 = 0x48;
                gUnk_03000790[1].unk8 = 0x88;
                gUnk_03000790[1].unk4 = 0x88;
                break;
        }

        for (var_r3 = 0; var_r3 < 9; var_r3++) {
            gEntityInfo[0x1B + var_r3].xPosBg2
                = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_r3 + 0xE].unk0[gUnk_03004C20.room - 1].unk0;
            gEntityInfo[0x24 + var_r3].xPosBg2
                = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_r3 + 0x17].unk0[gUnk_03004C20.room - 1].unk0;

            gEntityInfo[0x1B + var_r3].yPosBg2 = gUnk_03000790[0].unk4;
            gEntityInfo[0x24 + var_r3].yPosBg2 = gUnk_03000790[1].unk4;

            gEntityInfo[0x24 + var_r3].unk10 = 1;
            gEntityInfo[0x1B + var_r3].unk10 = 1;
        }

        gUnk_03000790[0].unk0 = gEntityInfo[0x1B].xPosBg2 - 0x10;
        gUnk_03000790[0].unk2 = gEntityInfo[0x1B].xPosBg2 + 0x10;
        gUnk_03000790[1].unk0 = gEntityInfo[0x24].xPosBg2 - 0x10;
        gUnk_03000790[1].unk2 = gEntityInfo[0x24].xPosBg2 + 0x10;
        m4aSongNumStart(0x68);
    } else {
        gUnk_03005220.unk3B = 0;
        gUnk_03005220.unk3A = 0;

        for (var_r3 = 0; var_r3 < 9; var_r3++) {
            gEntityInfo[0x1B + var_r3].unk10 = gEntityInfo[0x24 + var_r3].unk10 = 0;
        }
    }
}
/**
 * RollRandomLevelVariant: select a random level variant based on difficulty.
 *
 * Reads the current difficulty level, computes a variant index using
 * difficulty parity and a random modulo, and stores it to the level state.
 */
void RollRandomLevelVariant(void) {
    u8 *state = gGameFlags;
    u8 difficulty = state[0x0C];
    u32 difficultyIndex = (u8)(difficulty - 1);
    u32 rng = thunk_sub_080002A0();
    u8 *levelState = gControlBlock;
    u32 parity = difficultyIndex & 1;
    u32 variant = sub_0805193C((u8)rng, 5 - difficultyIndex);
    levelState[0x0E] = parity + variant + 1;
}
/**
 * UpdatePlayerBoss: per-frame update of the player during boss fights.
 */
void UpdatePlayerBoss(u8 arg0) {
    u8 sp14;
    s32 var_r0_3;
    s32 var_r0_4;

    do {
    } while (0); // Required to match

    sp14 = gUnk_03005400.unkC - 1;
    gEntityInfo[arg0].affineHFlip_matrixNum = 3;
    gEntityInfo[arg0].affineDouble = 1;
    gUnk_03003590[0].unk5_0 = gEntityInfo[0x12].unkC_2;
    if (gUnk_03005400.unk0 != 0) {
        gUnk_03005400.unk0 -= 1;
    }
    ApplyPlayerMovement(arg0, gUnk_03003620);

    if (gEntityInfo[arg0].unkF == 14) {
        switch (gUnk_03005400.unkA) {
            case 0:
                gUnk_030007E0.unkC_0 = 1;
                gUnk_030007E0.unkC_4 = 0;
                gUnk_030007E0.unk6 = 0x78;
                gUnk_030007E0.unk0 = 0x78;
                gUnk_030007E0.unk8 = 0x50;
                gUnk_030007E0.unk2 = 0x50;
                gUnk_030007E0.unkA = 0x40;
                gUnk_030007E0.unk4 = 0x40;

                SetPaletteAnimEntry(0, 0);
                SetPaletteAnimEntry(0x12, 2);
                SetPaletteAnimEntry(0x17, 0);
                SetPaletteAnimEntry(0x19, 1);

                gEntityInfo[0x12].priority = 2;
                gUnk_03003590[0].unk2 = -0xC0;
                gUnk_03003590[0].unk0 = -0xC0;
                gUnk_03005400.unk8_0 = 1;
                gUnk_03005400.unk13 = 0;
                gUnk_03005400.unkA = 1;
                gUnk_03005400.unkE_7 = 1;
                break;

            case 1:
                gUnk_03005400.unk13 += 4;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    + ((COS(gUnk_03005400.unk13) * 2) >> 5);

                if ((s16)gUnk_03003590[0].unk0 < gUnk_081168E2[gUnk_03005400.unkC]) {
                    gUnk_03003590[0].unk0 += 1;
                    gUnk_03003590[0].unk2 += 1;
                }

                if (((s16)gUnk_03003590[0].unk0 == gUnk_081168E2[gUnk_03005400.unkC]) && (gUnk_03005400.unk13 == 0x40)) {
                    SetupEntitySpawnTable(0x1C);
                    gUnk_03005400.unkA = 2;
                }
                break;

            case 2:
                gUnk_03005400.unk8_4 = 0;
                gUnk_03005400.unk13 = 0;
                gUnk_03005400.unk8_0 = 1;
                gEntityInfo[arg0].unk8.split.unk8 = 0x80;
                gUnk_03005400.unkA = 6;
                gEntityInfo[arg0].unkF = 0;
                gEntityInfo[0x12].priority = 1;
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 0) {
        switch (gUnk_03005400.unkA) {
            case 0:
                if ((thunk_sub_080002A0() % 10) <= 4) {
                    gEntityInfo[arg0].unkC_2 = 0;
                } else {
                    gEntityInfo[arg0].unkC_2 = 1;
                }

                SetPaletteAnimEntry(arg0, 0);
                gUnk_03005400.unkA = 2;
                break;

            case 1:
                if ((gUnk_030007CC < gUnk_081168E2[gUnk_03005400.unkC]) && (gUnk_03005400.unkC != 0)) {
                    gUnk_030007CC += 2;
                }

                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unk0 = 0x3C;
                    gUnk_03005400.unk8_0 = 1;
                    gUnk_03005400.unk8_6 = 1;
                }

                if (gUnk_03005400.unk0 == 1) {
                    DmaCopy16Wait(
                        3, &gUnk_08078628,
                        OBJ_PLTT
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20),
                        0x20);

                    if (gEntityInfo[0x13].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x13].xPosBg2, gEntityInfo[0x13].yPosBg2, 2, 0x13);
                        gEntityInfo[0x13].unkF = 0x1C;
                    }
                    if (gEntityInfo[0x14].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x14].xPosBg2, gEntityInfo[0x14].yPosBg2, 2, 0x14);
                        gEntityInfo[0x14].unkF = 0x1C;
                    }
                    if (gEntityInfo[0x15].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x15].xPosBg2, gEntityInfo[0x15].yPosBg2, 2, 0x15);
                        gEntityInfo[0x15].unkF = 0x1C;
                    }
                    if (gEntityInfo[0x16].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x16].xPosBg2, gEntityInfo[0x16].yPosBg2, 2, 0x16);
                        gEntityInfo[0x16].unkF = 0x1C;
                    }

                    if (gUnk_03005400.unkC == 2) {
                        SetupEntitySpawnTable(0x1C);
                    }

                    if (gUnk_03005400.unkC == 0) {
                        SetupEntitySpawnTable(0x1C);
                        gUnk_03005400.unkA = 9;
                        gUnk_03005400.unk13 = 0;
                        SetPaletteAnimEntry(arg0, 6);
                    } else {
                        gUnk_03005400.unk14 = gUnk_08116A02[gUnk_03005400.unkC][4];
                        gUnk_03005400.unk16 = gEntityInfo[arg0].unkC_2;
                        gUnk_03005400.unkA = 4;
                        gUnk_03005400.unk8_0 = 0;
                    }
                } else {
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 5) {
                        DmaCopy16Wait(
                            3, &gUnk_08078628,
                            OBJ_PLTT
                                + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                   * 0x20),
                            0x20);
                    }

                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 0) {
                        DmaFill16(3, 0xFFFF,
                                  OBJ_PLTT
                                      + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                         * 0x20),
                                  0x20);
                    }
                }
                break;

            case 2:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0) {
                    SetPaletteAnimEntry(arg0, 0);
                }

                if ((gUnk_03004C20.sceneFrameCounter % 2) == 0) {
                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 += gUnk_08116A02[gUnk_03005400.unkC][0];
                    } else {
                        gEntityInfo[arg0].xPosBg2 -= gUnk_08116A02[gUnk_03005400.unkC][0];
                    }
                }

                gUnk_03005400.unk13 += gUnk_08116A02[gUnk_03005400.unkC][1];
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    + ((COS(gUnk_03005400.unk13) * 2) >> 4);
                if ((gUnk_03005400.unk13 % 0x80) == 0) {
                    m4aSongNumStart(0x66);
                }

                if (gEntityInfo[arg0].xPosBg2 <= 0x5F) {
                    gEntityInfo[arg0].unkC_2 = 0;
                    gUnk_03005400.unk14 += 1;
                } else if (gEntityInfo[arg0].xPosBg2 > 0x180) {
                    gEntityInfo[arg0].unkC_2 = 1;
                    gUnk_03005400.unk14 += 1;
                }

                if (((u16)(gEntityInfo[arg0].xPosBg2 - 0xE7) <= 0x12) && (gEntityInfo[0x13].unkF == 0x1C)
                    && (gEntityInfo[0x14].unkF == 0x1C) && (gEntityInfo[0x15].unkF == 0x1C) && (gEntityInfo[0x16].unkF == 0x1C)) {
                    gUnk_03005400.unkA = 3;
                } else if (gUnk_03005400.unk14 != 3) {

                } else if (sp14 == 2) {

                } else {
                    SetupEntitySpawnTable(0x1C);
                    gEntityInfo[arg0].unk8.split.unk8 = 0x50;
                    gUnk_03005400.unk15 = 2;
                    gUnk_03005400.unkA = 0xA;

                    gUnk_030007E0.unkC_0 = 6;
                    gUnk_030007E0.unkC_4 = 0;
                    gUnk_030007E0.unk6 = 0x78;
                    gUnk_030007E0.unk8 = 0x3C;
                    gUnk_030007E0.unkA = 0x40;
                }
                break;

            case 3: {
                u32 sp10;
                u32 tmp;
                struct Unk_0800BEF0 sp4;
                sp4.unk0 = gEntityInfo[arg0].xPosBg2;
                sp4.unk2 = gEntityInfo[arg0].yPosBg2;
                sp4.unk4 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk0;
                sp4.unk6 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2;
                sp4.unk9 = 2;
                sp4.unk8 = 2;
                UpdateTextScroll(&sp10, sp4);
                tmp = sp10;
                gEntityInfo[arg0].xPosBg2 = tmp;
                gEntityInfo[arg0].yPosBg2 = tmp >> 0x10;
            }

                if ((gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk0 >> 3)
                    != (gEntityInfo[arg0].xPosBg2 >> 0x3)) {

                } else if ((gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2 >> 3)
                           != (gEntityInfo[arg0].yPosBg2 >> 0x3)) {

                } else {
                    gUnk_03005400.unk8_6 = 0;
                    SetPaletteAnimEntry(arg0, 2);
                    gEntityInfo[arg0].unk8.split.unk8 = 0x78;
                    gUnk_03005400.unkA = 6;
                }
                break;

            case 10:
                gUnk_03005400.unk13 += 2;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    + ((COS(gUnk_03005400.unk13) * 5) >> 7);

                if (gEntityInfo[arg0].unk8.split.unk8 == 0x32) {
                    RollRandomLevelVariant();
                    SetupEntitySpawnTable(0);
                }

                gEntityInfo[arg0].unk8.split.unk8 -= 1;
                if (gEntityInfo[arg0].unk8.split.unk8 == 0xFF) {
                    gUnk_03005400.unk14 = 0;
                    gUnk_03005400.unkA = gUnk_03005400.unk15;
                    gUnk_030007E0.unkC_0 = 1;
                    gUnk_030007E0.unkC_4 = 0;
                    gUnk_030007E0.unk6 = 0x78;
                    gUnk_030007E0.unk8 = 0x50;
                    gUnk_030007E0.unkA = 0x40;
                }
                break;

            case 6:
                gUnk_030007E0.unkA = 0x80;
                gUnk_03005400.unk8_0 = 0;
                gUnk_03005400.unk8_6 = 1;

                gUnk_03005400.unk15 += 1;
                var_r0_3 = (s8)gUnk_03005400.unk15;
                if (var_r0_3 < 0) {
                    var_r0_3 += 7;
                }
                if ((var_r0_3 >> 3) & 1) {
                    gUnk_03003590[0].unk0 += 8;
                    gUnk_03003590[0].unk2 += 8;
                } else {
                    gUnk_03003590[0].unk0 -= 8;
                    gUnk_03003590[0].unk2 -= 8;
                }

                if ((gUnk_03005400.unk15 % 0x10) == 0) {
                    m4aSongNumStart(0x69);
                }

                gEntityInfo[arg0].unk8.split.unk8 -= 1;
                if (gEntityInfo[arg0].unk8.split.unk8 == 0) {
                    SetupEntitySpawnTable(0x1C);
                    SetPaletteAnimEntry(arg0, 5);
                    gEntityInfo[arg0].unk8.split.unk8 = 0xB4;
                    gUnk_03005400.unkA = 7;
                    m4aSongNumStart(0x69);
                    gEntityInfo[0x16].unkF = 3;
                    gEntityInfo[0x15].unkF = 3;
                }
                break;

            case 7:
                gUnk_03005400.unk15 += 1;
                var_r0_4 = (s8)gUnk_03005400.unk15;
                if (var_r0_4 < 0) {
                    var_r0_4 += 7;
                }
                if ((var_r0_4 >> 3) & 1) {
                    gUnk_03003590[0].unk0 += 9;
                    gUnk_03003590[0].unk2 += 9;
                } else {
                    gUnk_03003590[0].unk0 -= 9;
                    gUnk_03003590[0].unk2 -= 9;
                }

                gEntityInfo[arg0].unk8.split.unk8 -= 1;
                if (gEntityInfo[arg0].unk8.split.unk8 == 0) {
                    gUnk_030007CC = gUnk_081168E2[gUnk_03005400.unkC];
                    gUnk_03003590[0].unk2 = 0;
                    gUnk_03003590[0].unk0 = 0;

                    gEntityInfo[0x16].xPosBg2 = gEntityInfo[arg0].xPosBg2;
                    gEntityInfo[0x15].xPosBg2 = gEntityInfo[arg0].xPosBg2;
                    gEntityInfo[0x15].yPosBg2 = gEntityInfo[0x16].yPosBg2 = gEntityInfo[arg0].yPosBg2 - 0x3C;
                    gEntityInfo[0x16].unkF = 0x19;
                    gEntityInfo[0x15].unkF = 0x19;

                    SetPaletteAnimEntry(arg0, 6);
                    m4aSongNumStart(0x6A);
                    gUnk_03005400.unkA = 8;
                }
                break;

            case 8:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_030007E0.unkA = 0x40;
                    SetupEntitySpawnTable(0x1C);
                    SetPaletteAnimEntry(0x12, 2);
                    gUnk_03005400.unk15 = 0;
                    gEntityInfo[arg0].unk8.split.unk8 = 0x50;
                    gUnk_03005400.unkA = 0xA;
                    gUnk_03005400.unk8_0 = 0;
                    gUnk_03005400.unk8_6 = 0;
                }
                break;

            case 4:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_03005400.unkA = 5;
                    SetPaletteAnimEntry(arg0, 4);
                    gEntityInfo[0x12].unkC_2 ^= 1;
                    m4aSongNumStart(0x67);
                }
                goto block_124;

            case 5:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_03005400.unkA = 4;
                    SetPaletteAnimEntry(arg0, 3);
                    gEntityInfo[0x12].unkC_2 ^= 1;
                    m4aSongNumStart(0x67);
                }
                goto block_124;

            case 9:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_03003590[0].unk4 += 4;
                    if ((s16)gUnk_03003590[0].unk0 <= -0xF0) {
                        if (gUnk_03005220.unk31 != 0) {
                            gEntityInfo[arg0].unk10 = 0;
                            gEntityInfo[arg0].yPosBg2 = 0;
                            gEntityInfo[arg0].xPosBg2 = 0;
                            gUnk_03005400.unkE_4 = 1;
                            gEntityInfo[arg0].unkF = 3;
                        } else {
                            gUnk_03005400.unk14 = 0x7F;
                            goto block_124;
                        }
                    } else {
                        if ((gUnk_03004C20.sceneFrameCounter % 4) == 0) {
                            gUnk_03003590[0].unk0 -= 4;
                            gUnk_03003590[0].unk2 -= 4;
                        }
                        gUnk_03005400.unk14 = 0x7F;
                        goto block_124;
                    }
                }
                break;

            block_124:
                if (gUnk_03005400.unk16 & 1) {
                    gEntityInfo[0x12].xPosBg2 += gUnk_08116A02[gUnk_03005400.unkC][2];
                } else {
                    gEntityInfo[0x12].xPosBg2 -= gUnk_08116A02[gUnk_03005400.unkC][2];
                }

                if (gUnk_03005400.unk16 & 2) {
                    gEntityInfo[0x12].yPosBg2 += gUnk_08116A02[gUnk_03005400.unkC][3];
                } else {
                    gEntityInfo[0x12].yPosBg2 -= gUnk_08116A02[gUnk_03005400.unkC][3];
                }

                if (gEntityInfo[0x12].xPosBg2 <= 0x4F) {
                    if (gUnk_03005400.unk14 == 0x7F) {
                        m4aSongNumStart(0x7E);
                    }

                    gUnk_03005400.unk14 -= 1;
                    gUnk_03005400.unk16 &= ~1;
                    gUnk_03005400.unk16 |= 1;
                } else if (gEntityInfo[0x12].xPosBg2 > 0x190) {
                    if (gUnk_03005400.unk14 == 0x7F) {
                        m4aSongNumStart(0x7E);
                    }

                    gUnk_03005400.unk14 -= 1;
                    gUnk_03005400.unk16 &= ~1;
                }

                if (gEntityInfo[0x12].yPosBg2 <= 0x7F) {
                    if (gUnk_03005400.unk14 == 0x7F) {
                        m4aSongNumStart(0x7E);
                    }

                    gUnk_03005400.unk14 -= 1;
                    gUnk_03005400.unk16 &= ~2;
                    gUnk_03005400.unk16 |= 2;
                } else if (gEntityInfo[0x12].yPosBg2 > 0x100) {
                    if (gUnk_03005400.unk14 == 0x7F) {
                        m4aSongNumStart(0x7E);
                    }

                    gUnk_03005400.unk14 -= 1;
                    gUnk_03005400.unk16 &= ~2;
                }

                if ((s8)gUnk_03005400.unk14 <= 0) {
                    if (gEntityInfo[0x12].xPosBg2 < 0xF0) {
                        gEntityInfo[0x12].unkC_2 = 0;
                    } else {
                        gEntityInfo[0x12].unkC_2 = 1;
                    }

                    gUnk_03005400.unk13 = 0;
                    gUnk_030034DC = 0;
                    gUnk_03005400.unkA = 3;
                    SetPaletteAnimEntry(arg0, 0);
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 3) {
        SpawnEntitiesForVision(arg0);
    }
    if (gUnk_03005220.hearts == 0) {
        SetupEntitySpawnTable(0x1C);
    }
}
/**
 * ConfigureEntityBehavior: ported from kleod ConfigureEntityBehavior.
 */
void ConfigureEntityBehavior(u8 arg0, u8 arg1, u8 arg2) {
    void *var_r3;
    void *var_r5;
    void *var_r6;
    u32 var_sb;
    u8 var_r1;

    if (arg2 != 0) {
        gEntityInfo[0x23].unkF = 0x19;
        gEntityInfo[0x22].unkF = 0x19;
        gEntityInfo[0x21].unkF = 0x19;
        gEntityInfo[0x20].unkF = 0x19;
    }

    switch (arg1) {
        case 0:
            var_sb = 0xD;
            var_r6 = &gUnk_03003D16[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x1B)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C;
            break;

        case 1:
            var_sb = 4;
            var_r6 = &gUnk_03003F56[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x24)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C + (gBgInfo[2].hLength * 0x13);
            break;

        case 2:
            var_sb = 6;
            var_r6 = &gUnk_03003E96[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x21)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C + (gBgInfo[2].hLength * 0x13);
            break;

        case 3:
            var_sb = 6;
            var_r6 = &gUnk_03003DD6[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x1E)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C + (gBgInfo[2].hLength * 0x13);
            break;

        case 4:
            var_sb = 6;
            var_r6 = &gUnk_03003D16[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x1B)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C + (gBgInfo[2].hLength * 0xD);
            break;

        case 5:
            var_sb = 6;
            var_r6 = &gUnk_03003D16[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x1B)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C + (gBgInfo[2].hLength * 0x16);
            break;

        case 6:
            var_sb = 4;
            var_r6 = &gUnk_03003F56[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x24)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C;
            break;

        case 7:
            var_sb = 7;
            var_r6 = &gUnk_03003E96[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x21)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C;
            break;

        case 8:
            var_sb = 0xA;
            var_r6 = &gUnk_03003DD6[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x1E)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C;
            break;

        case 9:
            var_sb = 0xD;
            var_r6 = &gUnk_03003D16[arg0];
            var_r5 = &gBgDataPtrs.pBufBg2Tilemap[6 + (arg0 * 8) + (gBgInfo[2].hLength * 0x1B)];
            var_r3 = gBgDataPtrs.pBufBg2Tilemap + 0x3C;
            break;
    }

    for (var_r1 = 0; var_r1 < var_sb; var_r1++) {
        DmaCopy16Wait(3, var_r3, var_r6, 0x8);
        DmaCopy16Wait(3, var_r3, var_r5, 0x8);
        var_r6 += 0x40;
        var_r5 += gBgInfo[2].hLength;
        var_r3 += gBgInfo[2].hLength;
    }
}
/**
 * ResetEntityTypesOnDeath: on player death, reverts entities of a given type back to their idle spawn state so they re-appear
 * correctly on respawn.
 */
void ResetEntityTypesOnDeath(u8 arg0) {
    u8 var_r5;

    if (gUnk_03005400.unkC == 0) {
        return;
    }

    if (arg0 == 0x19) {
        for (var_r5 = 0; var_r5 < 2; var_r5++) {
            if (gEntityInfo[var_r5 + 0x13].unkF == 0x1C) {
                gEntityInfo[var_r5 + 0x13].unkF = 0x19;
            }
        }
    } else {
        for (var_r5 = 0; var_r5 < 2; var_r5++) {
            if ((gEntityInfo[var_r5 + 0x13].unkF == 0) || (gEntityInfo[var_r5 + 0x13].unkF == 0x19)) {
                SpawnEntityAtPosition(gEntityInfo[var_r5 + 0x13].xPosBg2, gEntityInfo[var_r5 + 0x13].yPosBg2, 2, var_r5 + 0x13);
            }

            if (gEntityInfo[var_r5 + 0x13].unkF == 0x13) {
                SpawnEntityAtPosition(gEntityInfo[var_r5 + 0x13].xPosBg2, gEntityInfo[var_r5 + 0x13].yPosBg2, 2, var_r5 + 0x13);
            }
        }
    }
}
/**
 * UpdatePlayerMinigame: per-frame update for the minigame mode — reads input, refreshes the scrolling minigame tilemap region
 * (gUnk_03003650) from the decompressed BG2 buffer, applies random palette shuffles, and advances the minigame state timers.
 */
void UpdatePlayerMinigame(u8 arg0) {
    s32 var_r1;
    u8 var_r2;
    u8 var_r3;
    u8 var_r4;
    u8 var_r5;

    gEntityInfo[arg0].affineHFlip_matrixNum = 3;
    gEntityInfo[arg0].affineDouble = 1;
    gUnk_03003590[0].unk5_0 = gEntityInfo[0x12].unkC_2;
    if (gUnk_03005400.unk0 != 0) {
        gUnk_03005400.unk0 -= 1;
    }
    DmaCopy16(3, &gUnk_080D8C30[(gUnk_03004C20.sceneFrameCounter / 12) % 6], BG_PLTT + 0x100, 0x40);
    if ((gEntityInfo[0].yPosBg2 > 0x147) && (gUnk_03005220.hearts != 0)) {
        gUnk_03005220.hearts = 1;
        PlayerRespawnOrDeath(1);
    }
    if (gEntityInfo[arg0].unkF == 14) {
        if (gUnk_03005400.unkA == 0) {
            gUnk_030007E0.unkC_0 = 7;
            gUnk_030007E0.unkC_4 = 0;
            gUnk_030007E0.unk6 = 0x78;
            gUnk_030007E0.unk0 = 0x78;
            gUnk_030007E0.unk8 = 0xA0;
            gUnk_030007E0.unk2 = 0xA0;
            gUnk_030007E0.unkA = 0x60;
            gUnk_030007E0.unk4 = 0x60;
            gUnk_03005400.unk9 = 0;
            gUnk_03005400.unk8_4 = 1;
            gEntityInfo[arg0].unkF = 0;
            gUnk_03005400.unkE_7 = 1;
            gUnk_03005400.unk14 = 0xFF;
        }
    } else if (gEntityInfo[arg0].unkF == 0 || gEntityInfo[arg0].unkF == 26) {
        ApplyPlayerMovement(arg0, gUnk_03003620);
        switch (gUnk_03005400.unkA) {
            case 0:
                gUnk_03005400.unk13 = thunk_sub_080002A0() % 6;
                while (1) {
                    if ((s8)gUnk_03005400.unk15 != 3) {
                        if (((gUnk_03005400.unk14 >> gUnk_03005400.unk13) & 1) != 0) {
                            break;
                        }
                    } else {
                        if (((gUnk_03005400.unk14 >> gUnk_03005400.unk13) & 1) == 0) {
                            break;
                        }
                    }
                    gUnk_03005400.unk13 = (gUnk_03005400.unk13 + 1) % 6;
                }
                gEntityInfo[0x12].xPosBg2 = (gUnk_03005400.unk13 << 6) + 0x50;
                gEntityInfo[0x12].yPosBg2 = 0x190;
                if (gEntityInfo[0].xPosBg2 < gEntityInfo[0x12].xPosBg2) {
                    gEntityInfo[0x12].unkC_2 = 1;
                } else {
                    gEntityInfo[0x12].unkC_2 = 0;
                }
                gUnk_03003590[0].unk2 = 0;
                gUnk_03003590[0].unk0 = 0;
                gUnk_03003590[0].unk4 = 0xC0;
                gUnk_03005400.unk8_6 = 0;
                gUnk_03005400.unk8_0 = 0;
                gUnk_03005400.unk8_4 = 0;
                gEntityInfo[arg0].priority = 1;
                gEntityInfo[arg0].unkF = 0;
                SetPaletteAnimEntry(arg0, 0);
                gUnk_03005400.unk0 = gUnk_08116A46[gUnk_03005400.unkC][0] * 0x3C;
                gUnk_03005400.unkA = 4;
                break;

            case 4:
                if (gUnk_03005400.unk0 > (gUnk_08116A46[gUnk_03005400.unkC][1] * 0x3C)) {
                    break;
                }

                if (gUnk_03005400.unk0 != 0) {
                    if ((gUnk_03005400.unk14 >> gUnk_03005400.unk13) & 1) {
                        var_r1 = ((gUnk_03004C20.sceneFrameCounter / 8) % 4) + 1;
                        if (gUnk_03005400.unk0 == 1) {
                            var_r1 = 0;
                        }
                        for (var_r3 = 0; var_r3 < 3; var_r3++) {
                            for (var_r4 = 0; var_r4 < 3; var_r4++) {
                                gUnk_03003650[var_r3 + 0x17][8 + var_r4 + (gUnk_03005400.unk13 * 8)]
                                    = gBgDataPtrs.pBufBg2Tilemap[(var_r1 * 3) + var_r4 + ((var_r3 + 0x2D) * gBgInfo[2].hLength)];
                            }
                        }
                    }
                    if (gUnk_03005400.unk0 > 0x78) {
                        break;
                    }
                }

                if (gEntityInfo[0x12].yPosBg2 > 0x117) {
                    if ((gUnk_03004C20.sceneFrameCounter % 2) != 0) {
                        gEntityInfo[0x12].yPosBg2 -= 1;
                    }
                    if ((gUnk_03005400.unk14 >> gUnk_03005400.unk13) & 1) {
                        if (gEntityInfo[arg0].yPosBg2 == 0x164) {
                            m4aSongNumStart(0x6B);
                            ConfigureEntityBehavior(gUnk_03005400.unk13, 1, 1);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 2;
                        }
                        if (gEntityInfo[arg0].yPosBg2 == 0x154) {
                            m4aSongNumStart(0x6B);
                            ConfigureEntityBehavior(gUnk_03005400.unk13, 2, 1);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 2;
                        }
                        if (gEntityInfo[arg0].yPosBg2 == 0x144) {
                            m4aSongNumStart(0x6B);
                            ConfigureEntityBehavior(gUnk_03005400.unk13, 3, 1);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 4;
                        }
                        if (gEntityInfo[arg0].yPosBg2 == 0x134) {
                            m4aSongNumStart(0x6B);
                            ConfigureEntityBehavior(gUnk_03005400.unk13, 4, 1);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 4;
                        }
                        if (gEntityInfo[arg0].yPosBg2 == 0x118) {
                            m4aSongNumStart(0x6B);
                            ConfigureEntityBehavior(gUnk_03005400.unk13, 5, 1);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 4;
                        }
                    }
                } else {
                    gUnk_03005400.unk14 &= ~(1 << gUnk_03005400.unk13);
                    gUnk_03005400.unk13 = 0;
                    gUnk_03005400.unk15 += 1;
                    gUnk_03005400.unkA = 5;
                    gUnk_03005400.unk0 = 0x50;
                    gEntityInfo[arg0].priority = 0;
                }
                break;

            case 5:
                if (gUnk_03005400.unk0 == 0) {
                    if ((gUnk_03004C20.sceneFrameCounter % 8) == 0 && (gUnk_03005400.unk13 <= 7)) {
                        gUnk_03005400.unk13 += 1;
                    }

                    if ((s16)gEntityInfo[0x12].yPosBg2 <= 0x10) {
                        SetPaletteAnimEntry(arg0, 6);
                        gUnk_03005400.unk8_6 = 1;
                        gUnk_03005400.unk8_4 = 0;
                        if ((s8)gUnk_03005400.unk15 == 3) {
                            gEntityInfo[arg0].priority = 2;
                            gUnk_03005400.unk8_0 = 1;
                            gUnk_03005400.unkA = 0xB;
                        } else {
                            gUnk_03005400.unk0 = 0x60;
                            gUnk_03005400.unkA = 6;
                        }
                    } else {
                        gEntityInfo[0x12].yPosBg2 = (u16)(gEntityInfo[0x12].yPosBg2 - gUnk_03005400.unk13);
                    }
                }
                break;

            case 6:
                if (gUnk_03005400.unk0 == 0) {
                    if ((gUnk_03005400.unkC == 1) && ((thunk_sub_080002A0() % 100) > 0x46)) {
                        gEntityInfo[arg0].xPosBg2 = gEntityInfo[0].xPosBg2;
                        gEntityInfo[arg0].priority = 0;
                        gUnk_03005400.unk8_0 = 0;
                        m4aSongNumStart(0x42);
                        gUnk_03005400.unkA = 0xD;
                    } else {
                        gUnk_03005400.unk13 = thunk_sub_080002A0() % 6;
                        while ((gUnk_03005400.unk14 >> gUnk_03005400.unk13) & 1) {
                            gUnk_03005400.unk13 = (gUnk_03005400.unk13 + 1) % 6;
                        }
                        gEntityInfo[0x12].xPosBg2 = (gUnk_03005400.unk13 << 6) + 0x50;
                        gUnk_03003590[0].unk4 = 0x40;
                        m4aSongNumStart(0x6C);
                        gUnk_03005400.unkA = 7;
                    }
                }
                break;

            case 7:
                if (gEntityInfo[arg0].yPosBg2 <= 0x190) {
                    gEntityInfo[arg0].yPosBg2 += 8;
                } else {
                    gEntityInfo[arg0].priority = 0;
                    gUnk_03005400.unkA = 0;
                }
                break;

            case 1:
                gUnk_03005400.unk8_0 = 1;
                gUnk_03005400.unk8_6 = 1;
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    ResetEntityTypesOnDeath(0x1C);
                    gUnk_03005400.unkA = 2;
                    SetPaletteAnimEntry(arg0, 2);
                    gUnk_03005400.unk0 = 0x3C;
                }
                break;

            case 2:
                if (gUnk_03005400.unk0 != 0) {
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 5) {
                        DmaCopy16Wait(
                            3, &gUnk_08078628,
                            OBJ_PLTT
                                + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                   * 0x20),
                            0x20);
                    }
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 0) {
                        DmaFill16(3, 0xFFFF,
                                  OBJ_PLTT
                                      + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                         * 0x20),
                                  0x20);
                    }
                } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    DmaCopy16Wait(
                        3, &gUnk_08078768,
                        OBJ_PLTT
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20),
                        0x20);
                    SetPaletteAnimEntry(arg0, 3);
                    if (gUnk_03005400.unkC == 0) {
                        gUnk_03005400.unkA = 9;
                    } else {
                        gUnk_03005400.unkA = 3;
                    }
                }
                break;

            case 3:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    gUnk_03003590[0].unk4 += 4;
                    if (gEntityInfo[arg0].yPosBg2 <= 0x190) {
                        gEntityInfo[arg0].yPosBg2 += 2;
                        return;
                    }
                    for (var_r2 = 0; var_r2 < (0xA - ((gUnk_03005400.unkC - 1) * 2)); var_r2++) {
                        gEntityInfo[var_r2 + 0x16].unkF = 0x18;
                    }
                    gEntityInfo[arg0].xPosBg2 = 0xF0 - ((8 - gBgInfo[1].hOfs) * 4);
                    gEntityInfo[arg0].yPosBg2 = 0xF0;
                    gEntityInfo[arg0].priority = 2;
                    gUnk_03003590[0].unk4 = 0xC0;
                    gUnk_03003590[0].unk2 = 0xFF60;
                    gUnk_03003590[0].unk0 = 0xFF60;
                    gUnk_03005400.unk4 = 1;
                    gUnk_03005400.unkA = 8;
                    gEntityInfo[arg0].unkF = 0x1A;
                    DmaCopy16Wait(
                        3, &gUnk_08078728,
                        OBJ_PLTT
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20),
                        0x20);
                    SetPaletteAnimEntry(arg0, 6);
                }
                break;

            case 8:
                gEntityInfo[arg0].xPosBg2 = 0xF0 - ((8 - gBgInfo[1].hOfs) * 4);
                if (gUnk_03005220.hearts == 0) {
                    return;
                }

                for (var_r5 = 0; var_r5 < 6; var_r5++) {
                    if ((gEntityInfo[var_r5 + 0x16].unkF == 0x1C) && (((gUnk_03005400.unk14 >> var_r5) & 1) == 0)) {
                        gEntityInfo[var_r5 + 0x16].unk8.split.unk8 -= 1;
                        if (gEntityInfo[var_r5 + 0x16].unk8.split.unk8 == 0x3C) {
                            if (gEntityInfo[0].yPosBg2 > 0x122) {
                                gEntityInfo[0].yPosBg2 = 0x122;
                            }
                            ConfigureEntityBehavior(var_r5, 6, 0);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 1;
                        } else if (gEntityInfo[var_r5 + 0x16].unk8.split.unk8 == 0x28) {
                            if (gEntityInfo[0].yPosBg2 > 0x10A) {
                                gEntityInfo[0].yPosBg2 = 0x10A;
                            }
                            ConfigureEntityBehavior(var_r5, 7, 0);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 1;
                        } else if (gEntityInfo[var_r5 + 0x16].unk8.split.unk8 == 0x14) {
                            if (gEntityInfo[0].yPosBg2 > 0xF2) {
                                gEntityInfo[0].yPosBg2 = 0xF2;
                            }
                            ConfigureEntityBehavior(var_r5, 8, 0);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 1;
                        } else if (gEntityInfo[var_r5 + 0x16].unk8.split.unk8 == 0) {
                            if (gEntityInfo[0].yPosBg2 > 0xDA) {
                                gEntityInfo[0].yPosBg2 = 0xDA;
                            }
                            ConfigureEntityBehavior(var_r5, 9, 0);
                            gUnk_03005400.unkE_1 = 1;
                            gUnk_03005400.unkD = 1;
                            gUnk_03005400.unk14 |= 1 << var_r5;
                        }
                    }
                }

                if (gUnk_03005400.unk14 != 0xFF) {
                    break;
                }
                if (gEntityInfo[0x16].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x17].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x18].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x19].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x1A].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x1B].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x1C].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x1D].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x1E].unkF != 0x1C) {
                    break;
                }
                if (gEntityInfo[0x1F].unkF != 0x1C) {
                    break;
                }

                DmaCopy16Wait(
                    3, &gUnk_08078728,
                    OBJ_PLTT + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20),
                    0x20);
                gUnk_03005400.unk15 = 0;
                gUnk_03005400.unk16 = 0;
                if (gUnk_03005400.unk4 != 0) {
                    gEntityInfo[arg0].unkF = 0;
                    gEntityInfo[arg0].unk10 = 1;
                    if (gUnk_03005400.unk4 != 0) {
                        if (gEntityInfo[arg0].yPosBg2 > 0x2A) {
                            gEntityInfo[arg0].yPosBg2 -= 2;
                            break;
                        }
                    }
                    gEntityInfo[arg0].xPosBg2 = gEntityInfo[0].xPosBg2;
                    gEntityInfo[arg0].priority = 0;
                    gUnk_03003590[0].unk2 = 0;
                    gUnk_03003590[0].unk0 = 0;
                    gUnk_03003590[0].unk4 = 0x40;
                    gUnk_03005400.unk4 = 0;
                    gUnk_03005400.unk6 = 0;
                    gUnk_03005400.unk8_0 = 0;
                    m4aSongNumStart(0x42);
                    gUnk_03005400.unkA = 0xD;
                } else {
                    gUnk_03005400.unkA = 0;
                }
                break;

            case 9:
                if (gEntityInfo[0x12].yPosBg2 > 0x9F) {
                    gEntityInfo[0x12].yPosBg2 -= 4;
                }
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer != 0xFF) {
                    return;
                }

                DmaCopy16Wait(
                    3, &gUnk_08078728,
                    OBJ_PLTT + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20),
                    0x20);
                for (var_r4 = 0; var_r4 < 6; var_r4++) {
                    ConfigureEntityBehavior(var_r4, 0, 0);
                }
                if (gEntityInfo[0].yPosBg2 > 0xDA) {
                    gEntityInfo[0].yPosBg2 = 0xDA;
                }

                gEntityInfo[0x26].unkF = 0;
                gEntityInfo[0x25].unkF = 0;
                gEntityInfo[0x24].unkF = 0;

                gEntityInfo[0x24].xPosBg2 = gEntityInfo[0x25].xPosBg2 = gEntityInfo[0x26].xPosBg2 = gEntityInfo[0x12].xPosBg2;

                gEntityInfo[0x24].yPosBg2 = gEntityInfo[0x25].yPosBg2 = gEntityInfo[0x26].yPosBg2 = gEntityInfo[0x12].yPosBg2;

                gEntityInfo[0x24].unk8.split.unk8 = (((thunk_sub_080002A0() % 3) + 1) * 10) + 0x78;
                gEntityInfo[0x25].unk8.split.unk8 = (((thunk_sub_080002A0() % 3) + 1) * 10) + 0x64;
                gEntityInfo[0x26].unk8.split.unk8 = (((thunk_sub_080002A0() % 3) + 1) * 10) + 0x5A;

                gEntityInfo[0x12].unkF = 0x1C;
                gEntityInfo[0x12].unk10 = 0;
                gUnk_03005400.unkA = 0xA;
                m4aSongNumStart(0x80);
                break;

            case 10:
                if ((gEntityInfo[arg0].yPosBg2 <= 0xB3) && (gUnk_03004C20.sceneFrameCounter & (arg0 - 0x23))) {
                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 += 1;
                    } else {
                        gEntityInfo[arg0].xPosBg2 -= 1;
                    }
                    if (gEntityInfo[arg0].xPosBg2 < 0x50) {
                        gEntityInfo[arg0].xPosBg2 = 0x50;
                    }
                    if (gEntityInfo[arg0].xPosBg2 > 0x190) {
                        gEntityInfo[arg0].xPosBg2 = 0x190;
                    }
                }
                if (gEntityInfo[arg0].unk8.split.unk8 != 0) {
                    gEntityInfo[arg0].unk8.split.unk8 -= 1;
                }
                if (gEntityInfo[arg0].unk8.split.unk8 > 0x50) {
                    gEntityInfo[arg0].yPosBg2 -= 1;
                } else if (gEntityInfo[arg0].unk8.split.unk8 <= 0x3C) {
                    if (gEntityInfo[arg0].unk8.split.unk8 <= 0x28) {
                        if (gEntityInfo[arg0].yPosBg2 <= 0xD7) {
                            gEntityInfo[arg0].yPosBg2 += 1;
                        }
                    }
                }
                if (gEntityInfo[0x24].yPosBg2 <= 0xD7) {
                    break;
                }
                if (gEntityInfo[0x25].yPosBg2 <= 0xD7) {
                    break;
                }
                if (gEntityInfo[0x26].yPosBg2 <= 0xD7) {
                    break;
                }
                if (arg0 != 0x24) {
                    break;
                }
                if ((gUnk_03005220.unk31 == 0) && (gUnk_03005220.unk35 == 0)) {
                    break;
                }
                gUnk_03005400.unkE_4 = 1;
                (&gEntityInfo[0x24])->unkF = 3; // TODO: fix
                break;

            case 11:
                gEntityInfo[arg0].xPosBg2 = 0xF0 - ((8 - gBgInfo[1].hOfs) * 4);
                if ((s16)gUnk_03003590[0].unk0 > -0xA0) {
                    gUnk_03003590[0].unk0 -= 2;
                    gUnk_03003590[0].unk2 -= 2;
                }
                if (gEntityInfo[arg0].yPosBg2 < 0xE0) {
                    gEntityInfo[arg0].yPosBg2 += 2;
                } else {
                    gUnk_03005400.unkA = 0xC;
                }
                break;

            case 12:
                for (var_r2 = 0; var_r2 < (0xA - ((gUnk_03005400.unkC - 1) * 2)); var_r2++) {
                    gEntityInfo[var_r2 + 0x16].unkF = 0x18;
                }
                gUnk_03005400.unkA = 8;
                gEntityInfo[arg0].unkF = 0x1A;
                break;

            case 13:
                if (gUnk_03005400.unk6 == 0) {
                    gEntityInfo[arg0].yPosBg2 += 4;
                    if (gBgDataPtrs.pBufBg2Tilemap[(gEntityInfo[arg0].xPosBg2 >> 3)
                                                   + (((gEntityInfo[arg0].yPosBg2 - 8) >> 3) * gBgInfo[2].hLength)])
                        ; // FAKE
                    if ((gUnk_03004654[0x1B]
                         <= gBgDataPtrs.pBufBg2Tilemap[(gEntityInfo[arg0].xPosBg2 >> 3)
                                                       + (((gEntityInfo[arg0].yPosBg2 - 8) >> 3) * gBgInfo[2].hLength)])
                        && (gEntityInfo[arg0].yPosBg2 > 0x64)) {
                        if (gEntityInfo[arg0].xPosBg2 < gEntityInfo[0].xPosBg2) {
                            gEntityInfo[arg0].unkC_2 = 1;
                        } else {
                            gEntityInfo[arg0].unkC_2 = 0;
                        }
                        gUnk_03005400.unk6 = 1;
                        m4aSongNumStart(0x7F);
                        gUnk_03005400.unk8_0 = 1;
                    }
                }
                if (gUnk_03005400.unk6 == 1) {
                    if ((s16)gUnk_03003590[0].unk0 <= 0x7F) {
                        gUnk_03003590[0].unk0 += 4;
                        gUnk_03003590[0].unk2 += 4;
                    } else {
                        gUnk_03005400.unk6 = 2;
                    }
                    gUnk_03003590[0].unk4 += 4;
                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 += 1;
                    } else {
                        gEntityInfo[arg0].xPosBg2 -= 1;
                    }
                    gEntityInfo[arg0].yPosBg2 -= 1;
                }
                if (gUnk_03005400.unk6 == 2) {
                    if ((s16)gUnk_03003590[0].unk0 <= 0x9F) {
                        gUnk_03003590[0].unk0 += 4;
                        gUnk_03003590[0].unk2 += 4;
                    }
                    gUnk_03003590[0].unk4 += 4;
                    if (gEntityInfo[arg0].unkC_2 == 0) {
                        gEntityInfo[arg0].xPosBg2 += 1;
                    } else {
                        gEntityInfo[arg0].xPosBg2 -= 1;
                    }
                    gEntityInfo[arg0].yPosBg2 += 2;
                    if (gEntityInfo[arg0].yPosBg2 > 0x17B) {
                        gUnk_03005400.unk6 = 0;
                        gUnk_03005400.unk8_0 = 0;
                        gEntityInfo[arg0].priority = 1;
                        gUnk_03005400.unkA = 0;
                    }
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 3) {
        SpawnEntitiesForVision(arg0);
    }
}
/**
 * TransitionLevelVariant: ported from kleod TransitionLevelVariant.
 */
void TransitionLevelVariant(u8 arg0) {
    u8 var_sb;

    if ((gEntityInfo[0x18].unkF == arg0) && (gUnk_03005400.unkC > 2)) {
        return;
    }

    if (arg0 == 0) {
        if (gEntityInfo[0x18].unkF == 0) {
            return;
        }

        gUnk_03004C20.room = gUnk_03005400.unkC;

        for (var_sb = 0; var_sb < 6; var_sb++) {
            SetupOAMSprite(
                var_sb + 0x18, gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb + 0xB].unk28,
                gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb + 0xB].unk0[gUnk_03004C20.room - 1].unk0,
                gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb + 0xB].unk0[gUnk_03004C20.room - 1].unk2, 0, 0,
                gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb + 0xB].unk0[gUnk_03004C20.room - 1].unk5, arg0,
                gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][var_sb + 0xB].unk29);

            if ((gEntityInfo[var_sb + 0x18].unkC_4 == 3) || (gEntityInfo[var_sb + 0x18].unkC_4 == 1)) {
                DmaCopy16(3, &gUnk_08064868,
                          OBJ_VRAM0 + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb + 0xC].tileNum * 0x20),
                          0x200);
            } else {
                DmaCopy16(3, &gUnk_080B9468,
                          OBJ_VRAM0 + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[var_sb + 0xC].tileNum * 0x20),
                          0x200);
            }
        }
    } else {
        gEntityInfo[0x1D].unkF = 0x1C;
        gEntityInfo[0x1C].unkF = 0x1C;
        gEntityInfo[0x1B].unkF = 0x1C;
        gEntityInfo[0x1A].unkF = 0x1C;
        gEntityInfo[0x19].unkF = 0x1C;
        gEntityInfo[0x18].unkF = 0x1C;

        gEntityInfo[0x1D].unk10 = 0;
        gEntityInfo[0x1C].unk10 = 0;
        gEntityInfo[0x1B].unk10 = 0;
        gEntityInfo[0x1A].unk10 = 0;
        gEntityInfo[0x19].unk10 = 0;
        gEntityInfo[0x18].unk10 = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateLevelProgression);
/**
 * UpdatePlayerAlternate: per-frame update of the player in the alternate control mode, including the scene-transition input handling
 * at its tail.
 */
void UpdatePlayerAlternate(u8 arg0) {
    s8 temp_r1_4;
    s8 temp_r2_4;
    u32 var_r1_4;
    u8 var_r4_2;

    ApplyPlayerMovement(arg0, gUnk_03003620);
    gEntityInfo[arg0].affineHFlip_matrixNum = 3;
    gEntityInfo[arg0].affineDouble = 1;
    gUnk_03003590[0].unk5_0 = gEntityInfo[0x12].unkC_2;
    if (gUnk_03005400.unk0 != 0) {
        gUnk_03005400.unk0 -= 1;
    }

    if ((gEntityInfo[0].yPosBg2 > 0x14F) && (gUnk_03005220.hearts != 0)) {
        gUnk_03005220.hearts = 1;
        PlayerRespawnOrDeath(1);
    }
    UpdateLevelProgression();

    if ((gEntityInfo[0x13].yPosBg2 > 0x100) && (gEntityInfo[0x13].unkF != 0x1C)) {
        SpawnEntityAtPosition(gEntityInfo[0x13].xPosBg2, gEntityInfo[0x13].yPosBg2, 2, 0x13);
        gEntityInfo[0x13].unkF = 0x1C;
    }

    if ((gEntityInfo[0x14].yPosBg2 > 0x100) && (gEntityInfo[0x14].unkF != 0x1C)) {
        SpawnEntityAtPosition(gEntityInfo[0x14].xPosBg2, gEntityInfo[0x14].yPosBg2, 2, 0x14);
        gEntityInfo[0x14].unkF = 0x1C;
    }

    if (gEntityInfo[arg0].unkF == 14) {
        switch (gUnk_03005400.unkA) {
            case 0x0:
                gUnk_03005400.unk8_0 = 1;
                gUnk_03005400.unk9 = 0;
                gUnk_03003590[0].unk0 = -0xD0;
                gUnk_03003590[0].unk2 = -0xD0;
                gEntityInfo[arg0].yPosBg2 = 0;
                gEntityInfo[arg0].priority = 2;
                SetPaletteAnimEntry(arg0, 8);
                TransitionLevelVariant(0);
                gEntityInfo[0x12].unk8.split.unk8 = 0;

                gUnk_03005400.unkA = 1;
                gUnk_030007E0.unkC_0 = 0;
                gUnk_030007E0.unkC_4 = 0;
                gUnk_030007E0.unk6 = 0x78;
                gUnk_030007E0.unk0 = 0x78;
                gUnk_030007E0.unk8 = 0x46;
                gUnk_030007E0.unk2 = 0x46;
                gUnk_030007E0.unkA = 0x60;
                gUnk_030007E0.unk4 = 0x60;
                gUnk_03005400.unkE_2 = 1;
                gUnk_03005400.unkE_7 = 1;
                break;

            case 0x1:
                gEntityInfo[0x12].yPosBg2 = SIN(gEntityInfo[0x12].unk8.split.unk8++ + 0x8);
                gUnk_03003590[0].unk0 += 2;
                gUnk_03003590[0].unk2 += 2;

                if (gEntityInfo[0x12].unk8.split.unk8 < 0x50) {
                    gEntityInfo[0x12].priority = 2;
                } else {
                    gEntityInfo[0x12].priority = 1;
                }

                if (gEntityInfo[0x12].unk8.split.unk8 == 0x5A) {
                    gUnk_03005400.unkA = 2;
                    gUnk_03005400.unk14 = 0;
                    gBg2Alpha = 1;
                    gUnk_03005400.unk16 = 1;
                }
                break;

            case 0x2:
                gUnk_03003590[0].unk0 += 8;
                gUnk_03003590[0].unk2 += 8;
                if ((s16)gUnk_03003590[0].unk0 <= 0) {
                    gUnk_03003590[0].unk2 = 0;
                    gUnk_03003590[0].unk0 = 0;
                    gUnk_03005400.unkA = 3;
                    gUnk_03005400.unk16 = 2;
                }
                break;

            case 0x3:
                gUnk_03003590[0].unk0 -= 8;
                if (-((s16)gUnk_03003590[0].unk0) >= ((gBg2XMag + gUnk_081168DC[gUnk_03004C20.world - 1]) - 0x20)) {
                    SetPaletteAnimEntry(arg0, 0);
                    SetPaletteAnimEntry(0x17, 0);
                    gUnk_03005400.unkA = 4;
                    gUnk_03005400.unk16 = 4;
                }
                break;

            case 0x4:
                if (gEntityInfo[arg0].yPosBg2 > 0xA0) {
                    gEntityInfo[arg0].yPosBg2 -= 1;
                }

                if ((((s16)gUnk_03003590[0].unk0 >= 0) || ((s16)(gUnk_03003590[0].unk0 += 8) >= 0))
                    && (gEntityInfo[arg0].yPosBg2 <= 0xB0)) {
                    gUnk_03005400.unkA = 0xFF;
                    gUnk_03005400.unk0 = 0x40;
                    gUnk_03005400.unk16 = 6;
                }
                break;

            case 0xFF:
                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unk8_0 = 0;
                    gEntityInfo[arg0].unkF = 0;
                    gUnk_03005400.unkB = 4;
                    gUnk_03005400.unkA = 6;
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 0) {
        switch (gUnk_03005400.unkA) {
            case 0:
                temp_r1_4 = thunk_sub_080002A0() % 0x20;
                if (temp_r1_4 < gUnk_08116A4E[(gUnk_03005400.unkC > 2) ? 2 : 1][0]) {
                    gUnk_03005400.unk0 = gUnk_08116A4E[0][0];
                } else if (temp_r1_4 < (gUnk_08116A4E[(gUnk_03005400.unkC > 2) ? 2 : 1][0]
                                        + gUnk_08116A4E[(gUnk_03005400.unkC > 2) ? 2 : 1][1])) {
                    gUnk_03005400.unk0 = gUnk_08116A4E[0][1];
                } else if (temp_r1_4
                           < (gUnk_08116A4E[(gUnk_03005400.unkC > 2) ? 2 : 1][0] + gUnk_08116A4E[(gUnk_03005400.unkC > 2) ? 2 : 1][1]
                              + gUnk_08116A4E[(gUnk_03005400.unkC > 2) ? 2 : 1][2])) {
                    gUnk_03005400.unk0 = gUnk_08116A4E[0][2];
                } else {
                    gUnk_03005400.unk0 = gUnk_08116A4E[0][3];
                }

                if (gEntityInfo[arg0].xPosBg2 == 0xF0) {
                    gUnk_03005400.unk8_6 = 1;
                    gUnk_03005400.unk0 = 0;
                    gUnk_03005400.unk13 = 0;
                    m4aSongNumStart(0x6E);
                    gUnk_03005400.unkA = 9;
                } else {
                    gUnk_03005400.unkA = 0xE;
                }
                break;

            case 1:
                gUnk_03003590[0].unk0 += 2;
                gUnk_03003590[0].unk2 -= 2;

                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    TransitionLevelVariant(0x1C);
                    if (gUnk_03005400.unkC == 0) {
                        gUnk_03005400.unk8_4 = 1;
                        gUnk_03005400.unkA = 2;
                    } else {
                        for (var_r4_2 = 0; var_r4_2 < 2; var_r4_2++) {
                            if (gEntityInfo[var_r4_2 + 0x15].unkF != 0x1C) {
                                SpawnEntityAtPosition(gEntityInfo[var_r4_2 + 0x15].xPosBg2, gEntityInfo[var_r4_2 + 0x15].yPosBg2, 2,
                                                      var_r4_2 + 0x15);
                            }
                        }

                        gUnk_03005400.unk8_4 = 0;
                        gUnk_03005400.unk15 = 5 - gUnk_03005400.unkC;
                        gUnk_03005400.unk8_2 = 0;
                        gUnk_03005400.unkE_2 = 1;
                        gUnk_03005400.unkA = 0xF;
                    }
                }
                break;

            case 5:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 0) {
                    SetPaletteAnimEntry(arg0, 0);
                }

                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    + (SIN((gUnk_03004C20.sceneFrameCounter * 4) & 0xFF) >> 5);
                if ((gEntityInfo[0x15].unkF == 0x1C) && (gEntityInfo[0x16].unkF == 0x1C) && (gEntityInfo[0x13].unkF == 0x1C)
                    && (gEntityInfo[0x14].unkF == 0x1C)) {
                    gUnk_03005400.unkA = 6;
                    gUnk_03005400.unkB = 4;
                } else {
                    if (((gEntityInfo[arg0].xPosBg2 - 0x18) < gEntityInfo[0].xPosBg2)
                        && ((gEntityInfo[arg0].xPosBg2 + 0x18) > gEntityInfo[0].xPosBg2)) {
                        if (gUnk_03005400.unk13 == 1) {
                            SetPaletteAnimEntry(arg0, 3);
                            gUnk_03005400.unkA = 0xC;
                            gUnk_03005400.unk13 = 0;
                        } else {
                            goto block_89;
                        }
                    } else {
                        gUnk_03005400.unk13 = 1;
                    block_89:
                        if (gUnk_03005400.unk0 == 0) {
                            if ((gUnk_03005400.unkC <= 2) && ((thunk_sub_080002A0() % 100) < 50)) {
                                TransitionLevelVariant(0x1C);
                                gUnk_03005400.unk15 = 5 - gUnk_03005400.unkC;
                                gUnk_03005400.unkA = 0xF;
                            } else {
                                gUnk_03005400.unk8_6 = 1;
                                gUnk_03005400.unk0 = 0;
                                gUnk_03005400.unk13 = 0;
                                m4aSongNumStart(0x6E);
                                gUnk_03005400.unkA = 9;
                            }
                        }
                    }
                }
                break;

            case 12:
                if (gUnk_03005400.unk8_2 == 0) {
                    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                        gUnk_03005400.unk8_4 = 1;
                    } else {
                        gEntityInfo[arg0].yPosBg2 -= 1;
                    }
                } else {
                    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state != 1) {
                        SetPaletteAnimEntry(arg0, 1);
                        m4aSongNumStart(0x6D);
                        gUnk_03005400.unkE_1 = 1;
                        gUnk_03005400.unkD = 2;
                        if (gEntityInfo[arg0].xPosBg2 > 0x118) {
                            gBg2Alpha = 0xE;
                            gUnk_03005400.unkE_2 = 0;
                        }
                        if (gEntityInfo[arg0].xPosBg2 <= 0xC7) {
                            gBg2Alpha = 0xF2;
                            gUnk_03005400.unkE_2 = 0;
                        }

                        if (gUnk_03005220.unk31 != 0) {
                            gEntityInfo[0].yPosBg2 += (((gEntityInfo[0].xPosBg2 - 0xF0) * SIN(gBg2Alpha)) >> 8);
                            if ((s16)((gEntityInfo[0].xPosBg2 - 0xF0) * SIN(gBg2Alpha)) < 0) {
                                gEntityInfo[0].yPosBg2 -= 0x20;
                            }
                        }
                    }
                    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer != 0xFF) {

                    } else {
                        gUnk_03005400.unk8_4 = 0;
                        gUnk_03005400.unk8_2 = 0;
                        gUnk_03005400.unk13 = 0;
                        m4aSongNumStart(0x6E);
                        gUnk_03005400.unkA = 9;
                        gUnk_03005400.unk0 = 0x20;
                    }
                }
                break;

            case 9:
                gUnk_03005400.unk8_6 = 1;
                gUnk_03005400.unk8_0 = 1;
                if (gUnk_03005400.unk0 != 0) {

                } else {
                    gUnk_03003590[0].unk2 -= 8;
                    gUnk_03003590[0].unk0 += 8;
                    gEntityInfo[0x12].xPosBg2 += (gUnk_03005400.unk13 * (1 - ((gUnk_03004C20.sceneFrameCounter % 2) * 2)));
                    gUnk_03005400.unk13 += 4;
                    if (-((s16)gUnk_03003590[0].unk2) < ((gBg2YMag + gUnk_081168DC[gUnk_03004C20.world - 1]) - 0x20)) {

                    } else {
                        temp_r2_4 = thunk_sub_080002A0() % 0x20;
                        if (gEntityInfo[0].xPosBg2 <= 0xEF) {
                            gEntityInfo[arg0].unk8.all = 2;
                        } else {
                            gEntityInfo[arg0].unk8.all = 3;
                        }

                        if (temp_r2_4 <= 7) {
                            gUnk_03005400.unk0 = 0x78;
                        } else if (temp_r2_4 <= 0x17) {
                            gUnk_03005400.unk0 = 0xB4;
                        } else {
                            gUnk_03005400.unk0 = 0xF0;
                        }

                        gUnk_03005400.unkE_2 = 1;
                        gEntityInfo[arg0].yPosBg2 = 0;
                        gUnk_03005400.unkA = 0xB;
                    }
                }
                break;

            case 11:
                if (gUnk_03005400.unk0 != 0) {

                } else {
                    gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD]
                                                    .unk0[gEntityInfo[arg0].unk8.all - 1]
                                                    .unk2;
                    gEntityInfo[arg0].xPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD]
                                                    .unk0[gEntityInfo[arg0].unk8.all - 1]
                                                    .unk0;
                    gEntityInfo[arg0].unk8.all = gEntityInfo[arg0].xPosBg2;

                    gUnk_03005400.unkA = 0xA;
                    SetPaletteAnimEntry(arg0, 0);
                    TransitionLevelVariant(0);
                    m4aSongNumStart(0x6E);
                }
                break;

            case 10:
                gUnk_03003590[0].unk2 += 4;
                gUnk_03003590[0].unk0 -= 4;

                gEntityInfo[0x12].xPosBg2
                    = gEntityInfo[arg0].unk8.all + (gUnk_03005400.unk13 * (1 - ((gUnk_03004C20.sceneFrameCounter % 2) * 2)));

                if (gUnk_03005400.unk13 != 0) {
                    gUnk_03005400.unk13 -= 2;
                }

                if ((s16)gUnk_03003590[0].unk0 == 0) {
                    gUnk_03003590[0].unk0 = 0;
                    gUnk_03003590[0].unk2 = 0;
                    gUnk_03005400.unk8_6 = 0;
                    gUnk_03005400.unk8_0 = 0;
                    SetPaletteAnimEntry(arg0, 2);
                    gUnk_03005400.unk13 = 0;
                    gUnk_03005400.unkA = 0;
                }
                break;

            case 14:
                if (gUnk_03005400.unk13 == 4) {
                    gUnk_03005400.unkA = 5;
                } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer != 0xFF) {

                } else {
                    gEntityInfo[arg0].unkC_2 ^= 1;
                    SetPaletteAnimEntry(arg0, 2);
                    gUnk_03005400.unk13 += 1;
                }
                break;

            case 6:
                if (gEntityInfo[arg0].xPosBg2 < gEntityInfo[0].xPosBg2) {
                    gEntityInfo[arg0].unkC_2 = 0;
                } else {
                    gEntityInfo[arg0].unkC_2 = 1;
                }
                SetPaletteAnimEntry(arg0, 7);
                gUnk_03005400.unkA = 7;
                break;

            case 7:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer != 0xFF) {

                } else {
                    if (gUnk_03005400.unkB == 4) {
                        SetPaletteAnimEntry(arg0, 5);
                    } else if (gUnk_03005400.unkB == 8) {
                        SetPaletteAnimEntry(arg0, 9);
                    }
                    gUnk_03005400.unkA = gUnk_03005400.unkB;
                }
                break;

            case 8:
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    + (SIN((gUnk_03004C20.sceneFrameCounter * 4) & 0xFF) >> 5);
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer != 0xFF) {

                } else {
                    gUnk_03005400.unkA = 5;
                }
                break;

            case 4:
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    + (SIN((gUnk_03004C20.sceneFrameCounter * 4) & 0xFF) >> 5);
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer != 0xFF) {

                } else if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state == 5) {
                    gEntityInfo[0x16].unkF = 0x19;
                    gEntityInfo[0x15].unkF = 0x19;
                    m4aSongNumStart(0x6A);
                    SetPaletteAnimEntry(arg0, 6);
                } else {
                    gUnk_03005400.unkA = 0;
                }
                break;

            case 15:
                if ((s16)gEntityInfo[0x12].yPosBg2 > -0x20) {
                    gEntityInfo[0x12].yPosBg2 = (gEntityInfo[0x12].yPosBg2 - 1) & ~3;
                } else {
                    gUnk_03005400.unk13 = thunk_sub_080002A0() % 3;
                    gUnk_03003590[0].unk0 = gUnk_03003590[0].unk2 = 0x20 - (gBg2YMag + gUnk_081168DC[gUnk_03004C20.world - 1]);
                    gUnk_03005400.unk8_0 = 1;
                    gUnk_03005400.unk8_6 = 1;

                    gEntityInfo[0x12].xPosBg2 = gUnk_03005400.unk13 * 0xF0;
                    if (gUnk_03005400.unk13 != 1) {
                        gEntityInfo[0x12].xPosBg2 += ((thunk_sub_080002A0() % 2) * 0x20);
                    }
                    gEntityInfo[0x12].unk8.split.unk8 = 0;
                    gEntityInfo[0x12].priority = 2;

                    SetPaletteAnimEntry(arg0, 8);
                    gUnk_03005400.unk0 = 0x20;
                    gUnk_03005400.unkA = 0xD;
                }
                break;

            case 13:
                if (gUnk_03005400.unk0 != 0) {

                } else if ((gUnk_03004C20.sceneFrameCounter & 1) && (gEntityInfo[0x12].unk8.split.unk8 < (gUnk_03005400.unkC * 0xA))) {

                } else {
                    if (gUnk_03005400.unk13 == 0) {
                        if (gEntityInfo[0x12].unk8.split.unk8 < 0x40) {
                            gEntityInfo[0x12].xPosBg2 += 4;
                        } else if (gEntityInfo[0x12].unk8.split.unk8 < 0x60) {
                            gEntityInfo[0x12].xPosBg2 += 3;
                        }
                    } else if (gUnk_03005400.unk13 == 2) {
                        if (gEntityInfo[0x12].unk8.split.unk8 < 0x40) {
                            gEntityInfo[0x12].xPosBg2 -= 5;
                        } else if (gEntityInfo[0x12].unk8.split.unk8 < 0x60) {
                            gEntityInfo[0x12].xPosBg2 -= 2;
                        }
                    }

                    if (gEntityInfo[0x12].unk8.split.unk8 >= 0x20) {
                        if (gEntityInfo[0x12].unk8.split.unk8 == 0x20) {
                            m4aSongNumStart(0x6F);
                        } else if (gEntityInfo[0x12].unk8.split.unk8 < 0x40) {
                            if (gUnk_03004C20.sceneFrameCounter & 2) {
                                gUnk_03005400.unkE_1 = 1;
                                gUnk_03005400.unkD = 1;
                            }

                            gUnk_03003590[0].unk0 += 1;
                            gUnk_03003590[0].unk2 += 1;
                        } else {
                            if (gUnk_03004C20.sceneFrameCounter & 2) {
                                gUnk_03005400.unkE_1 |= 1;
                                gUnk_03005400.unkD = 3;
                            }

                            gUnk_03005400.unk8_0 = 0;
                            gUnk_03003590[0].unk0 += 0xA;
                            gUnk_03003590[0].unk2 += 0xA;
                        }
                    }

                    gEntityInfo[0x12].yPosBg2 = SIN(gEntityInfo[0x12].unk8.split.unk8++ + 0x8);
                    if (gEntityInfo[0x12].unk8.split.unk8 == 0x70) {
                        gEntityInfo[0x12].priority = 1;
                        gEntityInfo[0x12].yPosBg2 = 0;

                        gUnk_03005400.unk15 -= 1;
                        if ((s8)gUnk_03005400.unk15 == 0) {
                            gUnk_03005400.unkA = 9;
                            gUnk_03003590[0].unk2 = 0;
                            gUnk_03003590[0].unk0 = 0;
                        } else {
                            gUnk_03005400.unkA = 0xF;
                        }
                    }
                }
                break;

            case 2:
                if (gUnk_03005400.unk8_2 != 0) {
                    SetPaletteAnimEntry(arg0, 1);
                    gUnk_03005400.unkA = 3;
                }
                break;

            case 3:
                if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
                    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state == 0xA) {
                        if (gUnk_03005220.unk31 != 0) {
                            gUnk_03005400.unkE_4 = 1;
                            gEntityInfo[arg0].unkF = 3;
                        }
                    } else {
                        SetPaletteAnimEntry(arg0, 0xA);
                        m4aSongNumStart(0x81);
                    }
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 3) {
        gNewKeys = gHeldKeys = 0;
        SpawnEntitiesForVision(arg0);
    } else if (gEntityInfo[arg0].unkF == 4) {
        gNewKeys = gHeldKeys = 0;
    }
    {
        // TODO: put gUnk_08117110 data here
        u16 subroutine_arg0[8];
        memcpy(&subroutine_arg0, &gUnk_08117110, 0x10);
        if (gUnk_03005400.unkA == 0xD) {
            var_r1_4 = (gUnk_03004C20.sceneFrameCounter / 2) % 8;
        } else {
            var_r1_4 = (gUnk_03004C20.sceneFrameCounter / 8) % 8;
        }
        DmaCopy16(3, &gBgTilemapBufs[1][subroutine_arg0[var_r1_4] * 0x20], &gBgTilemapBufs[1][0x220], 0xC0);
    }
}
/**
 * BlitMapColumnStripToStageMap: DMA-copies a column/row of BG2 tilemap entries into the scrolling map buffer, used when streaming new
 * terrain rows at the screen edge.
 */
void BlitMapColumnStripToStageMap(u8 arg0, u8 arg1) {
    u8 *var_r3;
    u8 var_r4;
    void *var_r1;

    if (arg0 == 0xFF) {
        var_r3 = &gUnk_03003790[0][arg1];
        var_r1 = gBgDataPtrs.pBufBg2Tilemap + ((gBgInfo[2].hLength * 0x1F) + 0x3C);
        for (var_r4 = 0; var_r4 <= 0x1D; var_r4++) {
            DmaCopy16(3, var_r1, var_r3, 0x6);
            var_r3 += 0x40;
        }
    } else {
        var_r3 = &gUnk_03003790[0][arg1];
        var_r1 = gBgDataPtrs.pBufBg2Tilemap + ((arg0 * 6) + 0x3C);
        for (var_r4 = 0; var_r4 <= 0x1D; var_r4++) {
            DmaCopy16(3, var_r1, var_r3, 0x6);
            var_r1 += gBgInfo[2].hLength;
            var_r3 += 0x40;
        }
    }
}
void SetPaletteAnimEntry(s32, u8);

/**
 * SetEntityVisibility: toggles the boss/minigame entity set on or off.
 * When enabling (arg0 == 1) it activates the player/HUD entities, kicks off the
 * palette animation, and arms the flag entity (slot 0x1E). When disabling it
 * clears the boss flag, retires the flag entity, and deactivates entity slots
 * 0..0xC.
 */
void SetEntityVisibility(u8 arg0) {
    u8 var_r2;

    if (arg0 == 1) {
        gUnk_03005400.unkE_7 = 1;

        gEntityInfo[0].unkF = 0;
        gEntityInfo[0].unk10 = arg0;
        gEntityInfo[0x14].unkF = 0x19;
        gEntityInfo[0x13].unkF = 0x19;

        gUnk_03005220.unk3E = 0;
        if (gUnk_03005220.unk31 == 1) {
            SetPaletteAnimEntry(0, 0);
        } else {
            SetPaletteAnimEntry(0, 4);
        }

        if (gEntityInfo[0x1E].unk16 == 1) {
            gEntityInfo[0x1E].unkF = 0xE;
            gEntityInfo[0x1E].unk10 = 1;
            gEntityInfo[0x1E].unk16 = 0;
        }
    } else {
        gUnk_03005400.unkE_7 = 0;

        if (gEntityInfo[0x1E].unk10 == 1) {
            gEntityInfo[0x1E].unkF = 0x1C;
            gEntityInfo[0x1E].unk10 = 0;
            gEntityInfo[0x1E].unk16 = 1;
        }

        for (var_r2 = 0; var_r2 < 0xD; var_r2++) {
            gEntityInfo[var_r2].unkF = 0x1C;
            gEntityInfo[var_r2].unk10 = 0;
        }
    }
}
/**
 * UpdatePlayerSpecial: per-frame update of the player in special/minigame stages.
 */
void UpdatePlayerSpecial(u8 arg0) {
    struct Unk_0800BEF0 sp0;
    s32 spC;
    u32 temp_r1;
    u32 temp_r2;
    u32 temp_r5;
    u8 temp_r8;
    s32 tmp;

    if (gUnk_03005400.unk0 != 0) {
        gUnk_03005400.unk0 -= 1;
    }

    if (gEntityInfo[arg0].unkF == 14) {
        switch (gUnk_03005400.unkA) {
            case 0x0:
                gUnk_03003590[0].unk2 = -0x80;
                gUnk_03003590[0].unk0 = -0x80;
                gIntrTable.hBlank = HBlankScrollUpdate;
                gUnk_03005400.unkA = 1;
                SetPaletteAnimEntry(arg0, 8);
                SetPaletteAnimEntry(0x15, 0);

                gEntityInfo[arg0].priority = 1;
                gEntityInfo[0x1D].priority = 1;
                gEntityInfo[0x1C].priority = 1;
                gEntityInfo[0x17].priority = 1;
                gEntityInfo[0x16].priority = 1;
                gEntityInfo[0x15].priority = 1;

                gUnk_030007E0.unkC_0 = 0;
                gUnk_030007E0.unkC_4 = 0;
                gUnk_030007E0.unk6 = gEntityInfo[0x12].xPosBg2 - 0x78;
                gUnk_030007E0.unk0 = gEntityInfo[0x12].xPosBg2 - 0x78;
                gUnk_030007E0.unk8 = gEntityInfo[0x12].yPosBg2;
                gUnk_030007E0.unk2 = gEntityInfo[0x12].yPosBg2;
                gUnk_030007E0.unkA = 0x60;
                gUnk_030007E0.unk4 = 0x60;
                gUnk_03005400.unkE_7 = 1;
                break;

            case 0x1:
                gUnk_03003590[0].unk0 += 4;
                gUnk_03003590[0].unk2 += 4;
                if (gUnk_03003590[0].unk0 == 0) {
                    gUnk_03005400.unkA = 0xFF;
                    gUnk_03005400.unk0 = 0x3C;
                }
                break;

            case 0xFF:
                if (gUnk_03005400.unk0 == 0) {
                    gEntityInfo[arg0].unkF = 0;
                    gUnk_03005400.unkA = 2;
                    gUnk_030007E0.unkC_0 = 1;
                    gUnk_030007E0.unkC_4 = 0;
                    gUnk_030007E0.unkA = 0x60;

                    gEntityInfo[0x14].unkF = 0x19;
                    gEntityInfo[0x13].unkF = 0x19;

                    gBlendValue = BLEND_MAX;
                    REG_BLDCNT = BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG0;
                    REG_BLDALPHA = BLDALPHA_MAX;
                    REG_BLDY = BLDY_MAX;
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 0 || gEntityInfo[arg0].unkF == 26) {
        ApplyPlayerMovement(arg0, gUnk_03003620);
        switch (gUnk_03005400.unkA) {
            case 0:
                gEntityInfo[arg0].unk10 = 1;
                gEntityInfo[arg0].xPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk0;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2;
                gEntityInfo[arg0].unkF = 0;
                gUnk_03005400.unkA = 2;
                break;

            case 1:
                sp0.unk0 = gEntityInfo[arg0].xPosBg2;
                sp0.unk2 = gEntityInfo[arg0].yPosBg2;
                sp0.unk4 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk0;
                sp0.unk6 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2;
                sp0.unk8 = sp0.unk9 = 2;
                UpdateTextScroll(&spC, sp0);
                temp_r2 = spC;
                gEntityInfo[arg0].xPosBg2 = temp_r2;
                gEntityInfo[arg0].yPosBg2 = temp_r2 >> 0x10;
                if (((gEntityInfo[arg0].xPosBg2 >> 3)
                     == (gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[gUnk_03004C20.room - 1].unk0
                         >> 3))
                    && ((gEntityInfo[arg0].yPosBg2 >> 3)
                        == (gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD]
                                .unk0[gUnk_03004C20.room - 1]
                                .unk2
                            >> 3))) {
                    gUnk_03005400.unk0 = 0x1E;
                    gUnk_03005400.unkA = 0xD;
                }

                goto b;
                gUnk_03003590[0].unk0 = (COS((gUnk_03004C20.sceneFrameCounter * 8) & 0xFF) << 0x10) >> 0x12;
                gUnk_03003590[0].unk2 = (SIN((gUnk_03004C20.sceneFrameCounter * 8) & 0xFF) << 0x10) >> 0x13;
                break;

            case 13:
                if (gUnk_03005400.unk0 == 0) {
                    gUnk_03005400.unkA = 8;
                    gUnk_03003590[0].unk2 = 0;
                    gUnk_03003590[0].unk0 = 0;

                    gEntityInfo[arg0].unk10 = 0;
                    gEntityInfo[arg0].unkF = 0x1A;

                    gEntityInfo[0x1D].unkC_2 = 0;
                    gEntityInfo[0x1C].unkC_2 = 0;

                    gEntityInfo[arg0].yPosBg2 = 0;
                    gEntityInfo[arg0].xPosBg2 = 0;

                    gEntityInfo[0x1D].yPosBg2 = 0;
                    gEntityInfo[0x1D].xPosBg2 = 0;

                    gEntityInfo[0x1C].yPosBg2 = 0;
                    gEntityInfo[0x1C].xPosBg2 = 0;

                    REG_IE |= INTR_FLAG_HBLANK;
                    REG_DISPSTAT |= DISPSTAT_HBLANK_INTR;
                } else {
                b:
                    gUnk_03003590[0].unk0 = (COS((gUnk_03004C20.sceneFrameCounter * 8) & 0xFF) << 0x10) >> 0x12;
                    gUnk_03003590[0].unk2 = (SIN((gUnk_03004C20.sceneFrameCounter * 8) & 0xFF) << 0x10) >> 0x13;
                }
                break;

            case 2:
                gEntityInfo[arg0].unk8.split.unk8 += 1;
                if (gEntityInfo[arg0].unk8.split.unk8 == 0) {
                    gEntityInfo[arg0].unk8.split.unk9 += 1;
                }

                temp_r8 = gEntityInfo[arg0].unk8.split.unk9;
                if (temp_r8 >= ((gUnk_03005400.unkC * 2) + 1)) {
                    SetPaletteAnimEntry(arg0, 0xC);
                    gEntityInfo[arg0].unk8.split.unk9 = 0;
                    gUnk_03005400.unkA = 0xC;

                    temp_r1 = thunk_sub_080002A0() % 4;

                    gEntityInfo[0x18].unk8.split.unk8 = gUnk_08116A6E[temp_r1][3];
                    gEntityInfo[0x19].unk8.split.unk8 = gUnk_08116A6E[temp_r1][4];
                    gEntityInfo[0x1A].unk8.split.unk8 = gUnk_08116A6E[temp_r1][5];

                    gEntityInfo[0x18].xPosBg2 = gUnk_08116A6E[temp_r1][0] * 8;
                    gEntityInfo[0x19].xPosBg2 = gUnk_08116A6E[temp_r1][1] * 8;
                    gEntityInfo[0x1A].xPosBg2 = gUnk_08116A6E[temp_r1][2] * 8;

                    gEntityInfo[0x1A].unk11 = 0x1A;
                    gEntityInfo[0x19].unk11 = 0x1A;
                    gEntityInfo[0x18].unk11 = 0x1A;

                    gEntityInfo[0x1A].unkF = 0x19;
                    gEntityInfo[0x19].unkF = 0x19;
                    gEntityInfo[0x18].unkF = 0x19;
                    gUnk_03005400.unk16 = 3 - gUnk_03005400.unkC;
                    break;
                }

                if (gEntityInfo[arg0].xPosBg2 <= 0x40) {
                    gEntityInfo[arg0].unkC_2 = 0;
                }
                if (gEntityInfo[arg0].xPosBg2 > 0x19F) {
                    gEntityInfo[arg0].unkC_2 = 1;
                }

                gEntityInfo[arg0].xPosBg2 += 1 - (gEntityInfo[arg0].unkC_2 * 2);
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    - ((SIN(gEntityInfo[arg0].unk8.split.unk8) << 0x10) >> 0x15);
                tmp = temp_r8 & 1;
                if (tmp != 1) {
                    break;
                }

                if (gEntityInfo[arg0].xPosBg2 < gEntityInfo[0].xPosBg2) {
                    gEntityInfo[arg0].unkC_2 = 0;
                } else {
                    gEntityInfo[arg0].unkC_2 = 1;
                }

                gUnk_03005400.unkA = 3;
                gEntityInfo[arg0].unk8.split.unk9 += 1;
                break;

            case 3:
                gEntityInfo[arg0].unk8.split.unk8 += 1;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    - ((SIN(gEntityInfo[arg0].unk8.split.unk8) << 0x10) >> 0x15);

                if (gEntityInfo[0x18].unkF != 0x1C) {
                    break;
                }
                SetPaletteAnimEntry(arg0, 9);
                gEntityInfo[0x18].unkF = 0x19;
                break;

            case 12:
                gEntityInfo[arg0].unk8.split.unk8 += 1;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2
                    - ((SIN(gEntityInfo[arg0].unk8.split.unk8) << 0x10) >> 0x15);

                if (gEntityInfo[0x18].unk11 == 0x1A) {
                    break;
                }
                if (gEntityInfo[0x19].unk11 == 0x1A) {
                    break;
                }
                if (gEntityInfo[0x1A].unk11 == 0x1A) {
                    break;
                }
                if (gUnk_03005400.unk16 <= 0) {
                    gUnk_03005400.unkA = 2;
                    break;
                }

                temp_r1 = thunk_sub_080002A0() % 4;

                gEntityInfo[0x18].unk8.split.unk8 = gUnk_08116A6E[temp_r1][3];
                gEntityInfo[0x19].unk8.split.unk8 = gUnk_08116A6E[temp_r1][4];
                gEntityInfo[0x1A].unk8.split.unk8 = gUnk_08116A6E[temp_r1][5];

                gEntityInfo[0x18].xPosBg2 = gUnk_08116A6E[temp_r1][0] * 8;
                gEntityInfo[0x19].xPosBg2 = gUnk_08116A6E[temp_r1][1] * 8;
                gEntityInfo[0x1A].xPosBg2 = gUnk_08116A6E[temp_r1][2] * 8;

                gEntityInfo[0x1A].unk11 = 0x1A;
                gEntityInfo[0x19].unk11 = 0x1A;
                gEntityInfo[0x18].unk11 = 0x1A;

                gEntityInfo[0x1A].unkF = 0x19;
                gEntityInfo[0x19].unkF = 0x19;
                gEntityInfo[0x18].unkF = 0x19;

                SetPaletteAnimEntry(arg0, 0xC);
                gUnk_03005400.unk16 -= 1;
                break;

            case 8:
                if (gUnk_03005220.hearts == 0) {
                    break;
                }
                REG_BLDCNT = BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG0;
                if (gBlendValue <= 8) {
                    gUnk_03005400.unkA = 7;
                    gUnk_03005400.unk13 = 0;
                    SetPaletteAnimEntry(0x15, 5);
                    gEntityInfo[0x15].unkF = 0xE;
                    gUnk_030007E0.unkC_0 = 0;
                    gUnk_030007E0.unkC_4 = 0;
                    gUnk_030007E0.unk6 = 0x78;
                    gUnk_030007E0.unk8 = 0x50;
                    gUnk_030007E0.unkA = 0x40;
                    gUnk_030052A0 = gBg2Alpha;
                    m4aSongNumStart(0x73);
                    SetEntityVisibility(0);
                    gCallbackQueue.current[2] = UpdateScrollPosition;
                    break;
                }

                if ((gUnk_03004C20.sceneFrameCounter % 4) == 0) {
                    gBlendValue -= 1;
                }
                break;

            case 7:
                temp_r5 = gBlendValue << 0x18;
                if (gBlendValue != 0) {
                    if ((gUnk_03004C20.sceneFrameCounter % 4) == 0) {
                        gBlendValue -= 1;
                    }
                    break;
                }

                gUnk_03005400.unkA = 5;
                SetEntityVisibility(1);

                REG_BG1CNT = (REG_BG1CNT & ~3) | BGCNT_PRIORITY(3);
                REG_BG0CNT = (REG_BG0CNT & ~3) | BGCNT_PRIORITY(2);
                REG_BG2CNT = (REG_BG2CNT & ~3) | BGCNT_PRIORITY(1);
                REG_BLDCNT = 0;
                gBgInfo[1].hOfs = 0;
                gBgInfo[0].hOfs = 0;
                temp_r2 = temp_r5; // TODO: temp_r5 is fake
                REG_BG0HOFS = temp_r2 >> 0x1C;
                REG_BG1HOFS = 0;

                REG_IE &= ~INTR_FLAG_HBLANK;
                REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
                m4aSongNumStop(0x73);
                if (gUnk_03005400.unkC) {
                    gUnk_03005400.unk0 = 0x800;
                } else {
                    gUnk_03005400.unk0 = 0x800;
                }
                break;

            case 9:
                if (gUnk_03005220.hearts == 0) {
                    break;
                }
                REG_BG1CNT = (REG_BG1CNT & ~3) | BGCNT_PRIORITY(2);
                REG_BG0CNT = (REG_BG0CNT & ~3) | BGCNT_PRIORITY(3);
                REG_BG2CNT = (REG_BG2CNT & ~3) | BGCNT_PRIORITY(1);
                REG_BLDCNT = BLDCNT_TGT1_BG1 | BLDCNT_EFFECT_BLEND | BLDCNT_TGT2_BG0;
                REG_IE |= INTR_FLAG_HBLANK;
                REG_DISPSTAT |= DISPSTAT_HBLANK_INTR;

                if (gBlendValue > 8) {
                    gUnk_03005400.unkA = 0xA;
                    gUnk_030007E0.unkC_0 = 0;
                    gUnk_030007E0.unkC_4 = 0;
                    gUnk_030007E0.unk6 = 0x78;
                    gUnk_030007E0.unk8 = 0x50;
                    gUnk_030007E0.unkA = 0x40;
                    gUnk_030052A0 = gBg2Alpha;
                    m4aSongNumStart(0x73);
                    SetEntityVisibility(0);
                    gCallbackQueue.current[2] = UpdateScrollPosition;
                    break;
                }

                if (gUnk_03004C20.sceneFrameCounter & 3) {
                    break;
                }
                gBlendValue += 1;
                break;

            case 10:
                if (gBlendValue < BLEND_MAX) {
                    if (gUnk_03004C20.sceneFrameCounter & 3) {
                        break;
                    }
                    gBlendValue += 1;
                    break;
                }
                SetEntityVisibility(1);
                gBgInfo[1].hOfs = 0;
                gBgInfo[0].hOfs = 0;
                REG_BG0HOFS = 0;
                REG_BG1HOFS = 0;
                REG_IE &= ~INTR_FLAG_HBLANK;
                REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
                REG_BLDCNT = 0;

                gEntityInfo[arg0].xPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk0;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][arg0 - 0xD].unk0[0].unk2;
                gEntityInfo[0x1C].unkF = 3;
                gEntityInfo[0x1D].unkF = 4;

                SetPaletteAnimEntry(0x15, 5);
                m4aSongNumStop(0x73);

                if (gUnk_03005400.unkC == 0) {
                    gEntityInfo[0x15].unkF = 6;
                    gEntityInfo[0x15].unk8.split.unk8 = 0;
                    SpawnEntityAtPosition(gEntityInfo[0x13].xPosBg2, gEntityInfo[0x13].yPosBg2, 2, 0x13);
                    SpawnEntityAtPosition(gEntityInfo[0x14].xPosBg2, gEntityInfo[0x14].yPosBg2, 2, 0x14);
                    gEntityInfo[0x14].unkF = 0x1C;
                    gEntityInfo[0x13].unkF = 0x1C;
                    SetPaletteAnimEntry(0x15, 5);
                    gUnk_03005400.unkA = 6;
                    gEntityInfo[arg0].unkF = 0x1A;
                    gEntityInfo[arg0].unk10 = 0;
                } else {
                    gUnk_03005400.unkA = 0;
                    gEntityInfo[0x15].unkF = 0;
                    gEntityInfo[arg0].unkF = 0;
                }
                break;

            case 11:
                REG_BLDCNT = BLDCNT_TGT2_BG0 | BLDCNT_TGT2_BG1 | BLDCNT_EFFECT_BLEND;
                gBlendValue = 0xA;
                if (gUnk_03005220.unk31 != 0) {
                    gUnk_03005400.unkE_4 = 1;
                    gEntityInfo[arg0].unkF = 3;
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 3) {
        SpawnEntitiesForVision(arg0);
    }
}
/**
 * UpdateLevelScrollDMA: per-frame background palette/scroll animation cycle keyed off the scene frame counter (1800-frame loop). DMAs
 * palette banks from gUnk_0818B9F8 into BG_PLTT and paces the cycle speed via gEntityInfo[0x12] state.
 */
void UpdateLevelScrollDMA(void) {
    u16 temp_r6;

    temp_r6 = gUnk_03004C20.sceneFrameCounter % 1800;
    if (temp_r6 == 0) {
        gUnk_03005400.unk8_7 ^= 1;
    } else if (temp_r6 < 300) {
        DmaCopy16(3, gUnk_0818B9F8[1], BG_PLTT, 0x60);
    } else if (temp_r6 >= 300 && temp_r6 <= 1500) {
        if ((gUnk_03004C20.sceneFrameCounter % 8) == 0) {
            if (gEntityInfo[0x12].unk8.split.unk9 > 5) {
                gEntityInfo[0x12].unk8.split.unk9 -= 1;
            }
        }

        if ((gUnk_03004C20.sceneFrameCounter % gEntityInfo[0x12].unk8.split.unk9) == 0) {
            DmaCopy16(3, gUnk_0818B9F8[1 + (gUnk_03005400.unk8_7 * 4) + gEntityInfo[0x12].unk8.split.unk8], BG_PLTT, 0x60);
            gEntityInfo[0x12].unk8.split.unk8 = ((gEntityInfo[0x12].unk8.split.unk8 + 1) % 0x100u) % 4;
        }
    } else if (temp_r6 <= 1680) {
        if ((gUnk_03004C20.sceneFrameCounter % 8) == 0) {
            if (gEntityInfo[0x12].unk8.split.unk9 <= 0xF) {
                gEntityInfo[0x12].unk8.split.unk9 += 1;
            }
        }

        if ((gUnk_03004C20.sceneFrameCounter % gEntityInfo[0x12].unk8.split.unk9) == 0) {
            DmaCopy16(3, gUnk_0818B9F8[1 + (gUnk_03005400.unk8_7 * 4) + gEntityInfo[0x12].unk8.split.unk8], BG_PLTT, 0x60);
            gEntityInfo[0x12].unk8.split.unk8 = ((gEntityInfo[0x12].unk8.split.unk8 + 1) % 0x100u) % 4;
        }
    } else if (temp_r6 <= 1800) {
        DmaCopy16(3, gUnk_0818B9F8[1], BG_PLTT, 0x60);
    }

    if ((temp_r6 >= 300 && temp_r6 <= 1700) && ((gUnk_03004C20.sceneFrameCounter % (gEntityInfo[0x12].unk8.split.unk9 - 4)) == 0)
        && (gUnk_03005220.unk31 != 0)) {
        gEntityInfo->xPosBg2 += 0; // FAKE?
        if (gUnk_03005400.unk8_7 == 0) {
            gEntityInfo->xPosBg2 += 1;
        } else {
            gEntityInfo->xPosBg2 -= 1;
        }
    }

    if (gEntityInfo->xPosBg2 < 0x14) {
        gEntityInfo->xPosBg2 = 0x14;
    }
    if (gEntityInfo->xPosBg2 > 0x1C4) {
        gEntityInfo->xPosBg2 = 0x1C4;
    }

    if (gEntityInfo[0x14].unkF == 0) {
        if (gEntityInfo[0x14].xPosBg2 < 0x14) {
            gEntityInfo[0x14].xPosBg2 = 0x14;
            gEntityInfo[0x14].unkC_2 = 0;
        }
        if (gEntityInfo[0x14].xPosBg2 > 0x1C4) {
            gEntityInfo[0x14].xPosBg2 = 0x1C4;
            gEntityInfo[0x14].unkC_2 = 1;
        }
    }

    if (gEntityInfo[0x15].unkF == 0) {
        if (gEntityInfo[0x15].xPosBg2 < 0x14) {
            gEntityInfo[0x15].xPosBg2 = 0x14;
            gEntityInfo[0x15].unkC_2 = 0;
        }
        if (gEntityInfo[0x15].xPosBg2 > 0x1C4) {
            gEntityInfo[0x15].xPosBg2 = 0x1C4;
            gEntityInfo[0x15].unkC_2 = 1;
        }
    }
}
/**
 * UpdatePlayerFinalBoss: per-frame update of the player during the final boss.
 */
void UpdatePlayerFinalBoss(u8 arg0) {
    s32 var_r2_2;
    u32 var_r3_2;
    u32 var_r4;
    u32 temp_r0_8;
    u8 temp_r5_3;

    gEntityInfo[arg0].affineHFlip_matrixNum = 3;
    gUnk_03003590[0].unk5_0 = gEntityInfo[0x12].unkC_2;
    if (gUnk_030034E4 == 1) {
        return;
    }

    ApplyPlayerMovement(arg0, gUnk_03003620);
    UpdateLevelScrollDMA();

    if (gUnk_03005400.unk0 != 0) {
        gUnk_03005400.unk0 -= 1;
    }

    if (gEntityAnimationInfo[arg0 - gUnk_0300363C].state == 0x18) {
        if (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF) {
            SetPaletteAnimEntry(arg0, 0x19);
            gEntityInfo[arg0].unkC_2 = 1;
        }
    } else if ((gEntityAnimationInfo[arg0 - gUnk_0300363C].state == 0x19)
               && (gEntityAnimationInfo[arg0 - gUnk_0300363C].timer == 0xFF)) {
        SetPaletteAnimEntry(arg0, 0x18);
        gEntityInfo[arg0].unkC_2 = 0;
    }

    if (gEntityInfo[arg0].unkF == 14) {
        switch (gUnk_03005400.unkA) {
            case 0:
                gUnk_030007E0.unkC_0 = 3;
                gUnk_030007E0.unkC_4 = 0;
                gUnk_030007E0.unk6 = 0x78;
                gUnk_030007E0.unk8 = 0x80;
                gUnk_030007E0.unkA = 0;
                gUnk_03005400.unk8_0 = 1;
                gUnk_03005400.unk8_6 = 1;

                gUnk_03003590[0].unk2 = -gBg2XMag + 0x10;
                gUnk_03003590[0].unk0 = -gBg2XMag + 0x10;
                gUnk_03003590[4].unk2 = -gBg2XMag + 0x10;
                gUnk_03003590[4].unk0 = -gBg2XMag + 0x10;

                gIntrTable.hBlank = HBlankScrollUpdate;
                REG_IE &= ~INTR_FLAG_HBLANK;
                REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
                gUnk_03005400.unk0 = 0x40;
                gUnk_03005400.unkA = 1;
                SetPaletteAnimEntry(arg0, 0x18);
                break;

            case 1:
                if (gUnk_03005400.unk0 == 0) {
                    gEntityInfo[arg0].priority = 2;
                    gEntityInfo[0x21].priority = 2;
                    gEntityInfo[0x20].priority = 2;
                    gEntityInfo[0x1F].priority = 2;
                    gEntityInfo[0x13].priority = 1;
                    gEntityInfo[0x15].priority = 0;
                    gEntityInfo[0x14].priority = 0;
                    gEntityInfo[arg0].unkF = 0x1A;
                    gUnk_03005400.unkA = 1;
                    gEntityInfo[0x13].unkF = 0x19;
                }
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 26) {
        switch (gUnk_03005400.unkA) {
            case 0:
                gUnk_030007E0.unk8 = 0x80;
                gUnk_030007E0.unkA = 0;
                gUnk_03005400.unk0 = 0x78;
                gUnk_03005400.unkA = 1;
                break;

            case 1:
                gUnk_03003590[0].unk2 = -gBg2XMag + 0x10;
                gUnk_03003590[0].unk0 = -gBg2XMag + 0x10;

                gUnk_03003590[4].unk2 = -gBg2XMag + 0x10;
                gUnk_03003590[4].unk0 = -gBg2XMag + 0x10;

                gUnk_03003590[0].unk4 = 0;
                gUnk_03003590[4].unk4 = 0;

                if (gUnk_03005400.unk0 == 0) {
                    gEntityInfo[arg0].unkF = 0;
                    gEntityInfo[0x13].unkF = 0x19;
                    gUnk_03005400.unkA = 0;
                    gEntityInfo[arg0].yPosBg2 = 0x138;
                    gEntityInfo[arg0].priority = 2;
                    gEntityInfo[0x21].priority = 2;
                    gEntityInfo[0x20].priority = 2;
                    gEntityInfo[0x1F].priority = 2;
                    gEntityInfo[0x13].priority = 1;
                    gEntityInfo[0x15].priority = 0;
                    gEntityInfo[0x14].priority = 0;
                }
                break;

            case 3:
                if (gUnk_03005400.unkC == 2) {
                    gUnk_030007E0.unkC_0 = 3;
                    gUnk_030007E0.unkA = 0;
                    gUnk_030007E0.unk8 = 0x90;
                    gUnk_03005400.unk13 = gUnk_08116AA4[gUnk_03005400.unkC - 1];
                    gUnk_03005400.unkA = 2;
                } else {
                    gUnk_030007E0.unkA = 0x50;
                    gUnk_030007E0.unk8 = 0xA0;
                    gUnk_03005400.unk13 = gUnk_08116AA7[gUnk_03005400.unkC - 1];
                    gUnk_03005400.unkA = 5;
                }
                break;

            case 2:
                temp_r0_8 = thunk_sub_080002A0() % 5;
                for (var_r4 = 0; var_r4 < 6; var_r4++) {
                    if (gUnk_08116A86[temp_r0_8][var_r4] != 0xFF) {
                        gEntityInfo[0x19 + var_r4].xPosBg2 = (gUnk_08116A86[temp_r0_8][var_r4] << 5) + 0x20;
                        gEntityInfo[0x19 + var_r4].unkF = 0xE;
                    }
                }

                gUnk_03005400.unkA = 4;
                break;

            case 4:
                if ((gEntityInfo[0x19].unkF == 0x1C) && (gEntityInfo[0x1A].unkF == 0x1C) && (gEntityInfo[0x1B].unkF == 0x1C)
                    && (gEntityInfo[0x1C].unkF == 0x1C) && (gEntityInfo[0x1D].unkF == 0x1C) && (gEntityInfo[0x1E].unkF == 0x1C)) {
                    gUnk_03005400.unk13 -= 1;
                    if (gUnk_03005400.unk13 == 0) {
                        gUnk_03005400.unkA = 0;
                    } else {
                        gUnk_03005400.unkA = 2;
                    }
                }
                break;

            case 5:
                if (gUnk_030007E0.unk2 != gUnk_030007E0.unk8) {

                } else if ((u16)gUnk_030007E0.unk4 != (u16)gUnk_030007E0.unkA) {

                } else {
                    gEntityInfo[0x1F].unk8.split.unk8 = 0;
                    gEntityInfo[0x1F].unkF = 7;
                    gEntityInfo[0x1F].unk16 = 1;
                    SetPaletteAnimEntry(0x1F, 0xD);
                    gUnk_03005400.unkA = 6;
                }
                break;

            case 7:
                if ((gUnk_03004C20.sceneFrameCounter % 4) != 0) {

                } else if (gBlendValue < BLEND_MAX) {
                    gBlendValue += 1;
                } else {
                    gUnk_03005400.unkA = 8;
                    gEntityInfo[0x13].unkF = 0x10;
                }
                break;

            case 8:
                if ((gUnk_03004C20.sceneFrameCounter % 4) != 0) {

                } else if (gBlendValue != 0) {
                    gBlendValue -= 1;
                } else if (gUnk_03005220.unk31 == 0) {

                } else {
                    gUnk_03005400.unkA = 9;
                    gUnk_03005400.unkE_4 = 1;
                    REG_IE &= ~INTR_FLAG_HBLANK;
                    REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
                }
                break;

            case 9:
                gNewKeys = gHeldKeys = 0;
                SpawnEntitiesForVision(arg0);
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 0) {
        switch (gUnk_03005400.unkA) {
            case 0:
                gEntityInfo[arg0].xPosBg2 = gEntityInfo[0x13].xPosBg2;
                if (gEntityInfo[0x1F].unkF != 0xE) {

                } else {
                    if ((gUnk_03004C20.sceneFrameCounter % 3) == 0) {
                        gEntityInfo[arg0].yPosBg2 -= 1;
                    }

                    if (gBlendValue == BLEND_MAX) {
                        gUnk_03005400.unk16 = 0;
                        gUnk_03005400.unk4 = 0;
                        gUnk_03005400.unkA = 3;
                    }

                    if ((s16)gUnk_03003590[0].unk0 < 0) {
                        gUnk_03003590[0].unk0 += 8;
                        gUnk_03003590[0].unk2 += 8;
                    } else {
                        gUnk_03003590[0].unk0 = 0;
                        gUnk_03003590[0].unk2 = 0;
                    }
                }
                break;

            case 1:
                gUnk_03005400.unk8_0 = 1;
                SetPaletteAnimEntry(arg0, 0x18);

                if (gEntityInfo[0x1F].unkF == 0x11) {
                    gEntityInfo[0x1F].unkF = 0x12;
                } else {
                    gEntityInfo[0x21].unkF = 0x1A;
                    gEntityInfo[0x20].unkF = 0x1A;
                    gEntityInfo[0x1F].unkF = 0x1A;
                }

                gUnk_03005400.unkA = 2;
                gUnk_03005400.unk0 = 0x80;
                break;

            case 2:
                if (gUnk_03005400.unk0 == 1) {
                    DmaCopy16Wait(
                        3, &gUnk_080789C8,
                        OBJ_PLTT
                            + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum * 0x20),
                        0x20);

                    gEntityInfo[arg0].unkF = 0x1A;
                    if (gEntityInfo[0x14].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x14].xPosBg2, gEntityInfo[0x14].yPosBg2, 2, 0x14);
                    }
                    if (gEntityInfo[0x15].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x15].xPosBg2, gEntityInfo[0x15].yPosBg2, 2, 0x15);
                    }

                    if (gUnk_03005400.unkC != 0) {
                        gUnk_03005400.unkA = 3;
                    } else {
                        REG_BLDCNT = 0xBF;
                        gBlendValue = 0;
                        gUnk_03005400.unkA = 7;
                    }
                } else {
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 5) {
                        DmaCopy16Wait(
                            3, &gUnk_080789C8,
                            OBJ_PLTT
                                + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                   * 0x20),
                            0x20);
                    }
                    if ((gUnk_03004C20.sceneFrameCounter % 10) == 0) {
                        DmaFill16(3, 0x1F,
                                  OBJ_PLTT
                                      + (gUnk_0818B8E0[gUnk_03004C20.world - 1][gUnk_03004C20.level]->unk4[arg0 - 0xC].bpp_paletteNum
                                         * 0x20),
                                  0x20);
                    }
                }
                break;

            case 3:
                var_r2_2 = 0;
                if ((gUnk_03004C20.sceneFrameCounter % 2) == 0) {
                    gUnk_03005400.unk4 += 1;
                    if (gEntityInfo[0x14].unkF == 0x1C) {
                        var_r2_2 = 1;
                    }
                    if (gEntityInfo[0x15].unkF == 0x1C) {
                        var_r2_2 = 2;
                    }
                    if ((var_r2_2 != 0) && (gEntityInfo[0x17].unkF == 0x1C) && (gEntityInfo[0x18].unkF == 0x1C)
                        && (gEntityInfo[0x1F].unkF == 0) && (gEntityInfo[0x13].unkF == 0xF)) {
                        gEntityInfo[0x17].unkF = 0x19;
                    }

                    if (((u8)gUnk_03005400.unk4 % 0x80) == 0) {
                        temp_r5_3 = thunk_sub_080002A0() % 100;
                        if ((gUnk_03005400.unk16 == 0) && (temp_r5_3 > 50)) {
                            gUnk_03005400.unkA = 9;
                            break;
                        }

                        if ((gEntityInfo[0x13].unkF == 0xF) && (temp_r5_3 < (50 - ((gUnk_03005400.unkC - 1) * 0x14)))) {
                            m4aSongNumStart(0x83);
                            gEntityInfo[0x13].unkF = 1;
                            SetPaletteAnimEntry(0x13, 3);
                            gEntityInfo[0x13].unk8.split.unk9 |= 0x40;

                            temp_r5_3 = thunk_sub_080002A0() % 5;
                            for (var_r3_2 = 2; var_r3_2 < 5; var_r3_2++) {
                                if (gUnk_08116A86[temp_r5_3][var_r3_2] != 0xFF) {
                                    gEntityInfo[0x19 + var_r3_2].xPosBg2 = (gUnk_08116A86[temp_r5_3][var_r3_2] << 5) + 0x20;
                                    gEntityInfo[0x19 + var_r3_2].unkF = 0x10;
                                }
                            }
                        }
                    }
                    if ((gUnk_03004C20.sceneFrameCounter % 2) == 0) {
                        gEntityInfo[arg0].xPosBg2 = (SIN((u8)gUnk_03005400.unk4) >> 0x1) + 0xF0;
                    }
                }
                // case 4:                     /* switch 4 */

                goto block_145;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][6].unk0[0].unk2
                    - ((SIN((u8)gUnk_03005400.unk4) << 0x10) >> 0x15);
                break;

            case 9:
                if ((gUnk_03004C20.sceneFrameCounter % 4) == 0) {
                    if (gUnk_03005400.unk8_5 != 0) {
                        goto block_147;
                        gEntityInfo[arg0].xPosBg2 -= 1;
                    } else {
                        gEntityInfo[arg0].xPosBg2 += 1;
                    }
                }
                goto block_146;
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][6].unk0[0].unk2
                    - ((SIN((u8)gUnk_03005400.unk4) << 0x10) >> 0x15);
                break;

            case 5:
                gUnk_03005400.unk8_6 = 0;
                gUnk_03005400.unk8_0 = 0;
                if (gBlendValue < BLEND_MAX) {
                    REG_BLDCNT = BLDCNT_TGT1_ALL | BLDCNT_EFFECT_LIGHTEN;
                    if (gUnk_03004C20.sceneFrameCounter % 4) {

                    } else {
                        gBlendValue += 1;
                    }
                } else {
                    gEntityInfo[arg0].priority = 1;
                    gEntityInfo[0x1F].unkF = 0x10;
                    gEntityInfo[0x21].unkF = 0x1A;
                    gEntityInfo[0x20].unkF = 0x1A;
                    REG_IE &= ~INTR_FLAG_HBLANK;
                    REG_DISPSTAT &= ~DISPSTAT_HBLANK_INTR;
                    gUnk_03005400.unkA = 6;
                }
                break;

            case 6:
                if (gBlendValue != 0) {
                    gBlendValue -= 1;
                }

                gEntityInfo[arg0].xPosBg2 = gEntityInfo[0x1F].xPosBg2;
                gEntityInfo[arg0].yPosBg2 = gEntityInfo[0x1F].yPosBg2 - 4;
                if (gEntityInfo[0x1F].unkF == 0x12) {
                    gUnk_03005400.unkA = 7;
                }
                break;

            case 7:
                gUnk_03003590[0].unk0 -= 8;
                gUnk_03005400.unk8_6 = 1;
                gUnk_03005400.unk8_0 = 1;
                if (gEntityInfo[0x1F].unkF == 0x1C) {
                    if (gEntityInfo[0x14].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x14].xPosBg2, gEntityInfo[0x14].yPosBg2, 2U, 0x14U);
                    }
                    if (gEntityInfo[0x15].unkF != 0x1C) {
                        SpawnEntityAtPosition(gEntityInfo[0x15].xPosBg2, gEntityInfo[0x15].yPosBg2, 2U, 0x15U);
                    }
                    gEntityInfo[arg0].unkF = 0x1A;
                    gUnk_03005400.unkA = 0;
                }
                break;
                // TODO: fix this mess
            block_147:
                gEntityInfo[arg0].xPosBg2 -= 1;
            block_146:
                if (gEntityInfo[arg0].xPosBg2 < 0xE0 || gEntityInfo[arg0].xPosBg2 > 0x100) {
                    gUnk_03005400.unk16 = 1;
                    gUnk_03005400.unkA = 4;
                    gEntityInfo[0x21].unkF = 0xF;
                    gEntityInfo[0x20].unkF = 0xF;
                    gEntityInfo[0x1F].unkF = 0xF;
                }
            case 4:
            block_145:
                gEntityInfo[arg0].yPosBg2 = gUnk_080E2B64[gUnk_03004C20.world - 1][gUnk_03004C20.level - 1][6].unk0[0].unk2
                    - ((SIN((u8)gUnk_03005400.unk4) << 0x10) >> 0x15);
                break;
        }
    } else if (gEntityInfo[arg0].unkF == 3) {
        gNewKeys = gHeldKeys = 0;
    } else if (gEntityInfo[arg0].unkF == 4) {
        gNewKeys = gHeldKeys = 0;
    }

    gUnk_03003590[0].unk5_0 = gEntityInfo[0x12].unkC_2;
}
/**
 * Decompress: process a compressed asset's sub-header and decompress.
 *
 * If the first word of src has bit 31 set, the data uses two-stage
 * compression: first HuffUnComp, then LZ77. Otherwise, it's LZ77 only.
 * In both cases, data starts at src+4 (after the sub-header).
 */
void Decompress(void *dest, void *src) {
    void *heapPtr;

    if (((u32 *)src)[0] & 0x80000000) {
        heapPtr = thunk_HeapAlloc(((u32 *)src)[1] >> 8, 0);
        HuffUnComp((u8 *)src + 4, heapPtr);
        LZ77UnCompWram(heapPtr, dest);
        thunk_HeapFree(heapPtr);
    } else {
        LZ77UnCompWram((u8 *)src + 4, dest);
    }
}
/**
 * DecompressDma: decompress and DMA to palette or OBJ VRAM.
 *
 * Allocates a buffer sized from the source header, decompresses the data,
 * DMAs the result (skipping the 4-byte sub-header) as 16-bit transfers to
 * the destination, then frees the buffer.
 */
void DecompressDma(void *src, void *dest, u16 size) {
    void *heapPtr;

    heapPtr = thunk_HeapAlloc(((u32 *)src)[0] & 0x7FFFFFFF, 0);
    Decompress(heapPtr, src);
    DmaCopy16Wait(3, (u8 *)heapPtr + 4, dest, size);
    thunk_HeapFree(heapPtr);
}
/*
 * Allocates a buffer and decompresses data into it.
 * Reads the size from the first word of the source (masking off the top bit),
 * allocates that many bytes, then calls Decompress to fill the buffer.
 */
void *DecompressAlloc(void *src) {
    void *heapPtr;

    heapPtr = thunk_HeapAlloc(((u32 *)src)[0] & 0x7FFFFFFF, 0);
    Decompress(heapPtr, src);
    return heapPtr;
}
INCLUDE_ASM("asm/nonmatchings/code_3", InitSceneGraphics);
INCLUDE_ASM("asm/nonmatchings/code_3", RunSceneScript);
INCLUDE_ASM("asm/nonmatchings/code_3", RunTitleSequence);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateStageSelectScreen);
INCLUDE_ASM("asm/nonmatchings/code_3", DecompressAndLoadLevel); /* DecompressLZ77 */
INCLUDE_ASM("asm/nonmatchings/code_3", CheckTileCollisionRect);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateScrollPosition);
/**
 * RunVisionStartConfirmDelay: the confirm delay between picking a vision and loading it.
 *
 * NOTE: the name is a legacy misnomer — this has nothing to do with the pause menu (that is
 * HandlePauseMenuInput). It is installed as gCallbackQueue.current[1] by UpdatePlayerInput when A is
 * pressed on a vision node of the vision-select map, and it runs once per frame from then on.
 *
 * First frame (visionStartPending clear): play the confirm jingle (song 0x26), latch
 * visionStartPending and restart the scene frame counter. Subsequent frames: once the scene counter
 * passes 30, hand over to TransitionGameOver — the generic fade-out step that then queues the level
 * setup — and drop the latch.
 *
 * Verified at runtime (docs/dynamic-analysis/scripts/prove-vision-start-pending.mjs): the latch is
 * high for exactly the 31 frames between the vision-select map and the fade-out, three times in a
 * three-vision run, and stays 0 while the pause menu is open.
 *   no parameters
 *   no return value
 */
void RunVisionStartConfirmDelay(void) {
    if (gUnk_030034B0.visionStartPending == 0) {
        m4aSongNumStart(0x26);
        gUnk_030034B0.visionStartPending = 1;
        gUnk_03004C20.sceneFrameCounter = 0;
    } else if (gUnk_03004C20.sceneFrameCounter > 30) {
        gCallbackQueue.current[1] = TransitionGameOver;
        gUnk_030034B0.visionStartPending = 0;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_3", InitGameplayFromWorldMap);
/*
 * Runs the per-frame game update. If the pause flag at 0x030034E4 is zero,
 * calls four game subsystem update functions. Always calls UpdatePaletteAnimations
 * at the end regardless of pause state.
 *   no parameters
 *   no return value
 */
void GameUpdate(void) {
    if (gPauseFlag == 0) {
        UpdateWorldMapNodeState();
        UpdatePlayerInput();
        InitPlayerCollision();
        UpdateWorldMapCursor();
    }
    UpdatePaletteAnimations();
}
INCLUDE_ASM("asm/nonmatchings/code_3", SpawnLevelEntities);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerInput); /* UpdatePhysics */
INCLUDE_ASM("asm/nonmatchings/code_3", InitPlayerCollision); /* UpdateCollision */
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapCursor); /* UpdateCamera */
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateVisionStarIcons);
/* The world-map node record the selected vision lives on, and the BG2 rotation
 * angle at which that node faces the camera (byte 1 of the record, stored
 * negated). A macro rather than a local because the original recomputes the
 * lookup in every branch below; agbcc CSEs the shared address arithmetic on its
 * own, so hoisting it by hand only changes the register schedule. */
#define SELECTED_VISION_NODE                                                                                                          \
    gWorldMapNodes[gUnk_03004C20.world - 1][gWorldMapVisionNode[gUnk_03004C20.world - 1][gUnk_030034B0.selectedVision - 1]]
#define SELECTED_VISION_ANGLE    (-SELECTED_VISION_NODE[1])

/* The player's progress byte for the selected vision. */
#define SELECTED_VISION_PROGRESS gUnk_03004670->unk8[gUnk_03004C20.world - 1][gUnk_030034B0.selectedVision - 1]

/*
 * UpdateWorldMapNodeState: drives the world-map globe toward the currently
 * selected vision node.
 *
 * gUnk_030034B0.selectedVision holds the selected vision (1-based; 0 = world map not
 * active).  gWorldMapVisionNode[world-1][vision-1] indexes the per-node record table
 * gWorldMapNodes[world-1][]; byte 1 of that record is the BG2 rotation angle at
 * which the node faces the camera (stored negated).
 *
 * Each frame the routine compares the live rotation gBg2Alpha with that target
 * and fakes an L / R shoulder press in gHeldKeys so the normal world-map
 * rotation code spins the globe the short way round.  Once aligned it holds
 * both (L|R), runs a 0x80-frame arrival sequence (jingle + progress flag clear
 * at frame 0x40) and finally hands the selection to FindNextUnlockedVision.
 */
void UpdateWorldMapNodeState(void) {
    if (gUnk_030034B0.selectedVision == 0) {
        return;
    }

    if (gUnk_030034B0.visionArrivalTimer != 0) {
        gUnk_030034B0.visionArrivalTimer--;
    }

    if (gBg2Alpha == (u8)SELECTED_VISION_ANGLE) {
        /* Aligned: hold both shoulder buttons so the globe stops turning. */
        gHeldKeys = L_BUTTON | R_BUTTON;
        if (gUnk_030034B0.visionArrivalTimer == 0) {
            gUnk_030034B0.visionArrivalTimer = 0x80;
        }
        if (gUnk_030034B0.visionArrivalTimer == 0x40) {
            m4aSongNumStart(0x8B);
            SELECTED_VISION_PROGRESS &= 0x80;
            UpdateVisionStarIcons();
        }
        if (gUnk_030034B0.visionArrivalTimer == 1) {
            gUnk_030034B0.selectedVision = FindNextUnlockedVision();
        }
    } else if ((s8)(SELECTED_VISION_ANGLE - gBg2Alpha) < 0) {
        gHeldKeys = L_BUTTON;
    } else if ((s8)(SELECTED_VISION_ANGLE - gBg2Alpha) > 0) {
        gHeldKeys = R_BUTTON;
    }
}
/*
 * FindNextUnlockedVision: pick the world's next vision that is unlocked but not
 * yet cleared, and return it 1-based (0 = none left).
 *
 * gVisionUnlockMask[world][gUnk_030034B0.unk7_4] is the bitmask of visions this
 * world offers at the current world-map progress stage; bit i set = vision i + 1
 * exists.  gUnk_03004670->unk8[world - 1][i] is that vision's progress byte, and
 * 0x7F is the never-played sentinel in the low 7 bits (a score otherwise); UpdateWorldMapNodeState's `&= 0x80` is what CLEARS it, not
 * what leaves it behind.
 *
 * Note the row index is `world`, not `world - 1`, as in the original: every row of
 * gVisionUnlockMask is identical, so the off-by-one is unobservable for worlds 1..5.
 */
u8 FindNextUnlockedVision(void) {
    /* `i` must be u8, and the gVisionUnlockMask row must be re-read inside the
     * loop rather than hoisted out of it; either change costs the match. */
    u8 i;

    for (i = 0; i < 8; i++) {
        if (((gVisionUnlockMask[gUnk_03004C20.world][gUnk_030034B0.unk7_4] >> i) & 1)
            && gUnk_03004670->unk8[gUnk_03004C20.world - 1][i] == 0x7F) {
            return i + 1;
        }
    }
    return 0;
}
INCLUDE_ASM("asm/nonmatchings/code_3", SortEntityDrawOrder);
INCLUDE_ASM("asm/nonmatchings/code_3", SaveGameToSRAM);
INCLUDE_ASM("asm/nonmatchings/code_3", SaveGameWithVerify);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadGameFromSRAM);
INCLUDE_ASM("asm/nonmatchings/code_3", EraseAllSaveData);
INCLUDE_ASM("asm/nonmatchings/code_3", SavePlayerProgress);
/**
 * IsDpadUpHeld: returns 1 if D-pad Up (gHeldKeys bit 6, DPAD_UP) is held, 0 otherwise.
 * Reads the held state, not the edge, so the boot combo counts as long as Up is
 * down on the frame ConfigureInterruptsForGameplay runs.
 * Its single caller (ConfigureInterruptsForGameplay) latches the result into
 * gMinigamePlayerArmed, which arms the boot-menu minigame's player avatar.
 */
u32 IsDpadUpHeld(void) {
    if (gHeldKeys & DPAD_UP)
        return 1;
    return 0;
}
INCLUDE_ASM("asm/nonmatchings/code_3", ConfigureInterruptsForGameplay);
/**
 * UpdateBootMinigame: per-frame update of the hidden boot-menu minigame.
 *
 * Reached only through AgbMain's boot code (A+B+RIGHT+L+R held on frame 0), which
 * swaps callback slot 1 for MainGameFrameLoop; that loop calls this every frame on
 * top of the "Delete all save data?" screen. Entities 14..19 rain down the screen
 * (byte at +0x09 is each one's per-frame descent, redrawn as (rand % 3) + 2 when it
 * respawns off the bottom at y > 0xC0). If the run was armed by also holding Up at
 * boot (gMinigamePlayerArmed), Select spawns Klonoa at (0x78, 0x9C); R/L then slide
 * him 2px per frame within [0x10, 0xDF], and touching a falling enemy plays hit
 * animation 0x0C, which locks the controls until it ends.
 * See docs/dynamic-analysis/scripts/prove-minigame-player-armed.mjs.
 */
void UpdateBootMinigame(void) {
    u32 i;

    if ((gNewKeys & SELECT_BUTTON) && (gMinigamePlayerArmed == 1)) {
        gEntityInfo[0].unk10 = 1;
        gEntityInfo[0].xPosBg2 = 0x78;
        gEntityInfo[0].yPosBg2 = 0x9C;
        SetPaletteAnimEntry(0, 0x22);
    }
    /* state 0xC is the hit animation; it locks the controls until it ends. */
    if ((gEntityInfo[0].unk10 == 1) && (gEntityAnimationInfo[0].state != 0xC)) {
        if (gHeldKeys & R_BUTTON) {
            gEntityInfo[0].unkC_2 = 0;
            if (gEntityAnimationInfo[0].state != 1) {
                SetPaletteAnimEntry(0, 1);
            }
            if (gEntityInfo[0].xPosBg2 <= 0xDF) {
                gEntityInfo[0].xPosBg2 += 2;
            }
        } else if (gHeldKeys & L_BUTTON) {
            gEntityInfo[0].unkC_2 = 1;
            if (gEntityAnimationInfo[0].state != 1) {
                SetPaletteAnimEntry(0, 1);
            }
            if (gEntityInfo[0].xPosBg2 > 0x10) {
                gEntityInfo[0].xPosBg2 -= 2;
            }
        } else if (gEntityAnimationInfo[0].state != 0x22) {
            SetPaletteAnimEntry(0, 0x22);
        }
    }

    for (i = 0xE; i <= 0x13; i++) {
        switch (gEntityInfo[i].unkF) {
            case 0:
                gEntityInfo[i].yPosBg2 += gEntityInfo[i].unk8.split.unk9;
                if (gEntityInfo[i].yPosBg2 > 0xC0) {
                    gEntityInfo[i].unkF = 0x1C;
                }
                if (gEntityInfo[0].xPosBg2 - 0xC < gEntityInfo[i].xPosBg2 + 0xA
                    && gEntityInfo[0].xPosBg2 + 0xC > gEntityInfo[i].xPosBg2 - 0xA
                    && gEntityInfo[0].yPosBg2 - 0x18 < gEntityInfo[i].yPosBg2 - 8
                    && gEntityInfo[0].yPosBg2 > gEntityInfo[i].yPosBg2 - 0x14) {
                    SetPaletteAnimEntry(0, 0xC);
                }
                break;
            case 0x1C:
                gEntityInfo[i].xPosBg2 = ((thunk_sub_080002A0() % 6) * 40) + (thunk_sub_080002A0() % 0x28);
                gEntityInfo[i].yPosBg2 = 0;
                gEntityInfo[i].unk8.split.unk9 = (thunk_sub_080002A0() % 3) + 2;
                gEntityInfo[i].unkF = 0;
                gEntityInfo[i].unkC_2 = thunk_sub_080002A0() & 3;
                if (i <= 0x11) {
                    SetPaletteAnimEntry(i, 2);
                } else {
                    SetPaletteAnimEntry(i, 1);
                }
                break;
        }
    }
}
INCLUDE_ASM("asm/nonmatchings/code_3", MainGameFrameLoop);
INCLUDE_ASM("asm/nonmatchings/code_3", InitFadeTransition);
/* sub_08047F80 is a second, separate ROM function at 0x08047F80: it is never called
 * directly, only installed into callback slot 1 by UpdateScreenWipe below, so luvdis
 * saw no entry point for it and generate_asm.py folded it into UpdateScreenWipe's
 * INCLUDE_ASM slice (0x08047EC8..0x08048027, 0x160 bytes). Both are decompiled here. */
static void sub_08047F80(void);

void UpdateScreenWipe(void) {
    s32 h;
    s32 v;

    if (gUnk_03004D90.unk8 == 1) {
        h = gUnk_03004D90.unk4;
        if (h == 0xF0) {
            gUnk_03004D90.unk8 = 0;
            return;
        }
        h -= 0x4FB;
        gUnk_03004D90.unk4 = h;
        v = gUnk_03004D90.unk6;
        v -= 0x2FD;
        gUnk_03004D90.unk6 = v;
        REG_WIN1H = h;
        REG_WIN1V = v;
    }
    if (gUnk_03004D90.unk8 == 2) {
        h = gUnk_03004D90.unk4;
        if (h == 0x7878) {
            UpdateOamSortOrder();
            m4aSoundVSyncOn();
            m4aMPlayAllContinue();
            gCallbackQueue.current[1] = sub_08047F80;
            REG_BLDCNT = 0xD7;
            return;
        }
        h += 0x4FB;
        gUnk_03004D90.unk4 = h;
        v = gUnk_03004D90.unk6;
        v += 0x2FD;
        gUnk_03004D90.unk6 = v;
        REG_WIN1H = h;
        REG_WIN1V = v;
    }
    if ((gNewKeys & 0xF) && (gUnk_03004D90.unk8 == 0)) {
        gUnk_03004D90.unk8 = 2;
    }
}

static void sub_08047F80(void) {
    u32 i;

    if (gBlendValue == 0) {
        REG_WININ |= 0x20;
        gBlendValue = gUnk_030051F0.unkE;
        REG_BLDCNT = gUnk_030051F0.unk4;
        REG_BG0CNT = gUnk_030051F0.unk6;
        REG_BG1CNT = gUnk_030051F0.unk8;
        REG_BG2CNT = gUnk_030051F0.unkA;
        REG_BG3CNT = gUnk_030051F0.unkC;
        gUnk_03004C20.sceneFrameCounter = gUnk_030051F0.unk0;
        for (i = 0; i < 10; i++) {
            gCallbackQueue.next[i] = gCallbackQueue.previous[i];
        }
        gCallbackQueue.current[gCallbackQueue.currentCount - 1] = NULL;
        gCallbackQueue.nextCount = gCallbackQueue.previousCount;
        return;
    }
    REG_WININ = 1;
    REG_WINOUT = 0x3F;
    if ((gUnk_03004C20.globalFrameCounter & 3) == 0) {
        gBlendValue -= 1;
    }
}
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapLogic);
