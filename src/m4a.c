#include "global.h"
#include "gba.h"
#include "globals.h"
#include "include_asm.h"
#include "m4a_internal.h"

extern char gNumMusicPlayers[];
#define NUM_MUSIC_PLAYERS ((u16)gNumMusicPlayers)

/* ══════════════════════════════════════════════════════════════════════
 * m4a — Nintendo MusicPlayer2000 ("Sappy") sound engine
 *
 * Format: 39 songs + 126 SFX stored as MIDI-like bytecode in ROM.
 * Song table at ROM_MUSIC_META_TABLE (0x08118AE4).
 * 3 voice/instrument tables, ~300KB of signed 8-bit PCM samples.
 * Uses Direct Sound (FIFO A/B via DMA) + CGB channels 1-4.
 * ══════════════════════════════════════════════════════════════════════ */

/* ── Sound System Initialization & Shutdown ── */

/*
 * sub_0804EB64: top-level sound system initialization.
 * Sets up gSoundInfo, initializes the stream/gfx subsystems,
 * and prepares the sound engine for playback.
 *   248 lines, calls UpdateCursorBlink/ProcessScreenFade/UpdatePaletteFadeStep
 *   refs: gSoundInfo (0x0300081C), gGfxBufferPtr (0x030034A0)
 */
INCLUDE_ASM("asm/nonmatchings/m4a", sub_0804EB64);
/*
 * SoundDmaInit: DMA controller setup for sound data transfers.
 * Configures DMA channels for PCM sample streaming to FIFO.
 *   79 lines, has DMA register writes (REG_DMA3SAD/DAD/CNT)
 *   calls: thunk_sub_080001E0 (memory alloc)
 */
INCLUDE_ASM("asm/nonmatchings/m4a", SoundDmaInit);
/**
 * FreeSoundStruct: frees the sound info struct and its inner sample buffer.
 *
 * Reads gSoundInfo, frees the inner buffer (at offset -4),
 * then frees the struct itself.
 */
void FreeSoundStruct(void) {
    u32 *p = (u32 *)0x0300081C; /* &gSoundInfo */
    thunk_sub_0800020C(*(u32 *)(*p) - 4);
    thunk_sub_0800020C(*p);
}
/*
 * SoundReset: writes a tile value with palette bank 15 to screen buffer B.
 *
 * Computes the destination address as gScreenBufferA + 0x800 + (col + row*32)*2,
 * ORs the tile value with 0xF000 (palette bank 15), and stores the halfword.
 */
void SoundReset(s16 col, s16 row, u16 tile) {
    u32 bufAddr = 0x03000900;
    register u8 *buf asm("r3");
    u32 off;

    asm("" : "=r"(buf) : "0"(bufAddr));
    off = (u32)((col + row * 32) * 2);
    buf += 0x800;
    off += (u32)buf;
    *(u16 *)off = tile | 0xF000;
}
/*
 * DmaControllerInit: full DMA initialization with channel config.
 * Sets up DMA for both Direct Sound channels (FIFO A and B).
 *   125 lines, has DMA register writes
 *   calls: SoundReset
 */
INCLUDE_ASM("asm/nonmatchings/m4a", DmaControllerInit);
/*
 * SoundInfoInit: initialize SoundInfo struct fields.
 * Fills in default values for the sound info structure at gSoundInfo.
 *   73 lines, leaf function
 *   refs: gSoundInfo (0x0300081C), gStreamPtr (0x03004D84)
 */
/**
 * SoundInfoInit: update sound info timer and flags.
 *
 * Leaf function compiled with -fprologue-bugfix (separate compilation unit).
 * C source in src/m4a_nopush_SoundInfoInit.c.
 */
asm(".include \"build/m4a_nopush_SoundInfoInit.s\"");
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_GetStreamPtr);
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_ValidateStream);

/* ── Sound Data & Buffer Management ── */

/*
 * SoundBufferAlloc: allocate sound mixing buffers.
 * Allocates WRAM buffers for the PCM mixing pipeline.
 *   21 lines, calls sub_08051868
 */
INCLUDE_ASM("asm/nonmatchings/m4a", SoundBufferAlloc);
/*
 * SoundContextInit: setup sound context and mixer state.
 * Initializes the mixer configuration, channel assignments, and
 * links the sound context to the global control structures.
 *   102 lines, leaf function
 *   refs: gSoundInfo, gStreamPtr, gControlBlock (0x03004C20)
 */
/**
 * StreamCmd_SetSoundReverb: sets gSoundInfo reverb from stream nibble.
 * (shares literal pool with StreamCmd_GetSoundReverb — can't decompile independently)
 */
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_SetSoundReverb);
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_GetSoundReverb);
INCLUDE_ASM("asm/nonmatchings/m4a", SoundContextInit);
/*
 * SoundChannelTableInit: initialize sound channel table entries.
 * Sets up the per-channel state for all active sound channels.
 *   86 lines, calls sub_08051868
 */
/**
 * StreamCmd_StopSoundAndSync: stops sound, syncs freq to BG scroll, advances by 2.
 */
void sub_08051868(u32);
void StreamCmd_StopSoundAndSync(void) {
    u32 *soundInfoRef = (u32 *)0x0300081C;
    u16 *bgLayerState;
    u16 *soundInfo;

    sub_08051868(((u32 *)*soundInfoRef)[0x24 / 4]);

    ((u8 *)*soundInfoRef)[0x09] = 0;
    ((u8 *)*soundInfoRef)[0x08] = 0;

    bgLayerState = (u16 *)0x03003430;
    soundInfo = (u16 *)*soundInfoRef;
    bgLayerState[0x24 / 2] = soundInfo[0x10 / 2];
    bgLayerState[0x26 / 2] = soundInfo[0x12 / 2];
    ((u32 *)soundInfo)[0x1C / 4] = ((u32 *)soundInfo)[0x20 / 4];
    soundInfo[0x1A / 2] = soundInfo[0x18 / 2];

    gStreamPtr += 2;
}
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_StopAndAdvance);
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_ReadFreqParam);
INCLUDE_ASM("asm/nonmatchings/m4a", StreamCmd_SetChannelMode);

/* ── MIDI / Music Sequence Processing ── */

/*
 * MidiReadUnaligned: read a potentially unaligned MIDI value.
 * Used to parse variable-length MIDI data from track bytecode.
 *   32 lines, calls ReadUnalignedU16
 */
/**
 * StreamCmd_SetSoundFreqs: sets two sound frequency values from stream data.
 *
 * Reads u16 from stream bytes[2-3] -> gSoundInfo[0x10],
 * reads u16 from stream bytes[4-5] -> gSoundInfo[0x12].
 * Advances stream by 6.
 */
