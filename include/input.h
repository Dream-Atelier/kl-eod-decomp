#ifndef GUARD_INPUT_H
#define GUARD_INPUT_H

#include "global.h"

/* ── Input System ── */

/* Current frame pressed keys (active-high, edge-detected).
 * Written by ReadKeyInput each frame. */
extern u16 gKeysPressed;

/* Previous frame raw key state (active-high).
 * Used for edge detection in ReadKeyInput. */
extern u16 gKeysPrevious;

/* Extended input state for ProcessInputAndTimers.
 * Separate from the simple ReadKeyInput state. */
#define gInputState    (*(u16 *)0x03004668)
#define gInputPrevious (*(u16 *)0x0300362C)

/* A-button hold counter: incremented each frame while A is held, reset
 * to 0 on release.  Used by ReadKeyInput for press-and-hold detection. */
extern u16 gAButtonHold;

#endif /* GUARD_INPUT_H */
