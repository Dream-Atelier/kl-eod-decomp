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

### Sound System Init & Shutdown

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_0804EB64 | SoundMain | Main sound system setup; calls stream/gfx init, refs gSoundInfo |
| sub_0804ED68 | SoundDmaInit | DMA controller setup for sound; has DMA reg writes |
| sub_0804EE34 | SoundReset | Minimal state reset (leaf, 23 lines) |
| sub_0804EE60 | DmaControllerInit | Full DMA init; calls SoundReset |
| sub_0804EF50 | SoundInfoInit | Initialize SoundInfo struct fields (leaf) |
| sub_0804F294 | InitSoundEngine | Called by SoundInit wrapper |

### Sound Data & Buffer Management

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_0804EFDE | SoundBufferAlloc | Allocate mixing buffers |
| sub_0804F004 | SoundContextInit | Setup mixer state; refs gSoundInfo, gStreamPtr, gControlBlock |
| sub_0804F0D0 | SoundChannelTableInit | Initialize channel table entries |

### MIDI / Music Sequence Engine

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_0804F180 | MidiReadUnaligned | Read misaligned MIDI value; calls ReadUnalignedU16 |
| sub_0804F1C4 | MidiProcessEvent | Dispatch MIDI note/control event |
| sub_0804F248 | MPlayMain | **CORE**: main music player tick (587 lines, largest in m4a, uses DMA) |
| sub_0804F6D4 | VoiceUtil | Minimal voice utility (leaf, 22 lines) |
| sub_0804F6F4 | VoiceLookup | Voice lookup wrapper |
| sub_0804F724 | InstrumentLookup | ROM instrument table lookup (ROM_INSTRUMENT_TABLE) |
| sub_0804F73C | InstrumentGetEntry | Get instrument entry from ROM (leaf) |
| sub_0804F758 | MidiDecodeByte | Decode single MIDI byte (leaf, 11 lines) |
| sub_0804F766 | MidiNoteSetup | Setup MIDI note on channel |
| sub_0804F7B4 | MidiCommandHandler | MIDI command dispatch table; writes REG_SOUND1CNT_L |
| sub_0804F944 | MPlayContinue | Music playback continuation (327 lines) |
| sub_0804FB9C | SoundContextRef | Get sound context reference |

### Music Playback Control

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_0804FBE0 | MPlayStop_Channel | Stop single music channel (leaf) |
| sub_0804FC10 | MPlayStop | Stop all music playback (313 lines) |
| sub_0804FE50 | SoundEffectUtil | Sound effect utility (leaf) |
| sub_0804FE6C | SoundEffectProcess | Process sound effect chain |
| sub_0804FEA0 | FreqTableLookup | Frequency lookup from ROM pitch tables |
| sub_0804FF08 | MPlayChannelReset | Reset channel state; checks Sappy magic 0x68736D53 |
| sub_0804FF44 | m4aSoundInit_Impl | Full sound system init dispatcher |
| sub_0804FFC8 | m4aSongNumStart | Start playing music track by ID (ROM_MUSIC_TABLE) |
| sub_0804FFF6 | m4aSongNumContinue | Continue/queue music track |
| sub_08050042 | m4aSongNumLoad | Load music data from ROM |
| sub_08050094 | m4aMPlayCommand | Execute music player command |
| sub_080500C8 | m4aSongNumStop | Stop current music track |
| sub_080500FC | m4aSoundVSync | VBlank: update all sound channels (×4 loop) |

### Sound Hardware & Direct Sound

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08050134 | m4aSoundVSyncSetup | Setup VBlank sound sync |
| sub_08050162 | SappyStateCheck | Verify Sappy magic marker (0x68736D53) |
| sub_080501BA | SoundEffectTrigger | Trigger sound effect via gMPlayInfo_SE |
| sub_08050200 | SoundHardwareInit | **CRITICAL**: init all GBA sound regs (14 HW regs: SOUNDCNT, FIFO, DMA) |
| sub_08050344 | DirectSoundFifoSetup | **CRITICAL**: FIFO_A/B and DMA config (8 HW regs) |
| sub_0805043C | SoundTimerSetup | Configure timer for sample rate |
| sub_080504E0 | SoundSystemConfigure | Configure sound system mode |
| sub_08050578 | SoundPlatformDetect | Detect platform/capabilities |
| sub_080505CC | m4aSoundShutdown | Emergency stop all sound |
| sub_08050648 | m4aSoundVSyncOn | Register VBlank sound handler |
| sub_08050684 | VBlankSoundCallback | VBlank-triggered sound update |
| sub_080506FC | MPlayOpen | Load & open music player data from ROM |
| sub_080507E0 | MPlayChannelUpdate | Update single music channel |

