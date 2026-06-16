.include "asm/macros.inc"
.syntax unified
.text

@ Thumb-typed aliases for alternate entry points inside other functions.
@ Without these, the linker treats the symbols as untyped and emits
@ `__sub_NNNNNNNN_from_thumb` interwork stubs when called from Thumb.
@ Setting the LSB to 1 marks the value as a Thumb entry point.

.thumb
.global sub_08016EEC
.type sub_08016EEC, %function
.set sub_08016EEC, 0x08016EEC | 1

.global sub_0801BB6C
.type sub_0801BB6C, %function
.set sub_0801BB6C, 0x0801BB6C | 1

.global sub_0801BCC0
.type sub_0801BCC0, %function
.set sub_0801BCC0, 0x0801BCC0 | 1

.global sub_0801B044
.type sub_0801B044, %function
.set sub_0801B044, 0x0801B044 | 1

.global sub_0801E1A8
.type sub_0801E1A8, %function
.set sub_0801E1A8, 0x0801E1A8 | 1

.global sub_0801E354
.type sub_0801E354, %function
.set sub_0801E354, 0x0801E354 | 1
