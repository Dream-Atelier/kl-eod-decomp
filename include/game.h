#ifndef GUARD_GAME_H
#define GUARD_GAME_H

#include "global.h"

/* ── Game State ── */

/* Pause flag: when non-zero, GameUpdate skips the main update loop. */
#define gPauseFlag      (*(vu8 *)0x030034E4)

/* Frame/tick counter — decremented each frame, byte-sized. */
#define gFrameCounter   (*(u8 *)0x03005498)

/* Sound reset flag: when non-zero, VBlankHandler calls m4aSoundMain. */
#define gSoundResetFlag (*(u8 *)0x03003420)

/* Interrupt Master Enable write address for VBlank acknowledge. */
#define gIMEAcknowledge (*(u16 *)0x03007FF8)

/* ── Camera / Scroll State ── */

/* Camera state struct (accessed with s16 fields at various offsets).
 * Offset +0x0C low nibble = camera mode index (0-7, switch in CameraModeSwitchHandler).
 * Used by UpdateScrollPosition, UpdatePlayer*, CameraModeSwitchHandler. */
#define gCameraState    ((u8 *)0x030007E0)

/* Camera/scroll limits computed from level dimensions.
 * Used by UpdateScrollPosition, ComputeScrollLimits, InitLevelState. */
#define gScrollLimits   ((u8 *)0x030007CC)

/* Main loop state flag: controls game loop flow in MainGameFrameLoop.
 * Checked at loop entry and after transitions. */
#define gMainLoopState  (*(u8 *)0x030007F8)

#endif /* GUARD_GAME_H */
