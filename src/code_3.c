#include "global.h"
#include "globals.h"
#include "include_asm.h"
#include "structs/variables.h"
extern struct EntityAnimationInfo gEntityAnimationInfo[];
extern void SetupBG3WindowOverlay(void);
extern void UpdateScrollPosition(void);
#ifndef OAM_SIZE
#define OAM_SIZE 0x400
#endif
#ifndef BLDCNT_TGT1_ALL
#define BLDCNT_TGT1_ALL (BLDCNT_TGT1_BG0 | BLDCNT_TGT1_BG1 | BLDCNT_TGT1_BG2 | BLDCNT_TGT1_BG3 | BLDCNT_TGT1_OBJ | BLDCNT_TGT1_BD)
#endif
extern void VBlankCallback_Dialog(void);
/* AUTOPORT-SYMS */
void ReadKeyInput(void);
void VBlankCallback_Gameplay(void);
void AnimatePaletteEffects(void);
void IntroScrollAnimation(void);
extern u8 gUnk_03003790[][0x40];
extern void *gUnk_030052AC;
extern void *gUnk_030034F4;
extern u8 gUnk_0805D1E8[0x800];
extern void *gUnk_0818B800[6][7];
extern u8 gUnk_080A4888[0x800];
extern u8 gUnk_080A5888[0x800];
extern u8 gUnk_0805C9E8[0x800];
extern u8 gUnk_0805C968[0x80];
extern u8 gUnk_0805C8E8[0x80];
extern u8 gUnk_0805C6E8[0x200];
extern u8 gUnk_08077E48[0x20];
extern u8 gUnk_08077E28[0x20];
extern u8 gUnk_08078A68[0x20];
extern u8 gUnk_080657A8[0x20];
extern u8 gUnk_08065788[0x20];
extern u8 gUnk_08065768[0x20];
extern u8 gUnk_08078A48[0x20];
extern u8 gUnk_08078A28[0x20];
extern u8 gUnk_080789E8[0x20];
extern u8 gUnk_08078608[0x20];
extern u8 gUnk_080789A8[0x20];
extern u8 gUnk_08078988[0x20];
extern u8 gUnk_08078968[0x20];
extern u8 gUnk_08078948[0x20];
extern u8 gUnk_08065568[0x200];
extern u8 gUnk_08078908[0x20];
extern u8 gUnk_080635E8[0x80];
extern u8 gUnk_08062148[0x100];
extern u8 gUnk_080623C8[0x400];
extern u8 gUnk_080785C8[0x20];
extern u8 gUnk_08062348[0x80];
extern u8 gUnk_08065368[0x200];
extern u8 gUnk_08065168[0x200];
extern u8 gUnk_08064F68[0x200];
extern u8 gUnk_080788E8[0x20];
extern u8 gUnk_08064E68[0x100];
extern u8 gUnk_08064C68[0x200];
extern u8 gUnk_080788C8[0x20];
extern u8 gUnk_080788A8[0x20];
extern u8 gUnk_08061DA8[0x20];
extern u8 gUnk_08061D88[0x20];
extern u8 gUnk_08061D68[0x20];
extern u8 gUnk_08061D48[0x20];
extern u8 gUnk_08061D28[0x20];
extern u8 gUnk_080630E8[0x80];
extern u8 gUnk_08062848[0x80];
extern u8 gUnk_08064468[0x400];
extern u8 gUnk_08061C28[0x100];
extern u8 gUnk_08078868[0x20];
extern u8 gUnk_08062248[0x100];
extern u8 gUnk_08060608[0x100];
extern u8 gUnk_08060708[0x100];
extern u8 gUnk_08060808[0x200];
extern u8 gUnk_08078488[0x20];
extern u8 gUnk_0805FE08[0x800];
extern u8 gUnk_08078468[0x20];
extern u8 gUnk_0805F788[0x80];
extern u8 gUnk_08078848[0x20];
extern u8 gUnk_08062AE8[0x200];
extern u8 gUnk_080787E8[0x20];
extern u8 gUnk_08062AC8[0x20];
extern u8 gUnk_08062CE8[0x200];
extern u8 gUnk_080628C8[0x200];
extern u8 gUnk_080786A8[0x20];
extern u8 gUnk_08063168[0x200];
extern u8 gUnk_08062EE8[0x200];
extern u8 gUnk_0805FB08[0x100];
extern u8 gUnk_08078428[0x20];
extern u8 gUnk_0805FA08[0x100];
extern u8 gUnk_08062048[0x100];
extern u8 gUnk_08061FC8[0x80];
extern u8 gUnk_08064A68[0x200];
extern u8 gUnk_080633E8[0x200];
extern u8 gUnk_08063368[0x80];
extern u8 gUnk_0805F488[0x80];
extern u8 gUnk_080783C8[0x20];
extern u8 gUnk_08061DC8[0x200];
extern u8 gUnk_08078588[0x20];
extern u8 gUnk_0805F388[0x80];
extern u8 gUnk_0805F368[0x20];
extern u8 gUnk_08078388[0x20];
extern u8 gUnk_0805F2E8[0x80];
extern u8 gUnk_08078368[0x20];
extern u8 gUnk_0805F808[0x200];
extern u8 gUnk_0805F708[0x80];
extern u8 gUnk_08078408[0x20];
extern u8 gUnk_080627C8[0x80];
extern u8 gUnk_08064068[0x400];
extern u8 gUnk_08078888[0x20];
extern u8 gUnk_08063FE8[0x80];
extern u8 gUnk_08063BE8[0x400];
extern u8 gUnk_08063AE8[0x100];
extern u8 gUnk_080787C8[0x20];
extern u8 gUnk_0805F508[0x200];
extern u8 gUnk_080783E8[0x20];
extern u8 gUnk_08063A68[0x80];
extern u8 gUnk_08063868[0x200];
extern u8 gUnk_080787A8[0x20];
extern u8 gUnk_0805FC08[0x200];
extern u8 gUnk_08078448[0x20];
extern u8 gUnk_08061A28[0x200];
extern u8 gUnk_08078568[0x20];
extern u8 gUnk_0805EEE8[0x200];
extern u8 gUnk_08078328[0x20];
extern u8 gUnk_0805ECE8[0x200];
extern u8 gUnk_08078308[0x20];
extern u8 gUnk_0805F408[0x80];
extern u8 gUnk_080783A8[0x20];
extern u8 gUnk_08078788[0x20];
extern u8 gUnk_08078748[0x20];
extern u8 gUnk_0805F0E8[0x200];
extern u8 gUnk_08078348[0x20];
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
extern u8 gUnk_08061088[0x800];
extern u8 gUnk_080784E8[0x20];
extern u8 gUnk_08060A88[0x600];
extern u8 gUnk_080784C8[0x20];
extern u8 gUnk_08060A08[0x80];
extern u8 gUnk_080784A8[0x20];

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
INCLUDE_ASM("asm/nonmatchings/code_3", LoadLevel_World7_Vision1);
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
        UpdateHUDTimerAndLives();
    }

    DmaCopy16(3, &gBgTilemapBufs[gUnk_030034BC], gBgInfo[gUnk_030034BC].pTilemap, 0x800);

    if (gUnk_03004C20.unk10 == 1) {
        UpdateHUDTimerAndLives();
    }

    REG_IE |= INTR_FLAG_VBLANK;
    REG_DISPSTAT |= DISPSTAT_VBLANK_INTR;
}
INCLUDE_ASM("asm/nonmatchings/code_3", ProcessInputAndUpdateEntities);
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
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateWorldMapNodeAnim);
INCLUDE_ASM("asm/nonmatchings/code_3", RunWorldMapTransition);
INCLUDE_ASM("asm/nonmatchings/code_3", UpdateAllEntities);
INCLUDE_ASM("asm/nonmatchings/code_3", GameplayMainLoop);
INCLUDE_ASM("asm/nonmatchings/code_3", InitLevelState);
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
/**
 * DecompressRowToTilemap: DMA-copies a column/row of BG2 tilemap entries into the scrolling map buffer, used when streaming new
 * terrain rows at the screen edge.
 */
void DecompressRowToTilemap(u8 arg0, u8 arg1) {
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
INCLUDE_ASM("asm/nonmatchings/code_3", UpdatePlayerSpecial);
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
