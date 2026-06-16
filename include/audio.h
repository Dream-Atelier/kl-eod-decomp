#ifndef GUARD_AUDIO_H
#define GUARD_AUDIO_H

#include "global.h"

/* ── Sound / Music (m4a / MusicPlayer2000 / Sappy engine) ── */

/* Main sound info struct pointer. Contains channel state, mixer config,
 * and pointers to the currently playing tracks. */
#define gSoundInfo     (*(u32 *)0x0300081C)

/* Music player context pointers. Each MusicPlayer instance has its own
 * context for independent track playback. */
#define gMPlayInfo_BGM (*(u32 *)0x030064D8)
#define gMPlayInfo_SE  (*(u32 *)0x030064DC)

/* Sound command dispatch table pointer.
 * (Same slot as gMPlayJumpTable[0] in m4a_internal.h; declared here as
 * a raw u32 for SoundCommand_6450's untyped indirect call.) */
#define gSoundTablePtr (*(u32 *)0x03006450)

/* Sappy engine magic marker: "Smsh" (0x68736D53) in little-endian.
 * Used to verify the sound engine is properly initialized. */
#define SAPPY_MAGIC    0x68736D53

/* ── Sound ROM Data Tables ── */

/* struct MusicPlayer, struct Song, gMPlayTable, gSongTable are declared
 * in include/m4a_internal.h with typed fields. */

/* Sound command dispatch table.
 * Array of function pointers indexed by command byte.  An alias
 * gXcmdTable resolves to the same address via ldscript. */
extern const u32 gSoundCmdTable[];

/* Sound configuration init data. */
#define ROM_SOUND_INIT_DATA 0x081177E4

#endif /* GUARD_AUDIO_H */
