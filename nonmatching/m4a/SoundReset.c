typedef unsigned char u8; typedef signed char s8; typedef unsigned short u16; typedef signed short s16; typedef unsigned int u32; typedef signed int s32; typedef volatile unsigned short vu16; typedef volatile unsigned int vu32;
void SoundReset(s16 r0, s16 r1, u16 r2) { u16 *dst = (u16 *)(0x03000900 + 0x800 + (r0 + (r1 << 5)) * 2); *dst = r2 | 0xF000; }
