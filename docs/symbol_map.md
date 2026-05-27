# Symbol Map

Semantic names for not-yet-decompiled functions, identified by analyzing
call graphs, hardware register usage, ROM data references, and WRAM access patterns.

These names are **proposed** — they become official when the function is decompiled
and added to `[renames]` in `klonoa-eod-decomp.toml`.

## Text / UI Rendering Pipeline (code_0)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_0800D188 | TextStateMachine | 6147-line function; refs REG_BLDCNT/BLDALPHA, gEntityArray, gFrameCounter; calls DrawSpriteTilesPartial, WaitForVBlank; master UI/text state machine |
| sub_0800B3C0 | RenderCharacterTiles | Refs ROM sprite table 0x0800B7B4, gEntityArray, gRenderFlags; calls DrawSpriteTiles, DrawSpriteTilesFlipped, WaitForVBlank; character tile renderer |
| sub_08009064 | RenderDialogBox | Refs ROM_SPRITE_FRAME_TABLE (0x08078FC8 ×3), gSpriteSlotIndex, gEntityArray, gStatusTable; no bl calls; dialog/message box sprite layout |
| sub_080098C8 | RenderDialogSprites | Refs ROM_SPRITE_FRAME_TABLE (×4), gOamBuffer, gEntityArray; calls InitOamEntries; dialog sprite rendering |
| sub_080070A0 | RenderMenuUI | Refs ROM_SPRITE_FRAME_TABLE (×11), 30+ WRAM globals; calls InitOamEntries, RenderDialogBox; master menu/HUD renderer |
| sub_08005CF4 | RenderHUDTop | Refs ROM_SPRITE_FRAME_TABLE, gOamBuffer, gEntityArray; calls InitOamEntries; top HUD element rendering |
| sub_08005FA4 | RenderHUDBottom | Refs ROM_SPRITE_FRAME_TABLE (×6), gEntityArray, gStatusTable; no bl calls; bottom HUD element rendering |
| sub_0800CA0C | SetupDisplayConfig | Refs ROM_DISPLAY_CONFIG_TABLE (0x080D821C), 20+ WRAM globals; calls sub_08046DB8; configures display modes and layer setup |
| sub_0800AC34 | UpdateUIState | Calls TextStateMachine, WaitForVBlank, PlaySoundEffect; manages UI state transitions |
| sub_0800BEF0 | UpdateTextScroll | Refs gTextScrollState (0x030034DC); text scroll/advance logic |

## Engine Rendering Pipeline (engine)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08003DC0 | DrawSpriteTiles | 3971 lines; no bl/pool; core sprite/tile VRAM writer (pure register computation) |
| sub_08003D80 | DrawSpriteTilesPartial | Near DrawSpriteTiles; partial tile rendering variant |
| sub_08003DA0 | DrawSpriteTilesFlipped | Between the two DrawSprite functions; flipped tile variant |
| sub_08001158 | InitGraphicsSystem | Calls AllocAndDecompress, DecompressData, CopyDataToVram, SetupVBlankSoundHandler; refs all VRAM regions, BG control regs; full graphics initialization |
| sub_08003904 | RenderFrame | Calls DrawSpriteTiles (×11), StopAllSound, UpdateAllSoundChannels, RenderCharacterTiles, SetupVBlankSoundHandler; per-frame rendering dispatch |

## Entity / Object System (code_3)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_0803AC18 | IsEntityActive | Called by UpdateEntities in loop; returns bool per entity slot |
| sub_0803AD94 | UpdateEntityState | Called by UpdateEntities for active entities; first update pass |
| sub_0803AF38 | UpdateEntityAnimation | Called by UpdateEntities for active entities; second update pass |
| sub_080468B0 | UpdateGameLogic | Called by GameUpdate when not paused; first subsystem update |
| sub_08045874 | UpdatePhysics | Called by GameUpdate; second subsystem update |
| sub_08045F68 | UpdateCollision | Called by GameUpdate; third subsystem update |
| sub_08046288 | UpdateCamera | Called by GameUpdate; fourth subsystem update |

## Memory / Asset Management (code_3)

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08043AF4 | DecompressData | Called by AllocAndDecompress and InitGraphicsSystem; decompression routine |
| sub_08043B34 | CopyDataToVram | Called by InitGraphicsSystem; bulk data copy |

