#ifndef GUARD_M4A_INTERNAL_H
#define GUARD_M4A_INTERNAL_H

#include "global.h"
#include "globals.h"

/*
 * Types and constants for the m4a (MusicPlayer2000 / Sappy) sound engine,
 * mirroring kleod's include/gba/m4a_internal.h.  Grown incrementally as
 * functions in src/m4a.c adopt kleod's C bodies.
 *
 * Naming follows kleod / pokeemerald / sa3 (canonical Sappy names).
 * SAPPY_MAGIC (our project) and ID_NUMBER (kleod) are the same value;
 * the alias keeps kleod-style sources verbatim.
 */
#define ID_NUMBER                SAPPY_MAGIC

/* I/O register accessors (REG_NRxx, REG_DMA*, REG_TM*, etc.) live in
 * include/io_reg.h, transitively pulled in via gba.h.  This header
 * only adds m4a-engine-specific symbols on top. */

/* Pointer slot in BIOS sound-info area (0x03007FF0) — points at the
 * project's SoundMixerState (in IWRAM).  Kleod-style alias. */
#define SOUND_INFO_PTR           (*(struct SoundMixerState **)0x03007FF0)

#define PCM_DMA_BUF_SIZE         1584
#define MAX_DIRECTSOUND_CHANNELS 12

/* m4aSoundMode mode-word bit layout (matches kleod).
 *
 *  bit  0..6  reverb amount               (SOUND_MODE_REVERB_VAL)
 *  bit  7     reverb-set flag             (SOUND_MODE_REVERB_SET)
 *  bit  8..11 max DirectSound channels    (SOUND_MODE_MAXCHN, shift 8)
 *  bit 12..15 master volume               (SOUND_MODE_MASVOL,  shift 12)
 *  bit 16..19 mixer frequency preset      (SOUND_MODE_FREQ,    shift 16)
 *  bit 20..23 DAC resolution / DA bit     (SOUND_MODE_DA_BIT,  shift 20)
 */
#define SOUND_MODE_REVERB_VAL    0x0000007F
#define SOUND_MODE_REVERB_SET    0x00000080
#define SOUND_MODE_MAXCHN        0x00000F00
#define SOUND_MODE_MAXCHN_SHIFT  8
#define SOUND_MODE_MASVOL        0x0000F000
#define SOUND_MODE_MASVOL_SHIFT  12
#define SOUND_MODE_FREQ_05734    0x00010000
#define SOUND_MODE_FREQ_07884    0x00020000
#define SOUND_MODE_FREQ_10512    0x00030000
#define SOUND_MODE_FREQ_13379    0x00040000
#define SOUND_MODE_FREQ_15768    0x00050000
#define SOUND_MODE_FREQ_18157    0x00060000
#define SOUND_MODE_FREQ_21024    0x00070000
#define SOUND_MODE_FREQ_26758    0x00080000
#define SOUND_MODE_FREQ_31536    0x00090000
#define SOUND_MODE_FREQ_36314    0x000A0000
#define SOUND_MODE_FREQ_40137    0x000B0000
#define SOUND_MODE_FREQ_42048    0x000C0000
#define SOUND_MODE_FREQ          0x000F0000
#define SOUND_MODE_FREQ_SHIFT    16
#define SOUND_MODE_DA_BIT_9      0x00800000
#define SOUND_MODE_DA_BIT_8      0x00900000
#define SOUND_MODE_DA_BIT_7      0x00A00000
#define SOUND_MODE_DA_BIT_6      0x00B00000
#define SOUND_MODE_DA_BIT        0x00B00000
#define SOUND_MODE_DA_BIT_SHIFT  20

#define DEFAULT_SOUND_MODE                                                                                                            \
    (SOUND_MODE_DA_BIT_8 | SOUND_MODE_FREQ_13379 | (15 << SOUND_MODE_MASVOL_SHIFT) | (8 << SOUND_MODE_MAXCHN_SHIFT))

/* CpuFill32: BIOS-driven fill via the same SVC trampoline our project
 * exposes as BitUnPack.  Control word: bit 24 (fill), bit 26 (32-bit),
 * low bits = word count. */
#define CpuFill32(value, dst, size)                                                                                                   \
    do {                                                                                                                              \
        u32 _zero = (value);                                                                                                          \
        BitUnPack((u32) & _zero, (u32)(dst), 0x05000000 | ((size) / 4));                                                              \
    } while (0)

