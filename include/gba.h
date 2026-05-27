#ifndef GUARD_GBA_H
#define GUARD_GBA_H

#include "global.h"

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

/* ── LCD I/O Registers ── */

#define REG_DISPCNT                (*(vu16 *)0x04000000)
#define REG_DISPSTAT               (*(vu16 *)0x04000004)
#define REG_VCOUNT                 (*(vu16 *)0x04000006)

#define REG_BG0CNT                 (*(vu16 *)0x04000008)
#define REG_BG1CNT                 (*(vu16 *)0x0400000A)
#define REG_BG2CNT                 (*(vu16 *)0x0400000C)
#define REG_BG3CNT                 (*(vu16 *)0x0400000E)

#define REG_BG0HOFS                (*(vu16 *)0x04000010)
#define REG_BG0VOFS                (*(vu16 *)0x04000012)
#define REG_BG1HOFS                (*(vu16 *)0x04000014)
#define REG_BG1VOFS                (*(vu16 *)0x04000016)
#define REG_BG2HOFS                (*(vu16 *)0x04000018)
#define REG_BG2VOFS                (*(vu16 *)0x0400001A)
#define REG_BG3HOFS                (*(vu16 *)0x0400001C)
#define REG_BG3VOFS                (*(vu16 *)0x0400001E)

/* ── Window Registers ── */

#define REG_WIN0H                  (*(vu16 *)0x04000040)
#define REG_WIN1H                  (*(vu16 *)0x04000042)
#define REG_WIN0V                  (*(vu16 *)0x04000044)
#define REG_WIN1V                  (*(vu16 *)0x04000046)
#define REG_WININ                  (*(vu16 *)0x04000048)
#define REG_WINOUT                 (*(vu16 *)0x0400004A)

/* ── Special Effects ── */

#define REG_BLDCNT                 (*(vu16 *)0x04000050)
#define REG_BLDALPHA               (*(vu16 *)0x04000052)
#define REG_BLDY                   (*(vu16 *)0x04000054)

/* ── Sound Channel 1 (Square with Sweep) ── */

#define REG_SOUND1CNT_L            (*(vu16 *)0x04000060)
#define REG_SOUND1CNT_H            (*(vu16 *)0x04000062)
#define REG_SOUND1CNT_X            (*(vu16 *)0x04000064)

/* ── Sound Channel 2 (Square) ── */

#define REG_SOUND2CNT_L            (*(vu16 *)0x04000068)
#define REG_SOUND2CNT_H            (*(vu16 *)0x0400006C)

/* ── Sound Channel 3 (Wave) ── */

#define REG_SOUND3CNT_L            (*(vu16 *)0x04000070)
#define REG_SOUND3CNT_H            (*(vu16 *)0x04000072)
#define REG_SOUND3CNT_X            (*(vu16 *)0x04000074)

/* ── Sound Channel 4 (Noise) ── */

#define REG_SOUND4CNT_L            (*(vu16 *)0x04000078)
#define REG_SOUND4CNT_H            (*(vu16 *)0x0400007C)

/* ── Master Sound Control ── */

#define REG_SOUNDCNT_L             (*(vu16 *)0x04000080)
#define REG_SOUNDCNT_H             (*(vu16 *)0x04000082)
#define REG_SOUNDCNT_X             (*(vu16 *)0x04000084)
#define REG_SOUNDBIAS              (*(vu16 *)0x04000088)

/* ── Wave RAM ── */

#define REG_WAVE_RAM0              (*(vu32 *)0x04000090)
#define REG_WAVE_RAM1              (*(vu32 *)0x04000094)
#define REG_WAVE_RAM2              (*(vu32 *)0x04000098)
#define REG_WAVE_RAM3              (*(vu32 *)0x0400009C)

/* ── Direct Sound FIFO ── */

#define REG_FIFO_A                 (*(vu32 *)0x040000A0)
#define REG_FIFO_B                 (*(vu32 *)0x040000A4)

/* ── DMA Registers ── */

#define REG_DMA0SAD                (*(vu32 *)0x040000B0)
#define REG_DMA0DAD                (*(vu32 *)0x040000B4)
#define REG_DMA0CNT                (*(vu32 *)0x040000B8)

#define REG_DMA1SAD                (*(vu32 *)0x040000BC)
#define REG_DMA1DAD                (*(vu32 *)0x040000C0)
#define REG_DMA1CNT                (*(vu32 *)0x040000C4)
#define REG_DMA1CNT_H              (*(vu16 *)0x040000C6)