## Sound Engine (m4a)

The m4a module now follows kleod/pokeemerald/sa3 conventions: the
hand-written Sappy/MP2K assembly lives in `asm/m4a0.s` (covers
`0x0804F284 → 0x0804FE9F`, included into `src/m4a.c`), and the C
portion in `src/m4a.c` uses canonical Sappy names matching kleod.

**For current m4a function names, see `klonoa-eod-decomp.toml`
`[renames]` (authoritative).**  Types and constants are in
`include/m4a_internal.h`.  Porting history and known blockers are in
`docs/m4a-kleod-rename-notes.md`.

This section previously contained ~50 proposed names; almost all are
now committed in the TOML (mostly renamed to kleod canonical names —
e.g., the former `SoundHardwareInit` is now `MPlayExtender`,
`DirectSoundFifoSetup` is `SoundInit`, `CgbChannelMix` is `CgbSound`,
etc.).  Listing them here would just duplicate the TOML.

## System / Utility

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08025BA4 | VBlankWaitAndUpdate | Called by GameUpdate unconditionally at end |
| sub_08025B78 | WaitForVBlank | Called by RenderCharacterTiles (×7), TextStateMachine (×3) |
| sub_0804C050 | FinalizeGfxStream | Called by ShutdownGfxStream |
| sub_0804C0EC | ProcessStreamOpcode | Called by DispatchStreamCommand_C0EC |
| sub_0804C218 | ExecuteStreamCommand | Called by ProcessStreamCommand_C218 |
| sub_080008DC | MemoryCopy | Called by TextStateMachine |
| sub_0800A468 | InitOamEntries | Inits 128 OAM entries from template; called by RenderMenuUI, RenderDialogSprites, RenderHUDTop |

## ROM Data Tables

| Address | Name | Description |
|---------|------|-------------|
| 0x08078FC8 | ROM_SPRITE_FRAME_TABLE | Sprite frame/animation data; array of {count, dataPtr} pairs |
| 0x080D821C | ROM_DISPLAY_CONFIG_TABLE | Display configuration / sprite mapping table |
| 0x080E2A7C | ROM_OAM_TEMPLATE | OAM template data (initial attribute values) |
| 0x0818B7AC | ROM_GFX_ASSET_TABLE | Graphics asset table for InitGraphicsSystem |
| 0x0818B8E0 | ROM_TILESET_TABLE | Tileset table for RenderFrame |
| 0x0800B7B4 | ROM_CHAR_TILE_MAP | Character-to-tile mapping for RenderCharacterTiles |

### Sound ROM Data Tables

Defined as ldscript symbols in `ldscript.in.txt` and declared in
`include/m4a_internal.h` (kleod canonical names).

| Address | Symbol | Description |
|---------|--------|-------------|
| 0x08118AB4 | `gMPlayTable` | Music player table: `struct MusicPlayer[NUM_MUSIC_PLAYERS]` |
| 0x08118AE4 | `gSongTable` | Song metadata table: `struct Song[]` indexed by song ID |
| 0x08117C8C | `gSoundCmdTable` | Sound-command function-pointer array indexed by command byte |
| 0x081179E4 | `gMPlayJumpTableTemplate` | Template for `gMPlayJumpTable` (copied at init) |
| 0x08117A74 | `gScaleTable` | MIDI key → packed (freqIdx \| shift) lookup, indexed by key |
| 0x08117B28 | `gFreqTable` | Base PCM frequencies for Direct Sound mixer |
| 0x08117B70 | `gCgbScaleTable` | CGB MIDI key → packed lookup |
| 0x08117BF4 | `gCgbFreqTable` | CGB base frequencies (square / wave channels) |
| 0x08117C0C | `gNoiseTable` | Channel-4 noise frequency presets |
| 0x08117C58 | `gClockTable` | LFO/tempo cycle table |
| 0x08117B58 | `gPcmSamplesPerVBlankTable` | Samples-per-frame for each `SOUND_MODE_FREQ_*` |
| 0x081177E4 | `ROM_SOUND_INIT_DATA` | Sound configuration init data (used by gfx.c) |
