#ifndef GUARD_SAVE_H
#define GUARD_SAVE_H

#include "global.h"

/* ── Save System ── */

/* Shadow copies of the four DMA channel control halfwords, saved and restored
 * around every EEPROM access.
 *
 * EEPROM transfers drive DMA3 themselves and are cycle-critical, so the save
 * routines (SaveGameToSRAM, SaveGameWithVerify, LoadGameFromSRAM,
 * EraseAllSaveData) all open with the same idiom: copy DMA0..3 CNT_H into
 * these four slots, clear bit 15 (enable) on all four, do the EEPROM work,
 * then write the saved halfwords back.  Confirmed at runtime -- DMA1/DMA2 were
 * observed live at 0xB600, masked to 0x3600 for the duration, and restored.
 * See docs/unreachable-code-investigation.md.
 *
 * These are NOT save buffers or pointers; an earlier comment here said they
 * were.  They are u16, and each mirrors exactly one REG_DMAnCNT_H. */
#define gDma0CntShadow (*(u16 *)0x0300520C) /* mirrors REG_DMA0CNT_H (0x040000BA) */
#define gDma1CntShadow (*(u16 *)0x03005208) /* mirrors REG_DMA1CNT_H (0x040000C6) */
#define gDma2CntShadow (*(u16 *)0x0300465C) /* mirrors REG_DMA2CNT_H (0x040000D2) */
#define gDma3CntShadow (*(u16 *)0x030008E4) /* mirrors REG_DMA3CNT_H (0x040000DE) */

#endif /* GUARD_SAVE_H */
