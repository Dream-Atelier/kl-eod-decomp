#ifndef GUARD_GBA_H
#define GUARD_GBA_H

#include "global.h"
#include "io_reg.h"

/* ── Memory region base addresses ── */

#define EWRAM                      0x02000000
#define IWRAM                      0x03000000
#define IO_REG                     0x04000000
#define PAL_RAM                    0x05000000
#define BG_PAL_RAM                 0x05000000
#define OBJ_PAL_RAM                0x05000200
#define VRAM                       0x06000000
#define OBJ_VRAM                   0x06010000
#define OAM                        0x07000000
#define ROM                        0x08000000

/* ── DISPSTAT flag bits (project-specific subset; full set in io_reg.h) ── */
#define DISPSTAT_VBLANK_IRQ_ENABLE DISPSTAT_VBLANK_INTR

/* ── IE flag bits (project-specific alias; full set in io_reg.h) ── */
#define IE_VBLANK                  INTR_FLAG_VBLANK

/* ── WAITCNT subset masks used by our existing C ── */
#define WAITCNT_WS2_N_MASK         (3 << 8)
#define WAITCNT_WS2_S_MASK         (1 << 10)

/* ── DMA helpers (matching kleod naming for portability) ── */
/* clang-format off */
#define DmaSetWait(dmaNum, src, dest, control)    \
{                                                 \
    vu32 *dmaRegs = (vu32 *)REG_ADDR_DMA##dmaNum; \
    dmaRegs[0] = (vu32)(src);                     \
    dmaRegs[1] = (vu32)(dest);                    \
    dmaRegs[2] = (vu32)(control);                 \
    dmaRegs[2];                                   \
    while (dmaRegs[2] & (DMA_ENABLE << 16));      \
}

#define DMA_COPY_WAIT(dmaNum, src, dest, size, bit)                                         \
    DmaSetWait(dmaNum,                                                                      \
           src,                                                                             \
           dest,                                                                            \
           (DMA_ENABLE | DMA_START_NOW | DMA_##bit##BIT | DMA_SRC_INC | DMA_DEST_INC) << 16 \
         | ((size)/(bit/8)))

#define DmaCopy16Wait(dmaNum, src, dest, size) DMA_COPY_WAIT(dmaNum, src, dest, size, 16)
#define DmaCopy32Wait(dmaNum, src, dest, size) DMA_COPY_WAIT(dmaNum, src, dest, size, 32)
/* clang-format on */

#endif /* GUARD_GBA_H */
