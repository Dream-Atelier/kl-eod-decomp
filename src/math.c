#include "global.h"
#include "include_asm.h"

extern s16 FUN_080518a4(s32, s16);

s16 FUN_08000960(s16 arg0, s16 arg1) {
    return FUN_080518a4((s32)arg0 << 8, arg1);
}
s16 FUN_08000978(s16 arg0) {
    return FUN_080518a4(0x10000, arg0);
}
INCLUDE_ASM("asm/nonmatchings/math", FUN_08000992);
/*
 * FixedPointMul4: fixed-point multiply with 4-bit left shift on first arg.
 * Computes FUN_080518a4(arg0 << 4, arg1) and sign-extends the result to s16.
 *   arg0: s16 multiplicand (shifted left 4 = x16 scaling)
 *   arg1: s16 multiplier
 *   returns: s16 result
 */
s16 FUN_080009a8(s16 arg0, s16 arg1) {
    return FUN_080518a4((s32)arg0 << 4, arg1);
}
s16 FUN_080009c2(s16 arg0) {
    return FUN_080518a4(0x100, arg0);
}
INCLUDE_ASM("asm/nonmatchings/math", FUN_080009da);
INCLUDE_ASM("asm/nonmatchings/math", FUN_08000ab2);
