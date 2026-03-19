typedef unsigned char u8;
typedef unsigned int u32;

void SoundChannelRelease(u32 r0, u32 r1, u32 r2, u32 r3);

/* Logic is correct but register allocation differs:
 * - byte/mask check: ours r1/r0, original r0/r3
 * - info loading: ours r0, original r3
 * - strb zero: ours mov+strb, original uses pre-loaded r6
 * Needs -ftst (compile via m4a_1.c)
 */
void SoundContextRef(u32 unused, u32 *track) {
    u32 *node;
    u8 status = *(u8 *)track;

    if (!(status & 0x80))
        return;

    node = (u32 *)track[0x20 / 4];
    if (node == 0)
        goto store_node;

    {
        u32 zero = 0;
        do {
            if (*(u8 *)node != 0) {
                u32 masked = *((u8 *)node + 1) & 0x07;
                if (masked) {
                    u32 ctx = (*(u32 **)0x03007FF0)[0x2C / 4];
                    SoundChannelRelease(masked, (u32)track, (u32)node, ctx);
                }
                *(u8 *)node = zero;
            }
            node[0x2C / 4] = zero;
            node = (u32 *)node[0x34 / 4];
        } while (node != 0);
    }

store_node:
    track[0x20 / 4] = (u32)node;
}