/* MP2KTrack.status flag bits. */
#define MPT_FLG_VOLSET               0x01
#define MPT_FLG_VOLCHG               0x03
#define MPT_FLG_PITSET               0x04
#define MPT_FLG_PITCHG               0x0C
#define MPT_FLG_START                0x40
#define MPT_FLG_EXIST                0x80

/* MP2KPlayerState.status flag bits. */
#define MUSICPLAYER_STATUS_TRACK     0x0000FFFF
#define MUSICPLAYER_STATUS_PAUSE     0x80000000

/* MPlayFadeOut shift / FadeOutBody constants. */
#define FADE_VOL_SHIFT               2
#define FADE_IN                      0x0002
#define TEMPORARY_FADE               0x0001

/* MixerSource.status flag bits — note these overlap with envelope-state values. */
#define SOUND_CHANNEL_SF_START       0x80
#define SOUND_CHANNEL_SF_STOP        0x40
#define SOUND_CHANNEL_SF_LOOP        0x10
#define SOUND_CHANNEL_SF_IEC         0x04
#define SOUND_CHANNEL_SF_ENV         0x03
#define SOUND_CHANNEL_SF_ENV_ATTACK  0x03
#define SOUND_CHANNEL_SF_ENV_DECAY   0x02
#define SOUND_CHANNEL_SF_ENV_SUSTAIN 0x01
#define SOUND_CHANNEL_SF_ENV_RELEASE 0x00
#define SOUND_CHANNEL_SF_ON          (SOUND_CHANNEL_SF_START | SOUND_CHANNEL_SF_STOP | SOUND_CHANNEL_SF_IEC | SOUND_CHANNEL_SF_ENV)

#define CGB_CHANNEL_MO_PIT           0x02
#define CGB_CHANNEL_MO_VOL           0x01
#define CGB_NRx2_ENV_DIR_DEC         0x00
#define CGB_NRx2_ENV_DIR_INC         0x08

#define TONEDATA_TYPE_FIX            0x08

/*
 * Per-track playback state.  Field offsets match what the hand-written
 * MP2KPlayerMain / TrackStop / TrkVolPitSet code in asm/m4a0.s reads
 * and writes; do not reorder.
 *
 * Only fields needed by already-ported C are spelled out.  When a port
 * touches a new offset, name it and replace the surrounding gap.
 */
/*
 * Voicegroup header embedded in each track at offset 0x24, total 16
 * bytes.  Inner union holds sample/keySplit-specific fields starting
 * at offset 0x04 (= track offset 0x28).
 */
struct MP2KVoiceGroup {
    u8 type; // 0x00
    u8 drumKey; // 0x01
    u8 cgbLength; // 0x02
    u8 pan_sweep; // 0x03 (pan or sweep depending on type)
    union {
        struct {
            struct WaveData *wav; // 0x04
            u8 attack; // 0x08
            u8 decay; // 0x09
            u8 sustain; // 0x0A
            u8 release; // 0x0B
        } sound;
        struct {
            struct MP2KVoiceGroup *group; // 0x04
            u8 *keySplitTable; // 0x08
        } keySplit;
    } data;
};

struct MP2KSongHeader;
struct MP2KPlayerState;

/*
 * Single mixer channel slot.  Full struct = 0x40 bytes.  Union shape
 * matches kleod: `data.cgb.*` for CGB channels, `data.sound.*` for
 * DirectSound channels.
 */
struct MixerSource {
    u8 status; // 0x00
    u8 type; // 0x01
    u8 rightVol; // 0x02
    u8 leftVol; // 0x03
    union {
        struct {
            u8 attack; // 0x04
            u8 decay; // 0x05
            u8 sustain; // 0x06
            u8 release; // 0x07
            u8 key; // 0x08
            u8 envelopeVol; // 0x09
            u8 envelopeGoal; // 0x0A
            u8 envelopeCtr; // 0x0B
            u8 echoVol; // 0x0C
            u8 echoLen; // 0x0D
            u8 padding1; // 0x0E
            u8 padding2; // 0x0F
            u8 gateTime; // 0x10
            u8 untransposedKey; // 0x11
            u8 velocity; // 0x12
            u8 priority; // 0x13
            u8 rhythmPan; // 0x14
            u8 padding3; // 0x15
            u8 padding4; // 0x16
            u8 padding5; // 0x17
            u8 padding6; // 0x18
            u8 sustainGoal; // 0x19
            u8 nrx4; // 0x1A
            u8 pan; // 0x1B
            u8 panMask; // 0x1C
            u8 cgbStatus; // 0x1D
            u8 length; // 0x1E
            u8 sweep; // 0x1F
            u32 freq; // 0x20
        } cgb;
        struct {
            u8 attack; // 0x04
            u8 decay; // 0x05
            u8 sustain; // 0x06
            u8 release; // 0x07
            u8 key; // 0x08
            u8 envelopeVol; // 0x09
            u8 envelopeVolR; // 0x0A
            u8 envelopeVolL; // 0x0B
            u8 echoVol; // 0x0C
            u8 echoLen; // 0x0D
            u8 padding1; // 0x0E
            u8 padding2; // 0x0F
            u8 gateTime; // 0x10
            u8 untransposedKey; // 0x11
            u8 velocity; // 0x12
            u8 priority; // 0x13
            u8 rhythmPan; // 0x14
            u8 padding3; // 0x15
            u8 padding4; // 0x16
            u8 padding5; // 0x17
            u32 ct; // 0x18
            u32 fw; // 0x1C  (fixed8_24)
            u32 freq; // 0x20
        } sound;
    } data;
    void *wav; // 0x24
    void *current; // 0x28
    struct MP2KTrack *track; // 0x2C
    struct MixerSource *prev; // 0x30
    struct MixerSource *next; // 0x34
    u32 padding7; // 0x38
    u32 blockCount; // 0x3C
};

