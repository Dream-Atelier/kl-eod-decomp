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
#define ID_NUMBER SAPPY_MAGIC

/* Byte-level CGB sound-channel register accessors (NRxx names).
 * gba.h has the halfword SOUNDxCNT_L/H/X aliases; CgbOscOff and
 * CgbModVol need the byte granularity of the original NRxx layout. */
#define REG_NR12 (*(vu8 *)0x04000063)
#define REG_NR14 (*(vu8 *)0x04000065)
#define REG_NR22 (*(vu8 *)0x04000069)
#define REG_NR24 (*(vu8 *)0x0400006D)
#define REG_NR30 (*(vu8 *)0x04000070)
#define REG_NR42 (*(vu8 *)0x04000079)
#define REG_NR44 (*(vu8 *)0x0400007D)
#define REG_NR50 (*(vu8 *)0x04000080)  /* same address as SOUNDCNT_L low byte */

/* Pointer slot in BIOS sound-info area (0x03007FF0) — points at the
 * project's SoundMixerState (in IWRAM).  Kleod-style alias. */
#define SOUND_INFO_PTR (*(struct SoundMixerState **)0x03007FF0)

#define PCM_DMA_BUF_SIZE 1584
#define TIMER_ENABLE     0x0080
#define TIMER_1CLK       0x0000

/* MP2KTrack.status flag bits. */
#define MPT_FLG_VOLSET 0x01
#define MPT_FLG_VOLCHG 0x03
#define MPT_FLG_PITSET 0x04
#define MPT_FLG_PITCHG 0x0C
#define MPT_FLG_START  0x40
#define MPT_FLG_EXIST  0x80

/* MP2KPlayerState.status flag bits. */
#define MUSICPLAYER_STATUS_TRACK 0x0000FFFF
#define MUSICPLAYER_STATUS_PAUSE 0x80000000

/* MPlayFadeOut shift / FadeOutBody constants. */
#define FADE_VOL_SHIFT 2
#define FADE_IN        0x0002
#define TEMPORARY_FADE 0x0001

/* m4aSoundMode mode-word bits. */
#define SOUND_MODE_REVERB_SET 0x00000080

/*
 * Per-track playback state.  Field offsets match what the hand-written
 * MP2KPlayerMain / TrackStop / TrkVolPitSet code in asm/m4a0.s reads
 * and writes; do not reorder.
 *
 * Only fields needed by already-ported C are spelled out.  When a port
 * touches a new offset, name it and replace the surrounding gap.
 */
/*
 * Voicegroup header embedded in each track at offset 0x24.  Only the
 * `type` byte is named here; the union of sound/keySplit data follows.
 */
struct MP2KVoiceGroup {
    u8 type;        // 0x00
    u8 gap_01[0x0B];// 0x01..0x0B (12 bytes — full struct is 16)
};

struct MP2KSongHeader;
struct MP2KPlayerState;

/*
 * Single mixer channel slot.  Full struct = 0x40 bytes.  Only the
 * fields touched by ported C are named; data union is collapsed to
 * a gap with one named field at offset 0x18 (data.cgb.panMask) for
 * MPlayExtender.
 */
struct MixerSource {
    u8 status;        // 0x00
    u8 type;          // 0x01
    u8 rightVol;      // 0x02
    u8 leftVol;       // 0x03
    u8 gap_04[0x02];  // 0x04..0x05 (data.cgb.attack/decay)
    u8 sustain;       // 0x06 (data.cgb.sustain)
    u8 gap_07[0x03];  // 0x07..0x09 (release, key, envelopeVol)
    u8 envelopeGoal;  // 0x0A (data.cgb.envelopeGoal)
    u8 gap_0B[0x0E];  // 0x0B..0x18 (envelopeCtr..padding6)
    u8 sustainGoal;   // 0x19 (data.cgb.sustainGoal)
    u8 gap_1A;        // 0x1A (data.cgb.nrx4)
    u8 pan;           // 0x1B (data.cgb.pan)
    u8 panMask;       // 0x1C  (data.cgb.panMask)
    u8 gap_1D[0x23];  // 0x1D..0x3F  (rest of data + wav/current/track/prev/next/padding7/blockCount)
};

struct MP2KTrack {
    u8 status;                  // 0x00
    u8 gap_01[0x07];            // 0x01..0x07
    s8 keyShiftCalculated;      // 0x08
    u8 pitchCalculated;         // 0x09
    s8 keyShift;                // 0x0A
    s8 keyShiftPublic;          // 0x0B
    s8 tune;                    // 0x0C
    u8 pitchPublic;             // 0x0D
    s8 bend;                    // 0x0E
    u8 bendRange;               // 0x0F
    u8 volRightCalculated;      // 0x10
    u8 volLeftCalculated;       // 0x11
    u8 vol;                     // 0x12
    u8 volPublic;               // 0x13
    s8 pan;                     // 0x14
    s8 panPublic;               // 0x15
    s8 modCalculated;           // 0x16
    u8 modDepth;                // 0x17
    u8 modType;                 // 0x18
    u8 lfoSpeed;                // 0x19
    u8 lfoSpeedCounter;         // 0x1A
    u8 gap_1B[0x05];            // 0x1B..0x1F
    void *chan;                 // 0x20
    struct MP2KVoiceGroup voicegroup;  // 0x24..0x2F (16 bytes)
    u8 gap_30[0x10];            // 0x30..0x3F
    u8 *cmdPtr;                 // 0x40
    u8 gap_44[0x0C];            // 0x44..0x4F (track size = 0x50)
};

