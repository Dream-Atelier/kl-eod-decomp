# Symbol Map

Semantic names for not-yet-decompiled functions, identified by analyzing
call graphs, hardware register usage, ROM data references, and WRAM access patterns.

These names are **proposed** — they become official when the function is decompiled
and added to `[renames]` in `klonoa-eod-decomp.toml`.

## Text / UI Rendering Pipeline (code_0)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| FUN_0800d188 | TextStateMachine | 6147-line function; refs REG_BLDCNT/BLDALPHA, gEntityArray, gFrameCounter; calls DrawSpriteTilesPartial, WaitForVBlank; master UI/text state machine |
| FUN_0800b3c0 | RenderCharacterTiles | Refs ROM sprite table 0x0800B7B4, gEntityArray, gRenderFlags; calls DrawSpriteTiles, DrawSpriteTilesFlipped, WaitForVBlank; character tile renderer |
| FUN_08009064 | RenderDialogBox | Refs ROM_SPRITE_FRAME_TABLE (0x08078FC8 ×3), gSpriteSlotIndex, gEntityArray, gStatusTable; no bl calls; dialog/message box sprite layout |
| FUN_080098c8 | RenderDialogSprites | Refs ROM_SPRITE_FRAME_TABLE (×4), gOamBuffer, gEntityArray; calls InitOamEntries; dialog sprite rendering |
| FUN_080070a0 | RenderMenuUI | Refs ROM_SPRITE_FRAME_TABLE (×11), 30+ WRAM globals; calls InitOamEntries, RenderDialogBox; master menu/HUD renderer |
| FUN_08005cf4 | RenderHUDTop | Refs ROM_SPRITE_FRAME_TABLE, gOamBuffer, gEntityArray; calls InitOamEntries; top HUD element rendering |
| FUN_08005fa4 | RenderHUDBottom | Refs ROM_SPRITE_FRAME_TABLE (×6), gEntityArray, gStatusTable; no bl calls; bottom HUD element rendering |
| FUN_0800ca0c | SetupDisplayConfig | Refs ROM_DISPLAY_CONFIG_TABLE (0x080D821C), 20+ WRAM globals; calls FUN_08046db8; configures display modes and layer setup |
| FUN_0800ac34 | UpdateUIState | Calls TextStateMachine, WaitForVBlank, PlaySoundEffect; manages UI state transitions |
| FUN_0800bef0 | UpdateTextScroll | Refs gTextScrollState (0x030034DC); text scroll/advance logic |

## Engine Rendering Pipeline (engine)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| FUN_08003dc0 | DrawSpriteTiles | 3971 lines; no bl/pool; core sprite/tile VRAM writer (pure register computation) |
| FUN_08003d80 | DrawSpriteTilesPartial | Near DrawSpriteTiles; partial tile rendering variant |
| FUN_08003da0 | DrawSpriteTilesFlipped | Between the two DrawSprite functions; flipped tile variant |
| FUN_08001158 | InitGraphicsSystem | Calls AllocAndDecompress, DecompressData, CopyDataToVram, SetupVBlankSoundHandler; refs all VRAM regions, BG control regs; full graphics initialization |
| FUN_08003904 | RenderFrame | Calls DrawSpriteTiles (×11), StopAllSound, UpdateAllSoundChannels, RenderCharacterTiles, SetupVBlankSoundHandler; per-frame rendering dispatch |

## Entity / Object System (code_3)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| FUN_0803ac18 | IsEntityActive | Called by UpdateEntities in loop; returns bool per entity slot |
| FUN_0803ad94 | UpdateEntityState | Called by UpdateEntities for active entities; first update pass |
| FUN_0803af38 | UpdateEntityAnimation | Called by UpdateEntities for active entities; second update pass |
| FUN_080468b0 | UpdateGameLogic | Called by GameUpdate when not paused; first subsystem update |
| FUN_08045874 | UpdatePhysics | Called by GameUpdate; second subsystem update |
| FUN_08045f68 | UpdateCollision | Called by GameUpdate; third subsystem update |
| FUN_08046288 | UpdateCamera | Called by GameUpdate; fourth subsystem update |

## Memory / Asset Management (code_3)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| FUN_08043af4 | DecompressData | Called by AllocAndDecompress and InitGraphicsSystem; decompression routine |
| FUN_08043b34 | CopyDataToVram | Called by InitGraphicsSystem; bulk data copy |

## Sound Engine (m4a)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| FUN_0804f294 | InitSoundEngine | Called by SoundInit wrapper |
| FUN_0804ff08 | ResetSoundChannel | Called by StopSoundChannel wrapper |
| FUN_0804ffc8 | PlayMusicTrack | Called by DispatchMusicStreamCommand, EnableVBlankAndDispatchMusic |
| FUN_08050648 | SetupVBlankSoundHandler | Called by EnableVBlankHandler, EnableVBlankAndHandlers, InitGraphicsSystem |
| FUN_08050134 | SetupSoundInterrupt | Called by EnableVBlankAndHandlers |
| FUN_0805186c | PlaySoundEffect | Called by PlaySoundWithContext_D8/DC, UpdateUIState |
| FUN_08051870 | DispatchSoundCommand | Called by SoundCommand_6450 |
| FUN_080507e0 | UpdateSoundChannel | Called by UpdateAllSoundChannels loop |
| FUN_080506fc | LoadSoundData | Called by PlayMusicTrack |
| FUN_080505cc | StopAllSound | Called by InitGraphicsSystem, RenderFrame |
| FUN_080500fc | UpdateAllSoundChannels | Loops 4 times calling UpdateSoundChannel; called by RenderFrame |

## System / Utility

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| FUN_08025ba4 | VBlankWaitAndUpdate | Called by GameUpdate unconditionally at end |
| FUN_08025b78 | WaitForVBlank | Called by RenderCharacterTiles (×7), TextStateMachine (×3) |
| FUN_0804c050 | FinalizeGfxStream | Called by ShutdownGfxStream |
| FUN_0804c0ec | ProcessStreamOpcode | Called by DispatchStreamCommand_C0EC |
| FUN_0804c218 | ExecuteStreamCommand | Called by ProcessStreamCommand_C218 |
| FUN_08050094 | ExecuteMusicCommand | Called by ProcessStreamCommand_50094 |
| FUN_080008dc | MemoryCopy | Called by TextStateMachine |
| FUN_0800a468 | InitOamEntries | Inits 128 OAM entries from template; called by RenderMenuUI, RenderDialogSprites, RenderHUDTop |

## ROM Data Tables

| Address | Name | Description |
|---------|------|-------------|
| 0x08078FC8 | ROM_SPRITE_FRAME_TABLE | Sprite frame/animation data; array of {count, dataPtr} pairs |
| 0x080D821C | ROM_DISPLAY_CONFIG_TABLE | Display configuration / sprite mapping table |
| 0x080E2A7C | ROM_OAM_TEMPLATE | OAM template data (initial attribute values) |
| 0x0818B7AC | ROM_GFX_ASSET_TABLE | Graphics asset table for InitGraphicsSystem |
| 0x0818B8E0 | ROM_TILESET_TABLE | Tileset table for RenderFrame |
| 0x08118AB4 | ROM_SOUND_DATA_TABLE | Sound/music data table |
| 0x0800B7B4 | ROM_CHAR_TILE_MAP | Character-to-tile mapping for RenderCharacterTiles |