u32 ReadUnalignedU16(u8 *);
void StreamCmd_SetSoundFreqs(void) {
    u8 **streamRef = (u8 **)0x03004D84;
    u16 **soundInfoRef;
    u16 freqVal;

    freqVal = ReadUnalignedU16(*streamRef + 2);

    soundInfoRef = (u16 **)0x0300081C;
    (*soundInfoRef)[0x10 / 2] = freqVal;

    freqVal = ReadUnalignedU16(*streamRef + 4);
    (*soundInfoRef)[0x12 / 2] = freqVal;

    *streamRef += 6;
}
void StreamCmd_AdvanceStream(void) {
    gStreamPtr += 3;
}
/*
 * MidiProcessEvent: dispatch a MIDI note or control event.
 * Processes a single event from the track bytecode stream,
 * handling note-on, note-off, and control change messages.
 *   72 lines, calls DmaControllerInit
 */
INCLUDE_ASM("asm/nonmatchings/m4a", MidiProcessEvent);
/*
 * MPlayMain_SetAndProcess: stores a value into gSoundInfo[0xD]
 * then calls MidiProcessEvent.
 *   r0: value to store
 *   10 lines (split from former 587-line MPlayMain)
 */
u32 MidiProcessEvent(void);
u32 MPlayMain_SetAndProcess(u32 val) {
    u8 *info = (u8 *)gSoundInfo;
    info[0x0D] = val;
    return MidiProcessEvent();
}
/*
 * MPlayMain: CORE music player tick — largest function in m4a.
 * Called each frame to advance all active music tracks. Reads track bytecode,
 * processes MIDI commands, manages DMA transfers for PCM playback, and
 * updates channel state.
 */
INCLUDE_ASM("asm/nonmatchings/m4a", MPlayMain);

/* ══════════════════════════════════════════════════════════════════════
 * Handcrafted ARM/Thumb blob — 0x0804F284 → 0x0804FE9F
 *
 * Nintendo's original MusicPlayer2000 (Sappy) was hand-written in
 * assembly: interwork helpers, the SoundMain mixer, MP2KPlayerMain,
 * per-MIDI-event handlers, etc.  This file is reused from the
 * `kleod` decomp (same ROM SHA1) and assembles to identical bytes.
 * The other MP2K decomp projects use the same split pattern:
 *   • kleod        — asm/m4a0.s
 *   • sa3          — src/lib/m4a/m4a0.s
 *   • pokeemerald  — src/m4a_1.s
 * ══════════════════════════════════════════════════════════════════════ */
asm(".syntax unified\n"
    ".include \"asm/m4a0.s\"\n"
    ".syntax divided\n");
/**
 * MidiKeyToFreq: convert a MIDI note + fine adjust into a Direct
 * Sound playback frequency for the given sample.  Uses two ROM LUTs:
 * gScaleTable (key → packed [freqIdx | shift]) and gFreqTable
 * (the base frequencies).  Final result is wav->freq scaled by the
 * lerp between val1 and val2 across fineAdjust.
 *
 * The trailing asm("bx lr\n.hword 0\n") emits the 4 dead bytes that
 * appear after the literal pool in the original ROM — agbcc would
 * otherwise leave a literal-pool gap that throws downstream addresses.
 */
u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust) {
    u32 val1;
    u32 val2;
    u32 fineAdjustShifted = fineAdjust << 24;

    if (key > 178) {
        key = 178;
        fineAdjustShifted = 255 << 24;
    }

    val1 = gScaleTable[key];
    val1 = gFreqTable[val1 & 0xF] >> (val1 >> 4);

    val2 = gScaleTable[key + 1];
    val2 = gFreqTable[val2 & 0xF] >> (val2 >> 4);

    return umul3232H32(wav->freq, val1 + umul3232H32(val2 - val1, fineAdjustShifted));
}
asm("bx lr\n.hword 0\n");
/**
 * MPlayContinue: resume a paused music player.
 *
 * Clears MUSICPLAYER_STATUS_PAUSE if the player is in a valid state
 * (lockStatus == ID_NUMBER).  Despite the name "Continue", several
 * existing call sites in this file (m4aSongNumStop, StopSoundChannel,
 * m4aMPlayAllContinue) invoke it as a stop primitive; the ROM is what it
 * is.  Canonical Sappy/MP2K name.
 */