struct MP2KTrack {
    u8 status; // 0x00
    u8 wait; // 0x01
    u8 patternLevel; // 0x02
    u8 repeatCount; // 0x03
    u8 gateTime; // 0x04 (0 if TIE)
    u8 key; // 0x05
    u8 velocity; // 0x06
    u8 runningStatus; // 0x07
    s8 keyShiftCalculated; // 0x08 — calculated by TrkVolPitSet; semitones
    u8 pitchCalculated; // 0x09 — 256ths of a semitone
    s8 keyShift; // 0x0A — semitones
    s8 keyShiftPublic; // 0x0B — semitones
    s8 tune; // 0x0C — 64ths of a semitone
    u8 pitchPublic; // 0x0D — 256ths of a semitone
    s8 bend; // 0x0E — (bendRange/64)ths of a semitone
    u8 bendRange; // 0x0F
    u8 volRightCalculated; // 0x10
    u8 volLeftCalculated; // 0x11
    u8 vol; // 0x12
    u8 volPublic; // 0x13 — used by fades and MPlayVolumeControl
    s8 pan; // 0x14
    s8 panPublic; // 0x15
    s8 modCalculated; // 0x16 — pitch units: 16ths of a semitone
    u8 modDepth; // 0x17
    u8 modType; // 0x18
    u8 lfoSpeed; // 0x19
    u8 lfoSpeedCounter; // 0x1A
    u8 lfoDelay; // 0x1B
    u8 lfoDelayCounter; // 0x1C
    u8 priority; // 0x1D
    u8 echoVolume; // 0x1E
    u8 echoLength; // 0x1F
    struct MixerSource *chan; // 0x20
    struct MP2KVoiceGroup voicegroup; // 0x24..0x2F (12 bytes)
    u8 gap_30[0x0A]; // 0x30..0x39
    u16 unk_3A; // 0x3A
    u32 unk_3C; // 0x3C
    u8 *cmdPtr; // 0x40
    u8 *patternStack[3]; // 0x44, 0x48, 0x4C (track size = 0x50)
};

/*
 * Song header in ROM.  Variable-length: `part` is the start of an
 * inline array of u8 pointers sized by trackCount.
 */
struct MP2KSongHeader {
    u8 trackCount; // 0x00
    u8 blockCount; // 0x01
    u8 priority; // 0x02
    u8 reverb; // 0x03
    struct MP2KVoiceGroup *voicegroup; // 0x04
    u8 *part[1]; // 0x08..
};

/* Function-pointer types matching kleod's SoundMixerState slots.
 * Declared early so MP2KPlayerState below can spell its MPlayMain slot. */
typedef void (*MP2KEventNoteFunc)(u8, struct MP2KPlayerState *, struct MP2KTrack *);
typedef void (*MP2KEventFunc)(struct MP2KPlayerState *, struct MP2KTrack *);
typedef void (*CgbSoundFunc)(void);
typedef void (*CgbOscOffFunc)(u8);
typedef u32 (*MidiKeyToCgbFreqFunc)(u8, u8, u8);
typedef void (*ExtVolPitFunc)(void);
typedef void (*MPlayMainFunc)(struct MP2KPlayerState *);

/*
 * Per-player state.  Field offsets must match what asm/m4a0.s reads.
 * Only named fields are spelled out; the rest is gap padding to keep
 * later offsets correct.
 */