/*
 * Song header in ROM.  Variable-length: `part` is the start of an
 * inline array of u8 pointers sized by trackCount.
 */
struct MP2KSongHeader {
    u8 trackCount;                       // 0x00
    u8 blockCount;                       // 0x01
    u8 priority;                         // 0x02
    u8 reverb;                           // 0x03
    struct MP2KVoiceGroup *voicegroup;   // 0x04
    u8 *part[1];                         // 0x08..
};

/*
 * Per-player state.  Field offsets must match what asm/m4a0.s reads.
 * Only named fields are spelled out; the rest is gap padding to keep
 * later offsets correct.
 */
struct MP2KPlayerState {
    struct MP2KSongHeader *songHeader;   // 0x00
    u32 status;                          // 0x04
    u8 trackCount;                       // 0x08
    u8 priority;                         // 0x09
    u8 cmd;                              // 0x0A
    u8 checkSongPriority;                // 0x0B
    u32 clock;                           // 0x0C
    u8 gap_10[0x0C];                     // 0x10..0x1B
    u16 tempoRawBPM;                     // 0x1C
    u16 tempoScale;                      // 0x1E
    u16 tempoInterval;                   // 0x20
    u16 tempoCounter;                    // 0x22
    u16 fadeInterval;                    // 0x24
    u16 fadeCounter;                     // 0x26
    u16 fadeOV;                          // 0x28
    u8 gap_2A[0x02];                     // 0x2A..0x2B
    struct MP2KTrack *tracks;            // 0x2C
    struct MP2KVoiceGroup *voicegroup;   // 0x30
    u32 lockStatus;                      // 0x34
    u8 gap_38[0x08];                     // 0x38..0x3F
};

/*
 * Top-level sound-engine state, pointed to by *SOUND_INFO_PTR.  Only
 * the fields touched by already-ported C are named; the rest is gap
 * padding.  Full struct is ~0x350 bytes in kleod.
 */
struct SoundMixerState {
    u32 lockStatus;             // 0x00
    u8 gap_04[4];               // 0x04..0x07 (dmaCounter, reverb, numChans, masterVol)
    u8 freqOption;              // 0x08
    u8 gap_09[2];               // 0x09..0x0A (extensionFlags, cgbCounter15)
    u8 framesPerDmaCycle;       // 0x0B
    u8 maxScanlines;            // 0x0C
    u8 gap_0D[3];               // 0x0D..0x0F  (3 bytes gap)
    s32 samplesPerFrame;        // 0x10
    s32 sampleRate;             // 0x14
    s32 sampleRateReciprocal;   // 0x18
    struct MixerSource *cgbChans;  // 0x1C
    void *MPlayMainHead;        // 0x20
    void *musicPlayerHead;      // 0x24
    void (*CgbSound)(void);     // 0x28
    void (*CgbOscOff)(u8);      // 0x2C
    u32 (*MidiKeyToCgbFreq)(u8, u8, u8);  // 0x30
};

extern const u16 gPcmSamplesPerVBlankTable[];
extern void *gMPlayJumpTable[];

void m4aSoundVSyncOn(void);

/* MP2K MIDI-event handlers in asm/m4a0.s, referenced by MPlayExtender. */
void MP2K_event_lfos(void);
void MP2K_event_mod(void);
void MP2K_event_endtie(void);

/* Klonoa-specific names for MP2K event handlers (== kleod names in
 * jump-table order; ldscript aliases would let us use kleod names
 * directly, but we keep ours for now to minimise call-site churn). */
void MPlayCommandDispatch(void);   /* == MP2K_event_memacc (slot 8) */
void SoundCmd_Dispatch(u32, u32 *);/* == MP2K_event_xcmd (slot 28) */

/* PCM sample header (for kleod-style MidiKeyToFreq port). */
struct WaveData {
    u16 type;       // 0x00
    u16 status;     // 0x02
    u32 freq;       // 0x04
    u32 loopStart;  // 0x08
    u32 size;       // 0x0C
    s8 data[1];     // 0x10
};

extern const u8 gScaleTable[];
extern const u32 gFreqTable[];
extern const u8 gCgbScaleTable[];
extern const s16 gCgbFreqTable[];
extern const u8 gNoiseTable[];

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
void MPlayOpen(u32 *mplayInfo, u8 *tracks, u8 trackCount);
void MP2KPlayerMain(void);
u32 MidiProcessEvent(void);

#endif // GUARD_M4A_INTERNAL_H