void MPlayContinue(struct MP2KPlayerState *mplayInfo) {
    if (mplayInfo->lockStatus == ID_NUMBER) {
        mplayInfo->lockStatus++;
        mplayInfo->status &= ~MUSICPLAYER_STATUS_PAUSE;
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

/**
 * MPlayFadeOut: start a linear volume fade on the player.
 *
 * Initialises fade counters; the actual per-frame ramp happens in
 * FadeOutBody (called from MP2KPlayerMain in asm/m4a0.s).
 */
void MPlayFadeOut(struct MP2KPlayerState *mplayInfo, u16 speed) {
    if (mplayInfo->lockStatus == ID_NUMBER) {
        mplayInfo->lockStatus++;
        mplayInfo->fadeCounter = speed;
        mplayInfo->fadeInterval = speed;
        mplayInfo->fadeOV = (64 << FADE_VOL_SHIFT);
        mplayInfo->lockStatus = ID_NUMBER;
    }
}

/* ── Sound Init Dispatcher & Track Control ── */

/*
 * m4aSoundInit: full sound system initialization dispatcher.
 * Calls BitUnPack for data decompression, sets up sound channels,
 * and prepares the music player for first use.
 *   50 lines
 *   calls: BitUnPack, SoundHardwareInit, DirectSoundFifoSetup
 *   refs: ROM_MUSIC_TABLE (0x08118AB4)
 */
extern char SoundMainRAM[];
void BitUnPack(u32, u32, u32);
void SoundInit(struct SoundMixerState *);
void MPlayOpen(u32 *a, u8 *b, u8 c);

void m4aSoundInit(void) {
    s32 i;

    BitUnPack((u32)SoundMainRAM & ~1, 0x03000388, 0x04000100);
    SoundInit((struct SoundMixerState *)0x030054A0);
    MPlayExtender((struct MixerSource *)0x030064E0);
    m4aSoundMode(0x0094F800);

    {
        u16 count = NUM_MUSIC_PLAYERS;
        if (count != 0) {
            u8 *table = (u8 *)0x08118AB4;
            i = count;

            do {
                u32 *mplayInfo = *(u32 **)table;
                MPlayOpen(mplayInfo, (u8 *)*(u32 *)(table + 4), *(u8 *)(table + 8));
                ((u8 *)mplayInfo)[0x0B] = *(u16 *)(table + 0x0A);
                *(u32 *)((u8 *)mplayInfo + 0x18) = 0x030066A0;
                table += 0x0C;
                i--;
            } while (i != 0);
        }
    }
}
/**
 * m4aSoundMain: per-frame entry point for the sound engine — just
 * calls SoundMain (in asm/m4a0.s).  Kleod-canonical name.
 */
void m4aSoundMain(void) {
    SoundMain();
}
/**
 * m4aSongNumStart: start playing a music track by song index.
 *
 * Looks up the song from gSongTable, finds the assigned music player
 * from gMPlayTable, then calls MPlayStart to begin playback.
 */
void m4aSongNumStart(u16 idx) {
    u32 mplayAddr = 0x08118AB4;
    u32 songAddr = 0x08118AE4;
    register u8 *mplay asm("r2");
    register u8 *songs asm("r1");
    u32 songOff;
    u8 *song;
    u16 playerIdx;
    u32 off;

    asm("" : "=r"(mplay) : "0"(mplayAddr));
    asm("" : "=r"(songs) : "0"(songAddr));
    songOff = (u32)idx << 3;
    songOff += (u32)songs;
    song = (u8 *)songOff;
    playerIdx = *(u16 *)(song + 4);
    off = (u32)playerIdx * 2 + playerIdx;
    off <<= 2;
    off += (u32)mplay;
    {
        u32 player = *(u32 *)off;
        u32 header = *(u32 *)song;
        MPlayStart(player, header);
    }
}
/*
 * m4aSongNumStartOrChange: start playing a song; if it's already
 * playing and not paused, do nothing — but if track[0] is inactive
 * or paused, (re)start it.  Kleod-canonical name.
 */
INCLUDE_ASM("asm/nonmatchings/m4a", m4aSongNumStartOrChange);
/*
 * m4aSongNumStartOrContinue: like StartOrChange, but for a paused
 * song it resumes via MPlayContinue instead of restarting.
 * Kleod-canonical name.
 */
INCLUDE_ASM("asm/nonmatchings/m4a", m4aSongNumStartOrContinue);
/**
 * m4aSongNumStop: stop the given song if it's the one currently loaded.
 * Kleod-canonical name; calls MPlayStop on the assigned player.
 */
void m4aSongNumStop(u16 idx) {
    u32 mplayAddr = 0x08118AB4;
    u32 songAddr = 0x08118AE4;
    register u8 *mplay asm("r2");
    register u8 *songs asm("r1");
    u32 songOff;
    u8 *song;
    u16 playerIdx;
    u32 off;
    u32 *player;

    asm("" : "=r"(mplay) : "0"(mplayAddr));
    asm("" : "=r"(songs) : "0"(songAddr));
    songOff = (u32)idx << 3;
    songOff += (u32)songs;
    song = (u8 *)songOff;
    playerIdx = *(u16 *)(song + 4);
    off = (u32)playerIdx * 2 + playerIdx;
    off <<= 2;
    off += (u32)mplay;
    player = *(u32 **)off;
    if (player[0] == *(u32 *)song)
        MPlayStop(player);
}
/**
 * m4aSongNumContinue: resume the given song if it's the one currently
 * loaded.  Despite the name, the implementation calls MPlayContinue
 * (which clears the PAUSE bit) — kleod-canonical.
 */
void m4aSongNumContinue(u16 idx) {
    u32 mplayAddr = 0x08118AB4;
    u32 songAddr = 0x08118AE4;
    register u8 *mplay asm("r2");
    register u8 *songs asm("r1");
    u32 songOff;
    u8 *song;
    u16 playerIdx;
    u32 off;
    u32 *player;

    asm("" : "=r"(mplay) : "0"(mplayAddr));
    asm("" : "=r"(songs) : "0"(songAddr));
    songOff = (u32)idx << 3;
    songOff += (u32)songs;
    song = (u8 *)songOff;
    playerIdx = *(u16 *)(song + 4);
    off = (u32)playerIdx * 2 + playerIdx;
    off <<= 2;
    off += (u32)mplay;
    player = *(u32 **)off;
    if (player[0] == *(u32 *)song)
        MPlayContinue(player);
}
/*
 * m4aSoundVSync: VBlank sound update — called every frame.
 * Loops over 4 sound channels, calling MPlayChannelUpdate for each.
 * This is the main per-frame entry point for the sound engine.
 *   23 lines, refs: ROM_MUSIC_TABLE (0x08118AB4)
 *   calls: MPlayStop
 */
/**
 * m4aMPlayAllStop: stops all 4 music player instances.
 * Iterates ROM_MUSIC_TABLE (0x08118AB4), calling MPlayStop on each.
 */
void m4aMPlayAllStop(void) {
    u16 n;
    s32 count;
    const struct MusicPlayer *table;
    n = NUM_MUSIC_PLAYERS;
    if (n == 0)
        return;
    table = gMPlayTable;
    count = n;
    do {
        MPlayStop(table->info);
        table++;
        count--;
    } while (count != 0);
}
/** StopSoundChannel: wrapper that calls MPlayContinue to stop a channel. */
void StopSoundChannel(u32 channel) {
    MPlayContinue(channel);
}

/* ── Interrupt & VBlank Handlers ── */

/**
 * m4aMPlayAllContinue: loop over all 4 music players calling
 * MPlayContinue on each.  Resumes any paused tracks.  Kleod-canonical.
 */
void m4aMPlayAllContinue(void) {
    u16 n;
    s32 count;
    const struct MusicPlayer *table;
    n = NUM_MUSIC_PLAYERS;
    if (n == 0)
        return;
    table = gMPlayTable;
    count = n;
    do {
        MPlayContinue(table->info);
        table++;
        count--;
    } while (count != 0);
}
/** m4aMPlayFadeOut: public wrapper that calls MPlayFadeOut. */
void m4aMPlayFadeOut(struct MP2KPlayerState *mplayInfo, u16 speed) {
    MPlayFadeOut(mplayInfo, speed);
}
asm(".align 2, 0");

/*
 * m4aMPlayFadeOutTemporarily: fade out, then pause (not stop).
 * Kept as INCLUDE_ASM: kleod's C body would duplicate the 8-byte
 * literal pool that asm/nonmatchings/m4a/MPlayTrackFadeAndVerify.s
 * already contains as its "prelude" (luvdis labels the next function
 * 6 bytes too early, capturing this pool as fake instructions).
 */
INCLUDE_ASM("asm/nonmatchings/m4a", m4aMPlayFadeOutTemporarily);
/*
 * m4aMPlayFadeIn: fade in + clear PAUSE.  Kept as INCLUDE_ASM for
 * the same phantom-literal-pool reason as m4aMPlayFadeOutTemporarily.
 */
INCLUDE_ASM("asm/nonmatchings/m4a", m4aMPlayFadeIn);
/**
 * m4aMPlayImmInit: for each track whose status has START+EXIST set,
 * Clear64byte the track then seed its defaults (bendRange=2,
 * volPublic=64, lfoSpeed=22, voicegroup.type=1).
 */
void m4aMPlayImmInit(struct MP2KPlayerState *mplayInfo) {
    s32 trackCount = mplayInfo->trackCount;
    struct MP2KTrack *track = mplayInfo->tracks;

    while (trackCount > 0) {
        if (track->status & MPT_FLG_EXIST) {
            if (track->status & MPT_FLG_START) {
                Clear64byte((u32)track);
                track->status = MPT_FLG_EXIST;
                track->bendRange = 2;
                track->volPublic = 64;
                track->lfoSpeed = 22;
                track->voicegroup.type = 1;
            }
        }

        trackCount--;
        track++;
    }
}

/* ── Sound Hardware Initialization ── */

/*
 * SoundHardwareInit: CRITICAL — initialize all GBA sound hardware registers.
 * Writes to 14 hardware registers to configure the sound system:
 * REG_SOUNDCNT_L/H/X (master volume, channel enable, master on/off),
 * REG_FIFO_A/B (Direct Sound FIFO), DMA1/2 (sample streaming),
 * and various channel control registers.
 *   126 lines
 *   HW: 0x04000060, 0x04000063, 0x04000080, 0x04000084,
 *       0x040000A0, 0x040000A4, 0x040000BC, 0x040000C4,
 *       0x040000C6, 0x040000D0
 */
/**
 * MPlayExtender: install the CGB extension callbacks into the sound
 * engine and pre-clear the CGB channel array.  Writes the 9 jump-
 * table slots (8, 17, 19, 28-33) that route MP2K events to their
 * handlers, then hooks up SoundInfo->{cgbChans,CgbSound,CgbOscOff,
 * MidiKeyToCgbFreq,maxScanlines}.  Initialises each cgbChan's type
 * (1..4) and panMask (0x11,0x22,0x44,0x88).
 *
 * The BitUnPack call mirrors kleod's CpuFill32: control word
 * 0x05000040 = fill mode + 32-bit + count 64 words (256 bytes =
 * 4 × sizeof(MixerSource)).
 */
extern char gMaxLines[];
#define MAX_LINES ((u32)gMaxLines)

void MPlayExtender(struct MixerSource *cgbChans) {
    struct SoundMixerState *soundInfo;
    u32 lockStatus;

    REG_SOUNDCNT_X = 0x8F;  /* SOUND_MASTER_ENABLE | SOUND_4_ON | SOUND_3_ON | SOUND_2_ON | SOUND_1_ON */
    REG_SOUNDCNT_L = 0;
    REG_NR12 = 0x8;
    REG_NR22 = 0x8;
    REG_NR42 = 0x8;
    REG_NR14 = 0x80;
    REG_NR24 = 0x80;
    REG_NR44 = 0x80;
    REG_NR30 = 0;
    REG_NR50 = 0x77;

    soundInfo = SOUND_INFO_PTR;

    lockStatus = soundInfo->lockStatus;

    if (lockStatus != ID_NUMBER)
        return;

    soundInfo->lockStatus++;

    gMPlayJumpTable[8]  = MPlayCommandDispatch;  /* MP2K_event_memacc */
    gMPlayJumpTable[17] = MP2K_event_lfos;
    gMPlayJumpTable[19] = MP2K_event_mod;
    gMPlayJumpTable[28] = SoundCmd_Dispatch;     /* MP2K_event_xcmd */
    gMPlayJumpTable[29] = MP2K_event_endtie;
    gMPlayJumpTable[30] = SampleFreqSet;
    gMPlayJumpTable[31] = TrackStop;
    gMPlayJumpTable[32] = FadeOutBody;
    gMPlayJumpTable[33] = TrkVolPitSet;

    soundInfo->cgbChans = cgbChans;
    soundInfo->CgbSound = CgbSound;
    soundInfo->CgbOscOff = CgbOscOff;
    soundInfo->MidiKeyToCgbFreq = MidiKeyToCgbFreq;
    soundInfo->maxScanlines = MAX_LINES;

    {
        u32 zero = 0;
        BitUnPack((u32)&zero, (u32)cgbChans, 0x05000040);
    }

    cgbChans[0].type = 1;
    cgbChans[0].panMask = 0x11;
    cgbChans[1].type = 2;
    cgbChans[1].panMask = 0x22;
    cgbChans[2].type = 3;
    cgbChans[2].panMask = 0x44;
    cgbChans[3].type = 4;
    cgbChans[3].panMask = 0x88;

    soundInfo->lockStatus = lockStatus;
}
asm(".align 2, 0");
void SoundHardwareInit_Tail(void) {
    asm("swi 0x2A");
}
/**
 * ClearChain: indirect call through gMPlayJumpTable[34].
 *
 * The slot at 0x030064D8 (= gMPlayJumpTable[34]) holds a function pointer
 * installed by MPlayExtender; calling it with the channel struct routes
 * to the per-channel cleanup handler.  Sappy/MP2K-original name.
 */
void ClearChain(u32 chan) {
    sub_0805186C(chan, gMPlayInfo_BGM);
}
/**
 * Clear64byte: indirect call through gMPlayJumpTable[35].
 *
 * Mirror of ClearChain at jump-table slot 35 (0x030064DC); zeros 64
 * bytes starting at the given pointer.  Sappy/MP2K-original name.
 */
void Clear64byte(u32 x) {
    sub_0805186C(x, gMPlayInfo_SE);
}

/* ── Direct Sound & DMA Configuration ── */

/*
 * DirectSoundFifoSetup: configure Direct Sound FIFO and DMA channels.
 * Sets up FIFO_A and FIFO_B for PCM sample streaming, configures
 * DMA1/DMA2 for automatic sound data transfer on timer overflow.
 *   109 lines
 *   HW: REG_FIFO_A (0x040000A0), REG_FIFO_B (0x040000A4),
 *       DMA1/2 SAD/DAD/CNT, REG_SOUNDBIAS (0x04000089),
 *       REG_SOUNDCNT_X (0x04000084)
 */
/*
 * SoundInit: full sound-engine initialisation — set up DMA1/DMA2 for
 * Direct Sound FIFO playback, zero soundInfo, seed numChans/masterVol/
 * plynote, install MP2K_event_null stubs for the optional CGB hooks,
 * copy the jump-table template, then arm timer via SampleFreqSet.
 *
 * Kleod-canonical name (the symbol our project formerly called
 * "SoundInit" — the tiny SoundMain wrapper at 0x0804FFBC — should be
 * renamed to "m4aSoundMain" in a separate port).
 *
 * Currently kept as INCLUDE_ASM: porting the body requires
 * declarations for a dozen DMA / sound-config register macros plus
 * MP2K_event_nxx/null function-pointer slots.  Deferred to next pass.
 */
INCLUDE_ASM("asm/nonmatchings/m4a", SoundInit);
/**
 * SampleFreqSet: configure timer 0 to drive PCM playback at the
 * requested frequency.  Picks samplesPerFrame from the LUT, derives
 * sampleRate / reciprocal, reloads TM0CNT_L with -(280896 /
 * samplesPerFrame), then re-arms DMA via m4aSoundVSyncOn.  Waits one
 * vblank window before re-enabling the timer for a clean restart.
 */
void SampleFreqSet(u32 freq) {
    struct SoundMixerState *soundInfo = SOUND_INFO_PTR;

    freq = (freq & 0xF0000) >> 16;
    soundInfo->freqOption = freq;

    soundInfo->samplesPerFrame = gPcmSamplesPerVBlankTable[freq - 1];
    soundInfo->framesPerDmaCycle = PCM_DMA_BUF_SIZE / soundInfo->samplesPerFrame;

    /* LCD refresh rate 59.7275 Hz. */
    soundInfo->sampleRate = (597275 * soundInfo->samplesPerFrame + 5000) / 10000;

    /* CPU freq 16.78 MHz. */
    soundInfo->sampleRateReciprocal = (0x1000000 / soundInfo->sampleRate + 1) >> 1;

    REG_TM0CNT_H = 0;
    REG_TM0CNT_L = -(280896 / soundInfo->samplesPerFrame);

    m4aSoundVSyncOn();

    while (*(vu8 *)0x04000006 == 159);
    while (*(vu8 *)0x04000006 != 159);

    REG_TM0CNT_H = TIMER_ENABLE | TIMER_1CLK;
}
/*
 * m4aSoundMode: configure sound system operating mode.
 * Sets reverb, mixing frequency, and channel allocation based
 * on the game's audio requirements.
 *   80 lines
 *   HW: REG_SOUNDBIAS (0x04000089)
 *   calls: SampleFreqSet, m4aSoundShutdown (m4aSoundVSyncOff)
 */
void m4aSoundMode(u32 mode) {
    u32 *soundInfo;
    u32 magic;
    u32 temp;

    soundInfo = gBiosSoundInfo;
    magic = soundInfo[0];

    if (magic != SAPPY_MAGIC)
        return;

    soundInfo[0] = magic + 1;

    temp = mode & 0xFF;
    if (temp) {
        temp &= 0x7F;
        ((u8 *)soundInfo)[5] = temp;
    }

    temp = 0xF0 << 4;
    temp &= mode;
    if (temp) {
        ((u8 *)soundInfo)[6] = temp >> 8;
        temp = 0x0C;
        {
            u8 *ch = (u8 *)soundInfo + 0x50;
            u8 zero = 0;
            do {
                *ch = zero;
                temp -= 1;
                ch += 0x40;
            } while (temp != 0);
        }
    }

    temp = 0xF0 << 8;
    temp &= mode;
    if (temp) {
        ((u8 *)soundInfo)[7] = temp >> 12;
    }

    temp = 0xB0 << 16;
    temp &= mode;
    if (temp) {
        u32 shifted;
        shifted = 0xC0 << 14;
        shifted &= temp;
        temp = shifted >> 14;
        {
            vu8 *bias = (vu8 *)0x04000089;
            u8 val = *bias;
            u8 r0 = 0x3F;
            r0 &= val;
            r0 |= temp;
            *bias = r0;
        }
    }

    temp = 0xF0 << 12;
    temp &= mode;
    if (temp) {
        m4aSoundVSyncOff();
        SampleFreqSet(temp);
    }

    soundInfo[0] = SAPPY_MAGIC;
}
/*
 * SoundPlatformDetect: detect audio platform capabilities.
 * Checks hardware version and adjusts sound parameters accordingly.
 *   45 lines, calls sub_0805186C
 */
/**
 * SoundClear: resets all channel status bytes and processes channels.
 *
 * Checks SAPPY_MAGIC, clears 12 channel status bytes (stride 0x40),
 * then calls sub_0805186C for channels 1-4 with the voice table.
 * Restores SAPPY_MAGIC on exit.
 */
void SoundClear(void) {
    u32 **soundInfoRef = &gBiosSoundInfo;
    u32 *soundInfo;
    u32 magic;
    s32 channelIdx;
    u8 *channelPtr;

    soundInfo = *soundInfoRef;
    magic = soundInfo[0];

    if (magic != SAPPY_MAGIC)
        return;

    soundInfo[0] = magic + 1;

    channelIdx = 12;
    channelPtr = (u8 *)soundInfo + 0x50;
    do {
        *channelPtr = 0;
        channelIdx--;
        channelPtr += 0x40;
    } while (channelIdx > 0);

    channelPtr = (u8 *)soundInfo[0x1C / 4];
    if (channelPtr != NULL) {
        channelIdx = 1;
        do {
            sub_0805186C((u8)channelIdx, soundInfo[0x2C / 4]);
            *channelPtr = 0;
            channelIdx++;
            channelPtr += 0x40;
        } while (channelIdx <= 4);
    }

    soundInfo[0] = SAPPY_MAGIC;
}
/*
 * m4aSoundShutdown: emergency stop — shut down all sound output.
 * Silences all channels, stops DMA, and resets hardware registers.
 * Called during fatal errors or system shutdown.
 *   59 lines, calls BitUnPack
 */
/**
 * m4aSoundShutdown: emergency shutdown of the sound system.
 *
 * Checks magic offset (0x978C92AD + magic <= 1 means active).
 * Increments magic by 10 to lock, stops DMA1/DMA2 if active,
 * sets DMA control to 0x0400 mode, then calls BitUnPack to
 * clear the channel state array.
 */
void m4aSoundVSyncOff(void) {
    u32 scratch;
    u32 *info = gBiosSoundInfo;
    u32 magic = info[0];
    vu32 *dmaReg;

    if (magic + 0x978C92AD > 1)
        return;

    info[0] = magic + 10;

    dmaReg = (vu32 *)0x040000C4;
    if (*dmaReg & 0x02000000)
        *dmaReg = 0x84400004;

    dmaReg = (vu32 *)0x040000D0;
    if (*dmaReg & 0x02000000)
        *dmaReg = 0x84400004;

    *(vu16 *)0x040000C6 = 0x0400;
    *(vu16 *)0x040000D2 = 0x0400;

    scratch = 0;
    BitUnPack(&scratch, (u8 *)info + 0x350, (u32 *)0x05000318);
}

/* ── VBlank Sound Update Pipeline ── */

/*
 * m4aSoundVSyncOn: register the VBlank sound handler.
 * Installs the VBlank callback that drives the sound engine's
 * per-frame update cycle. Must be called after SoundHardwareInit.
 *   30 lines, leaf function
 *   refs: REG_SOUNDCNT_H (0x040000C6)
 */
void m4aSoundVSyncOn(void) {
    u32 *info = gBiosSoundInfo;
    u32 magic = info[0];
    u8 scratch;
    if (magic == SAPPY_MAGIC)
        return;
    *(vu16 *)0x040000C6 = 0xB600;
    *(vu16 *)0x040000D2 = 0xB600;
    scratch = ((vu8 *)info)[0x04];
    scratch = 0;
    ((u8 *)info)[0x04] = scratch;
    info[0] = magic - 10;
}
/*
 * VBlankSoundCallback: VBlank-triggered sound update routine.
 * Called by the VBlank interrupt handler to process pending
 * sound events and trigger the next DMA cycle.
 *   63 lines, calls PlaySoundWithContext_DC
 */
/**
 * MPlayOpen: initialize a MusicPlayerInfo and register it in the SoundInfo
 * linked list.  Clamps trackCount to 16, zeroes each track's flags byte,
 * and chains the player into the MPlayMain callback list.
 */
void MP2KPlayerMain(void);
void MPlayOpen(u32 *mplayInfo, u8 *tracks, u8 trackCount) {
    u32 *soundInfo;
    u32 ident;

    if (trackCount == 0)
        return;

    if (trackCount > 16)
        trackCount = 16;

    soundInfo = gBiosSoundInfo;
    ident = soundInfo[0];

    if (ident != SAPPY_MAGIC)
        return;

    soundInfo[0] = ident + 1;

    Clear64byte((u32)mplayInfo);

    mplayInfo[0x2C / 4] = (u32)tracks;
    ((u8 *)mplayInfo)[0x08] = trackCount;
    mplayInfo[0x04 / 4] = 0x80000000;

    while (trackCount != 0) {
        tracks[0x00] = 0;
        trackCount--;
        tracks += 0x50;
    }

    if (soundInfo[0x20 / 4] != 0) {
        mplayInfo[0x38 / 4] = soundInfo[0x20 / 4];
        mplayInfo[0x3C / 4] = soundInfo[0x24 / 4];
        soundInfo[0x20 / 4] = 0;
    }

    soundInfo[0x24 / 4] = (u32)mplayInfo;
    soundInfo[0x20 / 4] = (u32)MP2KPlayerMain;
    soundInfo[0] = SAPPY_MAGIC;
    mplayInfo[0x34 / 4] = SAPPY_MAGIC;
}
/**
 * MPlayStart: start (or restart) the given song on a music player.
 * Honours checkSongPriority — if the player has a higher priority
 * song already loaded and active, does nothing.  Otherwise resets
 * tempo, fade, and per-track state, then seeds each track from the
 * song header's part pointers.
 */
void MPlayStart(struct MP2KPlayerState *mplayInfo, struct MP2KSongHeader *songHeader) {
    s32 i;
    u8 checkSongPriority;
    struct MP2KTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    checkSongPriority = mplayInfo->checkSongPriority;

    if (!checkSongPriority
        || ((!mplayInfo->songHeader || !(mplayInfo->tracks[0].status & MPT_FLG_START))
            && ((mplayInfo->status & MUSICPLAYER_STATUS_TRACK) == 0 || (mplayInfo->status & MUSICPLAYER_STATUS_PAUSE)))
        || (mplayInfo->priority <= songHeader->priority)) {
        mplayInfo->lockStatus++;
        mplayInfo->status = 0;
        mplayInfo->songHeader = songHeader;
        mplayInfo->voicegroup = songHeader->voicegroup;
        mplayInfo->priority = songHeader->priority;
        mplayInfo->clock = 0;
        mplayInfo->tempoRawBPM = 150;
        mplayInfo->tempoInterval = 150;
        mplayInfo->tempoScale = 0x100;
        mplayInfo->tempoCounter = 0;
        mplayInfo->fadeInterval = 0;

        i = 0;
        track = mplayInfo->tracks;

        while (i < songHeader->trackCount && i < mplayInfo->trackCount) {
            TrackStop(mplayInfo, track);
            track->status = MPT_FLG_EXIST | MPT_FLG_START;
            track->chan = 0;
            track->cmdPtr = songHeader->part[i];
            i++;
            track++;
        }

        while (i < mplayInfo->trackCount) {
            TrackStop(mplayInfo, track);
            track->status = 0;
            i++;
            track++;
        }

        if (songHeader->reverb & SOUND_MODE_REVERB_SET)
            m4aSoundMode(songHeader->reverb);

        mplayInfo->lockStatus = ID_NUMBER;
    }
}
/*
 * MPlayChannelUpdate: update a single music player channel.
 * Processes pending notes, advances timing, and updates the
 * channel's hardware registers for the current frame.
 *   35 lines, calls SoundContextRef
 */
/**
 * MPlayStop: stops all tracks in a music player.
 * Checks Sappy magic, locks engine, sets stop flag, iterates
 * all tracks calling SoundContextRef, then restores magic.
 */
void MPlayStop(u32 *player) {
    u32 magic = player[0x34 / 4];
    s32 numTracks;
    u8 *track;

    if (magic != 0x68736D53)
        return;

    player[0x34 / 4] = magic + 1;
    player[0x04 / 4] |= 0x80000000;

    numTracks = (s32)(u8)((u8 *)player)[0x08];
    track = (u8 *)player[0x2C / 4];

    while (numTracks > 0) {
        TrackStop((u32)player, (u32)track);
        numTracks--;
        track += 0x50;
    }

    player[0x34 / 4] = 0x68736D53;
}

/* ── Volume, Pitch & CGB Sound Control ── */

/**
 * FadeOutBody: per-frame fade processing (volume ramp + track stop).
 * Each fadeInterval ticks, steps fadeOV toward zero and updates each
 * track's volPublic; once the fade completes, pauses or stops all
 * tracks via TrackStop.  Called from asm/m4a0.s (MP2KPlayerMain).
 */
void FadeOutBody(struct MP2KPlayerState *mplayInfo) {
    s32 i;
    struct MP2KTrack *track;
    u16 fadeOV;

    if (mplayInfo->fadeInterval == 0)
        return;
    if (--mplayInfo->fadeCounter != 0)
        return;

    mplayInfo->fadeCounter = mplayInfo->fadeInterval;

    if (mplayInfo->fadeOV & FADE_IN) {
        if ((u16)(mplayInfo->fadeOV += (4 << FADE_VOL_SHIFT)) >= (64 << FADE_VOL_SHIFT)) {
            mplayInfo->fadeOV = (64 << FADE_VOL_SHIFT);
            mplayInfo->fadeInterval = 0;
        }
    } else {
        if ((s16)(mplayInfo->fadeOV -= (4 << FADE_VOL_SHIFT)) <= 0) {
            i = mplayInfo->trackCount;
            track = mplayInfo->tracks;

            while (i > 0) {
                u32 val;

                TrackStop(mplayInfo, track);

                val = TEMPORARY_FADE;
                fadeOV = mplayInfo->fadeOV;
                val &= fadeOV;

                if (!val)
                    track->status = 0;

                i--;
                track++;
            }

            if (mplayInfo->fadeOV & TEMPORARY_FADE)
                mplayInfo->status |= MUSICPLAYER_STATUS_PAUSE;
            else
                mplayInfo->status = MUSICPLAYER_STATUS_PAUSE;

            mplayInfo->fadeInterval = 0;
            return;
        }
    }

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;

    while (i > 0) {
        if (track->status & MPT_FLG_EXIST) {
            fadeOV = mplayInfo->fadeOV;

            track->volPublic = (fadeOV >> FADE_VOL_SHIFT);
            track->status |= MPT_FLG_VOLCHG;
        }

        i--;
        track++;
    }
}
/**
 * TrkVolPitSet: recompute a track's left/right volume and pitch caches
 * from its public fields (vol, pan, bend, tune, keyShift, mod).
 * Called from asm/m4a0.s (MP2K_event_nxx).
 */
void TrkVolPitSet(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track) {
    if (track->status & MPT_FLG_VOLSET) {
        s32 x;
        s32 y;

        x = (u32)(track->vol * track->volPublic) >> 5;

        if (track->modType == 1)
            x = (u32)(x * (track->modCalculated + 128)) >> 7;

        y = 2 * track->pan + track->panPublic;

        if (track->modType == 2)
            y += track->modCalculated;

        if (y < -128)
            y = -128;
        else if (y > 127)
            y = 127;

        track->volRightCalculated = (u32)((y + 128) * x) >> 8;
        track->volLeftCalculated = (u32)((127 - y) * x) >> 8;
    }

    if (track->status & MPT_FLG_PITSET) {
        s32 bend = track->bend * track->bendRange;
        s32 x = (track->tune + bend) * 4 + (track->keyShift << 8) + (track->keyShiftPublic << 8) + track->pitchPublic;

        if (track->modType == 0)
            x += 16 * track->modCalculated;

        track->keyShiftCalculated = x >> 8;
        track->pitchCalculated = x;
    }

    track->status &= ~(MPT_FLG_PITSET | MPT_FLG_VOLSET);
}
/**
 * MidiKeyToCgbFreq: convert MIDI note + fine adjust into the
 * frequency register value for a CGB sound channel.  Channel 4
 * (noise) uses gNoiseTable; channels 1-3 (square/wave) interpolate
 * over gCgbScaleTable + gCgbFreqTable.
 */
u32 MidiKeyToCgbFreq(u8 chanNum, u8 key, u8 fineAdjust) {
    if (chanNum == 4) {
        if (key <= 20) {
            key = 0;
        } else {
            key -= 21;
            if (key > 59)
                key = 59;
        }

        return gNoiseTable[key];
    } else {
        s32 val1;
        s32 val2;

        if (key <= 35) {
            fineAdjust = 0;
            key = 0;
        } else {
            key -= 36;
            if (key > 130) {
                key = 130;
                fineAdjust = 255;
            }
        }

        val1 = gCgbScaleTable[key];
        val1 = gCgbFreqTable[val1 & 0xF] >> (val1 >> 4);

        val2 = gCgbScaleTable[key + 1];
        val2 = gCgbFreqTable[val2 & 0xF] >> (val2 >> 4);

        return val1 + ((fineAdjust * (val2 - val1)) >> 8) + 2048;
    }
}
/**
 * CgbOscOff: silence a CGB sound channel.  Writes the "envelope off
 * + restart" pattern (NRx2=8, NRx4=0x80) for channels 1, 2, and 4,
 * or NR30=0 for the Wave channel (3).
 */
void CgbOscOff(u8 chanNum) {
    switch (chanNum) {
        case 1:
            REG_NR12 = 8;
            REG_NR14 = 0x80;
            break;
        case 2:
            REG_NR22 = 8;
            REG_NR24 = 0x80;
            break;
        case 3:
            REG_NR30 = 0;
            break;
        default:
            REG_NR42 = 8;
            REG_NR44 = 0x80;
    }
}
/* CgbPan: helper (inlined into CgbModVol below); sets chan->pan to
 * 0x0F or 0xF0 when one channel is at least 2× louder than the other,
 * returns 1 if a hard pan was applied. */
static inline int CgbPan(struct MixerSource *chan) {
    u32 rightVol = chan->rightVol;
    u32 leftVol = chan->leftVol;

    if ((rightVol = (u8)rightVol) >= (leftVol = (u8)leftVol)) {
        if (rightVol / 2 >= leftVol) {
            chan->pan = 0x0F;
            return 1;
        }
    } else {
        if (leftVol / 2 >= rightVol) {
            chan->pan = 0xF0;
            return 1;
        }
    }

    return 0;
}

/**
 * CgbModVol: compute a CGB channel's envelopeGoal, sustainGoal, and
 * pan (with CgbPan inlined).  Kleod-canonical.
 */
void CgbModVol(struct MixerSource *chan) {
    if (!CgbPan(chan)) {
        chan->pan = 0xFF;
        chan->envelopeGoal = (u32)(chan->rightVol + chan->leftVol) / 16;
    } else {
        chan->envelopeGoal = (u32)(chan->rightVol + chan->leftVol) / 16;

        if (chan->envelopeGoal > 15)
            chan->envelopeGoal = 15;
    }

    chan->sustainGoal = (chan->envelopeGoal * chan->sustain + 15) >> 4;
    chan->pan &= chan->panMask;
}
/*
 * CgbSound: drive all 4 CGB hardware channels every frame.  The
 * envelope / pitch / volume state machine for channels 1-4 plus the
 * Wave RAM upload for channel 3.  In the original ROM this function
 * spans both the CgbSound region (0x08050AFC) and the SoundMixerMain
 * tail region (0x08050C70) — they're a single function in kleod.
 */
INCLUDE_ASM("asm/nonmatchings/m4a", CgbSound);

/* ── Core Mixer / Channel Processing Loop ── */

/*
 * SoundMixerMain: CORE — process all mixer channels (2nd largest, 393 lines).
 * The main PCM mixing loop that combines all active Direct Sound channels
 * into the output buffer. Handles sample interpolation, volume mixing,
 * and stereo panning. Output is fed to FIFO via DMA.
 *   393 lines
 *   HW: REG_SOUNDCNT_H (0x04000081), REG_SOUNDBIAS (0x04000089)
 */
INCLUDE_ASM("asm/nonmatchings/m4a", SoundMixerMain);

/* ── Music Player Control API ── */

/**
 * m4aMPlayTempoControl: scale a player's tempo by `tempo / 256`.
 * tempoScale is the user-facing 0..0x100 ratio; tempoInterval is the
 * tick count derived from tempoRawBPM * tempoScale.
 */
void m4aMPlayTempoControl(struct MP2KPlayerState *mplayInfo, u16 tempo) {
    if (mplayInfo->lockStatus == ID_NUMBER) {
        mplayInfo->lockStatus++;
        mplayInfo->tempoScale = tempo;
        mplayInfo->tempoInterval = (mplayInfo->tempoRawBPM * mplayInfo->tempoScale) >> 8;
        mplayInfo->lockStatus = ID_NUMBER;
    }
}
/**
 * m4aMPlayVolumeControl: set every track's volPublic to `volume / 4`
 * for each track whose bit is set in trackBits.
 */
void m4aMPlayVolumeControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, u16 volume) {
    s32 i;
    u32 bit;
    struct MP2KTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->volPublic = volume / 4;
                track->status |= MPT_FLG_VOLCHG;
            }
        }
        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}
