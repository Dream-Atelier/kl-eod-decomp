#include "global.h"
#include "globals.h"
#include "include_asm.h"

INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World1_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World1_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World2_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World2_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World3_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World3_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World4_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World4_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World5_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World5_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World6_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World6_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World7_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World7_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World8_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World8_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World9_Vision1);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World9_Vision2);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_BossArena);
INCLUDE_ASM("asm/nonmatchings/code_3", InitGameplayState);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateOamSortOrder);
INCLUDE_ASM("asm/nonmatchings/code_3", ProcessInputAndUpdateEntities);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapInput);
INCLUDE_ASM("asm/nonmatchings/code_3", CheckWorldCompletion); /* IsEntityActive */
INCLUDE_ASM("asm/nonmatchings/code_3", CopyWorldMapTiles); /* UpdateEntityState */
INCLUDE_ASM("asm/nonmatchings/code_3", SetWorldMapTilePalette);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapNodeTile); /* UpdateEntityAnimation */
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
INCLUDE_ASM("asm/nonmatchings/code_3", CountCollectedGems);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapNodeAnim);
INCLUDE_ASM("asm/nonmatchings/code_3", RunWorldMapTransition);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateAllEntities);
INCLUDE_ASM("asm/nonmatchings/code_3", GameplayMainLoop);
INCLUDE_ASM("asm/nonmatchings/code_3", InitLevelState);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateEntitySpawnState);
INCLUDE_ASM("asm/nonmatchings/code_3", SpawnEntitiesForVision);
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
INCLUDE_ASM("asm/nonmatchings/code_3", ComputeScrollLimits);
INCLUDE_ASM("asm/nonmatchings/code_3", ApplyPlayerMovement);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerNormal);
INCLUDE_ASM("asm/nonmatchings/code_3", SetupEntitySpawnTable);
/**
 * RollRandomLevelVariant: select a random level variant based on difficulty.
 *
 * Reads the current difficulty level, computes a variant index using
 * difficulty parity and a random modulo, and stores it to the level state.
 */
void RollRandomLevelVariant(void) {
    u8 *state = gGameFlags;
    u8 difficulty = state[0x0C];
    register u32 d asm("r4") = (u8)(difficulty - 1);
    u32 rng;
    register u8 *levelState asm("r6");
    register u32 parity asm("r5");
    u8 randByte;
    u32 variant;
    rng = thunk_sub_080002A0();
    levelState = gControlBlock;
    parity = 1;
    parity &= d;
    randByte = (u8)rng;
    rng = (u8)d;
    variant = sub_0805193C(randByte, 5 - rng);
    parity = parity + variant + 1;
    levelState[0x0E] = parity;
}
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerBoss);
INCLUDE_ASM("asm/nonmatchings/code_3", ConfigureEntityBehavior);
INCLUDE_ASM("asm/nonmatchings/code_3", ResetEntityTypesOnDeath);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerMinigame);
INCLUDE_ASM("asm/nonmatchings/code_3", TransitionLevelVariant);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateLevelProgression);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerAlternate);
INCLUDE_ASM("asm/nonmatchings/code_3", HandleSceneTransitionInput);
INCLUDE_ASM("asm/nonmatchings/code_3", DecompressRowToTilemap);
INCLUDE_ASM("asm/nonmatchings/code_3", SetEntityVisibility);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerSpecial);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateLevelScrollDMA);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerFinalBoss);
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
INCLUDE_ASM("asm/nonmatchings/code_3", InitPauseMenu);
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
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapNodeState); /* UpdateGameLogic */
INCLUDE_ASM("asm/nonmatchings/code_3", FindNextUnlockedVision);
INCLUDE_ASM("asm/nonmatchings/code_3", SortEntityDrawOrder);
INCLUDE_ASM("asm/nonmatchings/code_3", SaveGameToSRAM);
INCLUDE_ASM("asm/nonmatchings/code_3", SaveGameWithVerify);
INCLUDE_ASM("asm/nonmatchings/code_3", LoadGameFromSRAM);
INCLUDE_ASM("asm/nonmatchings/code_3", SaveGameRetryLoop);
INCLUDE_ASM("asm/nonmatchings/code_3", SavePlayerProgress);
/**
 * IsSelectButtonPressed: returns 1 if Select (bit 6) is held, 0 otherwise.
 */
u32 IsSelectButtonPressed(void) {
    if (gKeysPrevious & 0x40)
        return 1;
    return 0;
}
INCLUDE_ASM("asm/nonmatchings/code_3", ConfigureInterruptsForGameplay);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerEntity);
INCLUDE_ASM("asm/nonmatchings/code_3", MainGameFrameLoop);
INCLUDE_ASM("asm/nonmatchings/code_3", InitFadeTransition);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateScreenWipe);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapLogic);