### CGB Sound (Channels 1-4) & Pitch Control

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08050820 | CgbModVol | CGB channel volume modulation |
| sub_080508E8 | CgbLookupTable | CGB frequency/volume lookup data (leaf) |
| sub_0805099E | MidiKeyToCgbFreq | MIDI note → CGB frequency; writes 4 pitch regs |
| sub_08050A94 | CgbLookupUtil | CGB utility lookup (leaf) |
| sub_08050AFC | CgbSound | CGB channel hardware control (14 HW regs: SOUND1-4CNT, WAVE_RAM) |
| sub_08050C70 | SoundMixerMain | **CORE**: process all mixer channels (393 lines, 2nd largest) |

### Sound State Machine & MIDI Encoding

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08050F4A | SoundStateCheck1 | Sappy magic state check (leaf) |
| sub_08050F70 | SoundStateCheck2 | Sappy magic state check (leaf) |
| sub_08050FD8 | SoundStateCheck3 | Sappy magic state check (leaf) |
| sub_0805104C | MidiNoteLookup | MIDI note to frequency lookup (leaf) |
| sub_080510B4 | MidiUtilConvert | MIDI utility converter (leaf) |
| sub_080510D4 | MidiCommandEncode1 | Encode MIDI command type 1 |
| sub_08051148 | MidiCommandEncode2 | Encode MIDI command type 2 |
| sub_080511BC | MPlayCommandDispatch | Music command dispatcher (ROM table, 174 lines) |
| sub_08051314 | SoundCommandHandler | Command dispatch from ROM_SOUND_CMD_TABLE |
| sub_08051348 | BitMaskLUT | 32-bit channel bitmask lookup table (leaf, 118 lines) |
| sub_0805186C | PlaySoundEffect | Play sound effect; called by PlaySoundWithContext_D8/DC |
| sub_08051870 | DispatchSoundCommand | Dispatch sound command; called by SoundCommand_6450 |

## System / Utility

| Address | Proposed Name | Evidence |
|---------|---------------|----------|
| sub_08025BA4 | VBlankWaitAndUpdate | Called by GameUpdate unconditionally at end |
| sub_08025B78 | WaitForVBlank | Called by RenderCharacterTiles (×7), TextStateMachine (×3) |
| sub_0804C050 | FinalizeGfxStream | Called by ShutdownGfxStream |
| sub_0804C0EC | ProcessStreamOpcode | Called by DispatchStreamCommand_C0EC |
| sub_0804C218 | ExecuteStreamCommand | Called by ProcessStreamCommand_C218 |
| sub_08050094 | ExecuteMusicCommand | Called by ProcessStreamCommand_50094 |
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

| Address | Name | Description |
|---------|------|-------------|
| 0x08118AB4 | ROM_MUSIC_TABLE | Music track table: {count, trackDataPtr} entries indexed by track ID |
| 0x08118AE4 | ROM_MUSIC_META_TABLE | Music track metadata (offsets, lengths, loop points) |
| 0x08117C8C | ROM_SOUND_CMD_TABLE | Sound command dispatch: function pointer array by command byte |
| 0x081179E4 | ROM_INSTRUMENT_TABLE | Instrument/voice data: waveform, envelope, pitch per instrument |
| 0x08117A74 | ROM_FREQ_TABLE_1 | Pitch/frequency lookup table 1 |
| 0x08117B28 | ROM_FREQ_TABLE_2 | Pitch/frequency lookup table 2 |
| 0x08117B70 | ROM_PITCH_TABLE | MIDI note-to-pitch conversion table |
| 0x08117BF4 | ROM_WAVE_DUTY_TABLE | Square wave duty cycle table |
| 0x08117C0C | ROM_NOISE_TABLE | Noise channel parameter table |
| 0x08117C48 | ROM_ENVELOPE_TABLE | Volume envelope data |
| 0x08117C58 | ROM_SWEEP_TABLE | Frequency sweep data |
| 0x081177E4 | ROM_SOUND_INIT_DATA | Sound configuration init data |