struct MP2KPlayerState {
    struct MP2KSongHeader *songHeader; // 0x00
    u32 status; // 0x04
    u8 trackCount; // 0x08
    u8 priority; // 0x09
    u8 cmd; // 0x0A
    bool8 checkSongPriority; // 0x0B
    u32 clock; // 0x0C
    u8 padding[8]; // 0x10..0x17
    u8 *memAccArea; // 0x18
    u16 tempoRawBPM; // 0x1C
    u16 tempoScale; // 0x1E
    u16 tempoInterval; // 0x20
    u16 tempoCounter; // 0x22
    u16 fadeInterval; // 0x24
    u16 fadeCounter; // 0x26
    u16 fadeOV; // 0x28 (2 bytes implicit alignment pad before tracks)
    struct MP2KTrack *tracks; // 0x2C
    struct MP2KVoiceGroup *voicegroup; // 0x30
    u32 lockStatus; // 0x34
    MPlayMainFunc nextPlayerFunc; // 0x38
    struct MP2KPlayerState *nextPlayer; // 0x3C
};

/*
 * Top-level sound-engine state, pointed to by *SOUND_INFO_PTR.  Only
 * the fields touched by already-ported C are named; the rest is gap
 * padding.  Full struct is ~0x350 bytes in kleod.
 */
/*
 * MusicPlayer: 12-byte entry in gMPlayTable.  Kleod-canonical layout.
 * The original game has 4 entries (BGM, SE1, SE2, SE3) at 0x08118AB4.
 */
struct MusicPlayer {
    struct MP2KPlayerState *info; // 0x00
    struct MP2KTrack *track; // 0x04
    u8 numTracks; // 0x08
    u16 unk_A; // 0x0A (1 byte padding at 0x09)
};

/*
 * Song: 8-byte entry in gSongTable.  Kleod-canonical layout.
 * Indexed by song ID; song->ms picks the player slot in gMPlayTable.
 */
struct Song {
    struct MP2KSongHeader *header; // 0x00
    u16 ms; // 0x04
    u16 me; // 0x06
};

extern const struct MusicPlayer gMPlayTable[];
extern const struct Song gSongTable[];

struct SoundMixerState {
    u32 lockStatus; // 0x00
    vu8 dmaCounter; // 0x04
    u8 reverb; // 0x05
    u8 numChans; // 0x06
    u8 masterVol; // 0x07
    u8 freqOption; // 0x08
    u8 extensionFlags; // 0x09
    u8 cgbCounter15; // 0x0A
    u8 framesPerDmaCycle; // 0x0B
    u8 maxScanlines; // 0x0C
    u8 gap[3]; // 0x0D..0x0F
    s32 samplesPerFrame; // 0x10
    s32 sampleRate; // 0x14
    s32 sampleRateReciprocal; // 0x18
    struct MixerSource *cgbChans; // 0x1C
    MPlayMainFunc MPlayMainHead; // 0x20
    struct MP2KPlayerState *musicPlayerHead; // 0x24
    CgbSoundFunc CgbSound; // 0x28
    CgbOscOffFunc CgbOscOff; // 0x2C
    MidiKeyToCgbFreqFunc MidiKeyToCgbFreq; // 0x30
    void **MPlayJumpTable; // 0x34
    MP2KEventNoteFunc plynote; // 0x38
    ExtVolPitFunc ExtVolPit; // 0x3C
    void *reserved2; // 0x40
    void *reserved3; // 0x44
    void *reserved4; // 0x48
    void *reserved5; // 0x4C
    struct MixerSource chans[MAX_DIRECTSOUND_CHANNELS]; // 0x50..0x34F (12 * 0x40 = 0x300)
    s8 pcmBuffer[PCM_DMA_BUF_SIZE * 2]; // 0x350..0xFAF
};

extern const u16 gPcmSamplesPerVBlankTable[];
extern void *gMPlayJumpTable[];

void m4aSoundVSyncOn(void);

/* MP2K MIDI-event handlers in asm/m4a0.s, referenced by MPlayExtender + SoundInit. */
void MP2K_event_lfos(void);
void MP2K_event_mod(void);
void MP2K_event_endtie(void);
void MP2K_event_nxx(u8 clock, struct MP2KPlayerState *, struct MP2KTrack *);
void MPlayJumpTableCopy(void **mplayJumpTable);
void MusicPlayerJumpTableCopy(void); /* tiny `swi 0x2A` C wrapper after MPlayExtender */

void MP2K_event_memacc(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track);
void MP2K_event_xcmd(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track);
void MP2K_event_xwave(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track);

