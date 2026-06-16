#ifndef GUARD_SAVE_H
#define GUARD_SAVE_H

#include "global.h"

/* ── Save System ── */

/* Save data buffer pointers: four IWRAM addresses used together by
 * SaveGameWithVerify, SavePlayerProgress, SaveGameRetryLoop.
 * Each holds a pointer or buffer for EEPROM read/write operations. */
#define gSaveBuffer0 (*(u32 *)0x0300520C)
#define gSaveBuffer1 (*(u32 *)0x03005208)
#define gSaveBuffer2 (*(u32 *)0x0300465C)
#define gSaveBuffer3 (*(u32 *)0x030008E4)

#endif /* GUARD_SAVE_H */
