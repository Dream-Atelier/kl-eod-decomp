#include "global.h"
#include "globals.h"
#include "include_asm.h"
#include "structs/variables.h"

/* ── kleod code_08039D8C.c shared scaffolding ─────────────────────────────
 * ROM data tables, common globals, cross-module externs, and forward decls
 * for the logic functions ported from kleod's code_08039D8C.c. */
#define BLEND_MAX 16
#define OBJ_PLTT  ((void *)0x05000200)
#define BG_VRAM   ((void *)0x06000000)
#define BG_PLTT   ((void *)0x05000000)

extern struct BgDataPtrs gBgDataPtrs; /* 0x03004790 */
extern u8 gBlendValue; /* 0x03005498 */
extern struct Unk_03004C08 gUnk_03004C08;
extern u8 *gUnk_03004658;
extern struct Unk_0803D4AC gUnk_081168E8[];
extern u8 gUnk_03004D9C;
extern struct Unk_0803D4AC gUnk_03003620;
extern u8 gUnk_030034C0;
extern struct Unk_030034B0 gUnk_030034B0;
extern u8 gUnk_030007CC;
extern u8 gUnk_03003D16[][8];
extern u8 gUnk_03003DD6[][8];
extern u8 gUnk_03003E96[][8];
extern u8 gUnk_03003F56[][8];

/* Tile-collision query result (kleod CheckTileCollisionSloped). */
struct Unk_08014184 {
    u16 unk0;
    u8 unk2;
    u8 pad3[0x4 - 0x3];
};

/* Cross-module functions (defined in code_1). */
extern struct Unk_08014184 *CheckTileCollisionSloped(struct Unk_08014184 *, u16, u16, u8);
extern void PlayerRespawnOrDeath(s32);
extern void SpawnEntityAtPosition(u16, u16, u8, u8);

/* ROM data tables. */
extern u8 gUnk_08064868[0x200];
extern const u8 gUnk_08078508[0x20];
extern u8 gUnk_08078628[0x20];
extern u8 gUnk_08078728[0x20];
extern u8 gUnk_08078768[0x20];
extern u8 gUnk_080789C8[0x20];
extern u8 gUnk_080B9468[0x200];
extern u8 gUnk_080D8C30[6][0x40];
extern const u8 gUnk_081166F8[4][4];
extern const u16 gUnk_08116728[8][2];
extern const u8 gUnk_08116780[8][0x20];
extern const s8 gUnk_08116888[6][2];
extern const u8 gUnk_081168DC[6];
extern const u8 gUnk_081168E2[4];
extern const u8 gUnk_081169F9[3][3];
extern const u8 gUnk_08116A02[4][5];
extern u8 gUnk_08116A46[4][2];
extern u16 gUnk_08116A4E[4][4];
extern const u8 gUnk_08116A6E[4][6];
extern const u8 gUnk_08116A86[5][6];
extern const u8 gUnk_08116AA4[3];
extern const u8 gUnk_08116AA7[3];
extern const u8 gUnk_0811710A[6];
extern u16 gUnk_08117110[8];
extern const void *gUnk_0818B9F8[];
extern u32 gUnk_082EAF8C;
extern u32 gUnk_082EB488;
extern u32 gUnk_082EB5B8;
extern u32 gUnk_082EBB20;
extern u32 gUnk_082EBC68;
extern u32 gUnk_082EC1A4;
extern u32 gUnk_082EC2E4;
extern u32 gUnk_082EC7C8;
extern u32 gUnk_082EC8F4;
extern u32 gUnk_082ECD74;
extern u32 gUnk_083128F8;
extern u32 gUnk_08312A58;
extern u32 gUnk_08312B70;
extern u32 gUnk_08312BD8;
extern u32 gUnk_08313C34;
extern u32 gUnk_08313F24;
extern u32 gUnk_083141F0;
extern u32 gUnk_083142EC;
extern u32 gUnk_083155C4;

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

/* World-map node/tile ROM lookup tables (kleod-canonical). */
extern const u8 gUnk_08116708[8][4];
extern const u8 gUnk_08116748[7][8];
extern const u8 gUnk_08116880[8];

/* Forward decls for world-map cluster + callbacks referenced across definitions. */
u8 CheckWorldCompletion(u8);
void CopyWorldMapTiles(u8);
void SetWorldMapTilePalette(u8, u8);
void UpdateWorldMapNodeTile(u8);
void CountCollectedGems(void);
void UpdateWorldMapNodeAnim(void);
void GameplayMainLoop(void);
void InitGameplayState(void);

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
INCLUDE_ASM("asm/nonmatchings/code_3", sub_0803A8B8);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapInput);
INCLUDE_ASM("asm/nonmatchings/code_3", CheckWorldCompletion);
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
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerBoss);
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
INCLUDE_ASM("asm/nonmatchings/code_3", ResetEntityTypesOnDeath);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerMinigame);
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
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerAlternate);
INCLUDE_ASM("asm/nonmatchings/code_3", HandleSceneTransitionInput);
INCLUDE_ASM("asm/nonmatchings/code_3", DecompressRowToTilemap);
void SetPaletteAnimEntry(u32, u8);

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
