#ifndef GUARD_INPUT_H
#define GUARD_INPUT_H

#include "global.h"

/* ── Input System ── */

/* Current frame pressed keys (active-high, edge-detected).
 * Written by ReadKeyInput each frame. */
#define gKeysPressed   (*(u16 *)0x03004DA0)

/* Previous frame raw key state (active-high).
 * Used for edge detection in ReadKeyInput. */
#define gKeysPrevious  (*(u16 *)0x030051E4)

/* Extended input state for ProcessInputAndTimers.
 * Separate from the simple ReadKeyInput state. */
#define gInputState    (*(u16 *)0x03004668)
#define gInputPrevious (*(u16 *)0x0300362C)

#endif /* GUARD_INPUT_H */