#define REG_DMA2SAD                (*(vu32 *)0x040000C8)
#define REG_DMA2DAD                (*(vu32 *)0x040000CC)
#define REG_DMA2CNT                (*(vu32 *)0x040000D0)
#define REG_DMA2CNT_H              (*(vu16 *)0x040000D2)

#define REG_DMA3SAD                (*(vu32 *)0x040000D4)
#define REG_DMA3DAD                (*(vu32 *)0x040000D8)
#define REG_DMA3CNT                (*(vu32 *)0x040000DC)
#define REG_DMA3CNT_H              (*(vu16 *)0x040000DE)

#define REG_WAITCNT                (*(vu16 *)0x04000204)
#define WAITCNT_WS2_N_MASK         (3 << 8)
#define WAITCNT_WS2_S_MASK         (1 << 10)

/* ── Timer Registers ── */

#define REG_ADDR_TMCNT             0x04000100
#define REG_TMCNT(n)               (*(vu32 *)(REG_ADDR_TMCNT + ((n) * 4)))

#define REG_TM0CNT_L               (*(vu16 *)0x04000100)
#define REG_TM0CNT_H               (*(vu16 *)0x04000102)
#define REG_TM1CNT_L               (*(vu16 *)0x04000104)
#define REG_TM1CNT_H               (*(vu16 *)0x04000106)
#define REG_TM2CNT_L               (*(vu16 *)0x04000108)
#define REG_TM2CNT_H               (*(vu16 *)0x0400010A)
#define REG_TM3CNT_L               (*(vu16 *)0x0400010C)
#define REG_TM3CNT_H               (*(vu16 *)0x0400010E)

/* ── Interrupt / System Registers ── */

#define REG_IE                     (*(vu16 *)0x04000200)
#define REG_IF                     (*(vu16 *)0x04000202)
#define REG_IME                    (*(vu16 *)0x04000208)

/* ── DISPSTAT flag bits ── */

#define DISPSTAT_VBLANK_IRQ_ENABLE 0x0008

/* ── IE flag bits ── */

#define IE_VBLANK                  0x0001

/* ── DMA control flags (halfword form, suitable for REG_DMAxCNT_H or
 *    shifted-left-16 into REG_DMAxCNT) ── */

#define DMA_DEST_INC               0x0000
#define DMA_DEST_DEC               0x0020
#define DMA_DEST_FIXED             0x0040
#define DMA_DEST_RELOAD            0x0060
#define DMA_SRC_INC                0x0000
#define DMA_SRC_DEC                0x0080
#define DMA_SRC_FIXED              0x0100
#define DMA_REPEAT                 0x0200
#define DMA_16BIT                  0x0000
#define DMA_32BIT                  0x0400
#define DMA_DREQ_ON                0x0800
#define DMA_START_NOW              0x0000
#define DMA_START_VBLANK           0x1000
#define DMA_START_HBLANK           0x2000
#define DMA_START_SPECIAL          0x3000
#define DMA_IRQ_ENABLE             0x4000
#define DMA_ENABLE                 0x8000

/* ── SOUNDCNT_X master flags (byte at 0x04000084) ── */
#define SOUND_MASTER_ENABLE        0x80
#define SOUND_4_ON                 0x08
#define SOUND_3_ON                 0x04
#define SOUND_2_ON                 0x02
#define SOUND_1_ON                 0x01

/* ── SOUNDCNT_H Direct Sound mixer flags (halfword at 0x04000082) ── */
/* SOUND_ALL_MIX_FULL = Sound1-4 ratio 100% (0x02) | DSoundA ratio full
 * (0x04) | DSoundB ratio full (0x08). */
#define SOUND_ALL_MIX_FULL         0x000E
#define SOUND_A_RIGHT_OUTPUT       0x0100
#define SOUND_A_LEFT_OUTPUT        0x0200
#define SOUND_A_TIMER_0            0x0000
#define SOUND_A_TIMER_1            0x0400
#define SOUND_A_FIFO_RESET         0x0800
#define SOUND_B_RIGHT_OUTPUT       0x1000
#define SOUND_B_LEFT_OUTPUT        0x2000
#define SOUND_B_TIMER_0            0x0000
#define SOUND_B_TIMER_1            0x4000
#define SOUND_B_FIFO_RESET         0x8000

/* ── SOUNDBIAS byte accessor (high byte) ── */
#define REG_SOUNDBIAS_H            (*(vu8 *)0x04000089)

#endif /* GUARD_GBA_H */