/**
 * m4aMPlayPitchControl: set per-track keyShiftPublic (semitones, high
 * byte of pitch) and pitchPublic (1/256 semitone, low byte) for each
 * track whose bit is in trackBits.
 */
void m4aMPlayPitchControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, s16 pitch) {
    s32 i;
    u32 bit;
    struct MP2KTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->keyShiftPublic = pitch >> 8;
                track->pitchPublic = pitch;
                track->status |= MPT_FLG_PITCHG;
            }
        }
        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

/**
 * m4aMPlayPanpotControl: set each selected track's panPublic.
 */
void m4aMPlayPanpotControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, s8 pan) {
    s32 i;
    u32 bit;
    struct MP2KTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->panPublic = pan;
                track->status |= MPT_FLG_VOLCHG;
            }
        }
        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}

/* ── MIDI Note & Command Encoding ── */
/**
 * ClearModM: reset a track's LFO/modulation accumulators and flag the
 * appropriate per-frame recalc (pitch or volume) depending on modType.
 * Called whenever modDepth or lfoSpeed drops to zero.
 */
void ClearModM(struct MP2KTrack *track) {
    track->lfoSpeedCounter = 0;
    track->modCalculated = 0;
    if (track->modType == 0)
        track->status |= MPT_FLG_PITCHG;
    else
        track->status |= MPT_FLG_VOLCHG;
}
/**
 * m4aMPlayModDepthSet: set per-track modDepth; clear LFO accumulators
 * (via ClearModM) when modDepth drops to zero.
 */