typedef void (*XcmdFunc)(struct MP2KPlayerState *, struct MP2KTrack *);
extern const XcmdFunc gXcmdTable[];

/* PCM sample header (for kleod-style MidiKeyToFreq port). */
struct WaveData {
    u16 type; // 0x00
    u16 status; // 0x02
    u32 freq; // 0x04
    u32 loopStart; // 0x08
    u32 size; // 0x0C
    s8 data[1]; // 0x10
};

extern const u8 gScaleTable[];
extern const u32 gFreqTable[];
extern const u8 gCgbScaleTable[];
extern const s16 gCgbFreqTable[];
extern const u8 gNoiseTable[];
extern const u8 gCgb3Vol[];

/* Linker-provided "address as integer" constants (kleod-canonical idiom).
 * The ldscript assigns each of these to a small numeric value (e.g.
 * gNumMusicPlayers = 4, gMaxLines = 0).  The symbol's *address* is that
 * value, so the (u16)/(u32) cast yields the intended integer.            */
extern char gNumMusicPlayers[];
extern char gMaxLines[];
#define NUM_MUSIC_PLAYERS ((u16)gNumMusicPlayers)
#define MAX_LINES         ((u32)gMaxLines)

/* SoundMainRAM: thumb function label inside the handcrafted asm/m4a0.s
 * blob.  m4aSoundInit copies it into IWRAM at 0x03000388 via BitUnPack;
 * we only need its address, not its prototype.                          */
extern char SoundMainRAM[];

/* CGB voice mixer-source array used as the MPlayExtender argument
 * (IWRAM 0x030064E0). */
extern struct MixerSource gCgbChans[];

/* Memory-access scratch area assigned into each player's memAccArea
 * field at the end of m4aSoundInit (IWRAM 0x030066A0). */
extern u8 gMPlayMemAccArea[];

u32 umul3232H32(u32 multiplier, u32 multiplicand);
u32 MidiKeyToFreq(struct WaveData *wav, u8 key, u8 fineAdjust);
u32 MidiKeyToCgbFreq(u8 chanNum, u8 key, u8 fineAdjust);

void ClearModM(struct MP2KTrack *track);
void MPlayContinue(struct MP2KPlayerState *mplayInfo);
void MPlayFadeOut(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayFadeOut(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayFadeOutTemporarily(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayFadeIn(struct MP2KPlayerState *mplayInfo, u16 speed);
void m4aMPlayTempoControl(struct MP2KPlayerState *mplayInfo, u16 tempo);
void m4aMPlayVolumeControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, u16 volume);
void m4aMPlayPitchControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, s16 pitch);
void m4aMPlayPanpotControl(struct MP2KPlayerState *mplayInfo, u16 trackBits, s8 pan);
void m4aMPlayModDepthSet(struct MP2KPlayerState *mplayInfo, u16 trackBits, u8 modDepth);
void m4aMPlayLFOSpeedSet(struct MP2KPlayerState *mplayInfo, u16 trackBits, u8 lfoSpeed);
void m4aMPlayContinue(struct MP2KPlayerState *mplayInfo);
void MP2K_event_null(void);
void m4aSongNumStart(u16 n);
void m4aSongNumStartOrChange(u16 n);
void m4aSongNumStartOrContinue(u16 n);
void m4aSongNumStop(u16 n);
void m4aSongNumContinue(u16 n);
void m4aMPlayAllStop(void);
void m4aMPlayAllContinue(void);
void CgbOscOff(u8 chanNum);
void Clear64byte(u32 addr);
void m4aMPlayImmInit(struct MP2KPlayerState *mplayInfo);
void TrkVolPitSet(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track);
void SampleFreqSet(u32 freq);
void FadeOutBody(struct MP2KPlayerState *mplayInfo);
void TrackStop(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *track);
void MPlayStart(struct MP2KPlayerState *mplayInfo, struct MP2KSongHeader *songHeader);
void m4aSoundMode(u32 mode);
void MPlayExtender(struct MixerSource *cgbChans);
void CgbSound(void);
void CgbModVol(struct MixerSource *chan);

/* Klonoa-EOD m4a entry points still INCLUDE_ASM but called from C. */
void SoundInit(struct SoundMixerState *soundInfo);
void MPlayOpen(struct MP2KPlayerState *mplayInfo, struct MP2KTrack *tracks, u8 trackCount);
void MP2KPlayerMain(void);
u32 MidiProcessEvent(void);

#endif // GUARD_M4A_INTERNAL_H
