#ifndef GUARD_INPUT_H
#define GUARD_INPUT_H

#include "global.h"

/* ── Input System ── */

/* The two key globals ReadKeyInput writes every frame, both active-high
 * (REG_KEYINPUT is active-low, so it stores `0x3FF ^ REG_KEYINPUT`):
 *
 *     u16 now  = 0x3FF ^ REG_KEYINPUT;
 *     gNewKeys  = now & ~gHeldKeys;   // edge: read gHeldKeys before overwriting
 *     gHeldKeys = now;
 *
 * So gNewKeys is high only on the frame a button goes down, while gHeldKeys
 * stays high for as long as it is held. Confirmed at runtime — see
 * docs/dynamic-analysis/scripts/prove-key-globals.mjs: holding A for six
 * frames gives gNewKeys 1,0,0,0,0,0 and gHeldKeys 1,1,1,1,1,1.
 *
 * Use gNewKeys for "was just pressed" and gHeldKeys for "is being held". */

/* Buttons that went down THIS frame (active-high, edge-detected). */
extern u16 gNewKeys;

/* Buttons currently held (active-high). */
extern u16 gHeldKeys;

/* Extended input state for ProcessInputAndTimers.
 * Separate from the simple ReadKeyInput state. */
#define gInputState    (*(u16 *)0x03004668)
#define gInputPrevious (*(u16 *)0x0300362C)

/* A-button hold counter: incremented each frame while A is held, reset
 * to 0 on release.  Used by ReadKeyInput for press-and-hold detection. */
extern u16 gAButtonHold;

#endif /* GUARD_INPUT_H */
