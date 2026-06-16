#include "global.h"
#include "game.h"
#include "include_asm.h"

extern s16 sub_080518A4(s32, s16);

/**
 * DivideQ8: fixed-point division with 8-bit left shift.
 * Computes sub_080518A4(arg0 << 8, arg1).
 */
s16 DivideQ8(s16 num1, s16 num2) {
    return (num1 << 8) / num2;
}

/**
 * ReciprocalQ8: fixed-point reciprocal (1.0 / arg0).
 * Computes sub_080518A4(0x10000, arg0) = 65536 / arg0.
 */
s16 ReciprocalQ8(s16 num1) {
    s32 numerator = 0x10000;
    return (numerator / num1);
}

/** MultiplyQ4: 4.4 fixed-point signed multiply (s16*s16 >> 4). */
s16 MultiplyQ4(s16 num1, s16 num2) {
    s32 product;
    s32 rounded;

    product = num1 * num2;
    rounded = product;
    if (rounded < 0) {
        rounded += 0xF;
    }
    product = rounded >> 4;
    return product;
}

/**
 * DivideQ4: fixed-point multiply with 4-bit left shift.
 * Computes sub_080518A4(arg0 << 4, arg1).
 */
s16 DivideQ4(s16 num1, s16 num2) {
    return ((num1 << 4) / num2);
}

/**
 * ReciprocalQ4: fixed-point division by unit scale (256).
 * Computes sub_080518A4(0x100, arg0) = 256 / arg0.
 */
s16 ReciprocalQ4(s16 num1) {
    s32 numerator = 0x100;
    return (numerator / num1);
}

/**
 * VBlankHandler: main VBlank interrupt handler.
 *
 * DMAs screen buffers A/B/C to VRAM screenbases, DMAs the tilemap work
 * buffer, DMAs OAM shadow to hardware OAM, checks sound reset flag,
 * sets IME to acknowledge the interrupt.
 */
INCLUDE_ASM("asm/nonmatchings/math", VBlankHandler);

/**
 * VBlankHandlerMinimal: simplified VBlank for scene transitions.
 * Calls m4aSoundVSync + m4aSoundMain, sets IME.
 */
void m4aSoundVSync(void);
void m4aSoundMain(void);

void VBlankHandlerMinimal(void) {
    m4aSoundVSync();
    m4aSoundMain();
    gIMEAcknowledge = 1;
}
