#include "global.h"
#include "include_asm.h"

extern s16 FUN_080518a4(s32, s16);

s16 FUN_08000960(s16 arg0, s16 arg1) {
    return FUN_080518a4((s32)arg0 << 8, arg1);
}
INCLUDE_ASM("asm/nonmatchings/math", FUN_08000978);
__attribute__((naked))
s16 FUN_08000992(s16 arg0, s16 arg1) {
    asm(
        "lsl r0, r0, #16\n\t"
        "asr r0, r0, #16\n\t"
        "lsl r1, r1, #16\n\t"
        "asr r1, r1, #16\n\t"
        "mul r0, r1\n\t"
        "mov r1, r0\n\t"
        "cmp r0, #0\n\t"
        ".2byte 0xDA00\n\t"
        "add r1, #15\n\t"
        "lsl r0, r1, #12\n\t"
        "asr r0, r0, #16\n\t"
        "bx lr\n\t"
    );
}
INCLUDE_ASM("asm/nonmatchings/math", FUN_080009a8);
s16 FUN_080009c2(s16 arg0) {
    return FUN_080518a4(0x100, arg0);
}
INCLUDE_ASM("asm/nonmatchings/math", FUN_080009da);
INCLUDE_ASM("asm/nonmatchings/math", FUN_08000ab2);
