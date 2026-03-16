#include "global.h"
#include "include_asm.h"

INCLUDE_ASM("asm/nonmatchings/util", FUN_08051450);
INCLUDE_ASM("asm/nonmatchings/util", FUN_080514b2);
INCLUDE_ASM("asm/nonmatchings/util", FUN_0805150c);
INCLUDE_ASM("asm/nonmatchings/util", FUN_08051570);
INCLUDE_ASM("asm/nonmatchings/util", FUN_080515b4);
INCLUDE_ASM("asm/nonmatchings/util", FUN_08051634);
INCLUDE_ASM("asm/nonmatchings/util", FUN_080516ec);
INCLUDE_ASM("asm/nonmatchings/util", FUN_080517d0);
/**
 * SaveGameRetry: attempts write+verify save up to 3 times.
 * Calls FUN_080516ec (write) then FUN_080517d0 (verify).
 */
INCLUDE_ASM("asm/nonmatchings/util", SaveGameRetry);