void m4aMPlayModDepthSet(struct MP2KPlayerState *mplayInfo, u16 trackBits, u8 modDepth) {
    s32 i;
    u32 bit;
    struct MP2KTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->modDepth = modDepth;

                if (!track->modDepth)
                    ClearModM(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}
/**
 * m4aMPlayLFOSpeedSet: set per-track lfoSpeed; clear LFO accumulators
 * (via ClearModM) when lfoSpeed drops to zero.
 */
void m4aMPlayLFOSpeedSet(struct MP2KPlayerState *mplayInfo, u16 trackBits, u8 lfoSpeed) {
    s32 i;
    u32 bit;
    struct MP2KTrack *track;

    if (mplayInfo->lockStatus != ID_NUMBER)
        return;

    mplayInfo->lockStatus++;

    i = mplayInfo->trackCount;
    track = mplayInfo->tracks;
    bit = 1;

    while (i > 0) {
        if (trackBits & bit) {
            if (track->status & MPT_FLG_EXIST) {
                track->lfoSpeed = lfoSpeed;

                if (!track->lfoSpeed)
                    ClearModM(track);
            }
        }

        i--;
        track++;
        bit <<= 1;
    }

    mplayInfo->lockStatus = ID_NUMBER;
}
/*
 * MPlayCommandDispatch: music player command dispatcher.
 * Reads a command byte from the track stream and dispatches to the
 * appropriate handler via a ROM-based function pointer table.
 *   174 lines
 *   calls: sub_08051870
 *   refs: gSoundCmdTablePtr (0x03006454), ROM command table (0x080511F0)
 */
INCLUDE_ASM("asm/nonmatchings/m4a", MPlayCommandDispatch);
/*
 * SoundCommandHandler: dispatch a sound command from ROM_SOUND_CMD_TABLE.
 * Reads a command byte, indexes into the command table at 0x08117C8C,
 * and calls the corresponding handler function.
 *   r0: sound context, r1: channel struct
 *   16 lines, calls sub_08051870
 */
void sub_08051870(u32, u32 *, u32);
void SoundCmd_Dispatch(u32 ctx, u32 *channel) {
    u8 *cmdPtr = (u8 *)channel[0x40 / 4];
    u8 cmd = *cmdPtr;
    channel[0x40 / 4] = (u32)(cmdPtr + 1);
    sub_08051870(ctx, channel, gSoundCmdTable[cmd]);
}
/** SoundCommand_6450: dispatches a sound command using gSoundTablePtr. */
void SoundCommand_6450(u32 ctx, u32 channel) {
    sub_08051870(ctx, channel, gSoundTablePtr);
}
/*
 * BitMaskLUT: 32-bit channel bitmask lookup table.
 * Contains precomputed masks (0x00FFFFFF, 0xFF00FFFF, 0xFFFF00FF,
 * 0xFFFFFF00) for isolating individual byte lanes in 32-bit channel
 * state words. Used by the mixer to update per-channel volumes.
 *   118 lines, leaf function
 */
INCLUDE_ASM("asm/nonmatchings/m4a", MPlayCmd_ReadU32Param);
typedef struct {
    u8 unk00[0x1E];
    u8 unk1E;
    u8 unk1F;
    u8 unk20[0x04];
    u8 keyShift;
    u8 unk25;
    u8 unk26;
    u8 unk27;
    u8 unk28[0x04];
    u8 unk2C;
    u8 unk2D;
    u8 unk2E;
    u8 unk2F[0x11];
    u8 *unk40;
} TrackStruct;

void ply_keysh(void *r0, TrackStruct *track) {
    track->keyShift = *track->unk40;
    track->unk40++;
}
asm(".align 2, 0");
void ply_voice(void *r0, TrackStruct *track) {
    track->unk2C = *track->unk40;
    track->unk40++;
}

void ply_vol(void *r0, TrackStruct *track) {
    track->unk2D = *track->unk40;
    track->unk40++;
}

void ply_pan(void *r0, TrackStruct *track) {
    track->unk2E = *track->unk40;
    track->unk40++;
}
asm(".align 2, 0");
void ply_bend(void *r0, TrackStruct *track) {
    track->unk2F[0] = *track->unk40;
    track->unk40++;
}
asm(".align 2, 0");
void ply_bendr(void *r0, TrackStruct *r1) {
    u8 *ptr;
    ptr = r1->unk40;
    r1->unk1E = *ptr;
    r1->unk40 = ptr + 1;
}

void ply_lfos(void *r0, TrackStruct *r1) {
    u8 *ptr;
    ptr = r1->unk40;
    r1->unk1F = *ptr;
    r1->unk40 = ptr + 1;
}
void ply_lfodl(void *r0, TrackStruct *track) {
    track->unk26 = *track->unk40;
    track->unk40++;
}
asm(".align 2, 0");
void ply_mod(void *r0, TrackStruct *track) {
    track->unk27 = *track->unk40;
    track->unk40++;
}
asm(".align 2, 0");
asm("bx lr");
asm(".align 2, 0");
