#include "global.h"
#include "include_asm.h"

INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804eb64);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ed68);
/*
 * Frees a sound data structure and its inner buffer.
 * Reads the struct pointer from 0x0300081C, frees *(struct) - 4 (inner buffer
 * with header), then frees the struct pointer itself.
 *   no parameters
 *   no return value
 */
void FreeSoundStruct(void) {
    u32 *p = (u32 *)0x0300081C;
    thunk_FUN_0800020c(*(u32 *)(*p) - 4);
    thunk_FUN_0800020c(*p);
}
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ee34);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ee60);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ef50);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804efde);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f004);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f0d0);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f180);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f1c4);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f248);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f6d4);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f6f4);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f724);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f73c);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f758);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f766);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f7b4);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804f944);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fb9c);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fbe0);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fc10);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fe50);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fe6c);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fea0);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ff08);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ff44);
/*
 * Wrapper that calls FUN_0804f294 to initialize the sound engine.
 *   no parameters
 *   no return value
 */
void SoundInit(void) {
    FUN_0804f294();
}
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804ffc8);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0804fff6);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050042);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050094);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080500c8);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080500fc);
/*
 * Wrapper that calls FUN_0804ff08 to stop/reset a sound channel.
 * Passes through r0 (sound struct pointer).
 *   r0: pointer to sound struct
 *   no return value
 */
void StopSoundChannel(u32 r0) {
    FUN_0804ff08(r0);
}
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050134);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050162);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080501ba);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050200);
/*
 * Calls FUN_0805186c with the given parameter and a global sound context
 * pointer from 0x030064D8 as the second argument.
 *   r0: first argument passed through to FUN_0805186c
 *   no return value
 */
void PlaySoundWithContext_D8(u32 r0) {
    FUN_0805186c(r0, *(u32 *)0x030064D8);
}
/*
 * Calls FUN_0805186c with the given parameter and a global sound context
 * pointer from 0x030064DC as the second argument.
 *   r0: first argument passed through to FUN_0805186c
 *   no return value
 */
void PlaySoundWithContext_DC(u32 r0) {
    FUN_0805186c(r0, *(u32 *)0x030064DC);
}
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050344);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0805043c);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080504e0);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050578);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080505cc);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050648);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050684);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080506fc);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080507e0);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050820);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080508e8);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0805099e);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050a94);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050afc);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050c70);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050f4a);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050f70);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08050fd8);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_0805104c);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080510b4);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080510d4);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08051148);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_080511bc);
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08051314);
/*
 * Calls FUN_08051870 with two pass-through arguments and a global
 * sound table pointer from 0x03006450 as the third argument.
 *   r0, r1: passed through to FUN_08051870
 *   no return value
 */
void SoundCommand_6450(u32 r0, u32 r1) {
    FUN_08051870(r0, r1, *(u32 *)0x03006450);
}
INCLUDE_ASM("asm/nonmatchings/m4a", FUN_08051348);
