typedef unsigned char u8; typedef signed char s8; typedef unsigned short u16; typedef signed short s16; typedef unsigned int u32; typedef signed int s32; typedef volatile unsigned short vu16; typedef volatile unsigned int vu32;
void MPlayChannelReset(u32 *player) { if (player[0x34/4] != 0x68736D53) return; player[0x04/4] &= 0x7FFFFFFF; }
