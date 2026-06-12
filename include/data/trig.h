#ifndef GUARD_DATA_TRIG_H
#define GUARD_DATA_TRIG_H

#include "global.h"

/* gSineTable: 320 s16 entries (5 quadrants × 64), ROM address 0x080D8E14.
 * Q_8_8 fixed-point: full circle = 256, so PI = 128 and PI/2 = 64.
 * Indexed by u8, COS reuses the table shifted by PI/2 (wraparound entries
 * past index 255 cover the high-input case).
 * Declared extern (with linker-assigned address) — declaring as a
 * literal-address macro would let the compiler constant-fold
 * `gSineTable + PI_2*2` in COS(), breaking matching. */
extern const s16 gSineTable[];

/* Q_8_8(0.5) = 128 = half a rotation; Q_8_8(1.0) = 256 = full rotation. */
#define Q_8_8(n)   ((s16)((n) * 256))
#define PI         Q_8_8(.5f)
#define PI_2       (PI / 2)

#define SIN(value) (gSineTable[(value)])
#define COS(value) (gSineTable[(value) + PI_2])

#endif /* GUARD_DATA_TRIG_H */
