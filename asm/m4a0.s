@ asm/macros.inc is loaded by include_asm.h when m4a0.s is .include'd
@ into src/m4a.c; re-including here would re-define the macros and error.

	.syntax unified

	.text

	thumb_func_start umul3232H32
umul3232H32: @ 0x0804F284
	add r2, pc, #0x0 @ =sub_0804F288
	bx r2

	arm_func_start sub_0804F288
sub_0804F288: @ 0x0804F288
	umull r2, r3, r0, r1
	add r0, r3, #0
	bx lr

	thumb_func_start SoundMain
SoundMain: @ 0x0804F294
	ldr r0, _0804F300 @ =0x03007FF0
	ldr r0, [r0]
	ldr r2, _0804F304 @ =0x68736D53
	ldr r3, [r0]
	cmp r2, r3
	beq _0804F2A2
	bx lr
_0804F2A2:
	adds r3, #1
	str r3, [r0]
	push {r4, r5, r6, r7, lr}
	mov r1, r8
	mov r2, sb
	mov r3, sl
	mov r4, fp
	push {r0, r1, r2, r3, r4}
	sub sp, #0x18
	ldrb r1, [r0, #0xc]
	cmp r1, #0
	beq _0804F2C6
	ldr r2, _0804F30C @ =0x04000006
	ldrb r2, [r2]
	cmp r2, #0xa0
	bhs _0804F2C4
	adds r2, #0xe4
_0804F2C4:
	adds r1, r1, r2
_0804F2C6:
	str r1, [sp, #0x14]
	ldr r3, [r0, #0x20]
	cmp r3, #0
	beq _0804F2D6
	ldr r0, [r0, #0x24]
	bl sub_0804F6B6
	ldr r0, [sp, #0x18]
_0804F2D6:
	ldr r3, [r0, #0x28]
	bl sub_0804F6B6
	ldr r0, [sp, #0x18]
	ldr r3, [r0, #0x10]
	mov r8, r3
	ldr r5, _0804F310 @ =0x00000350
	adds r5, r5, r0
	ldrb r4, [r0, #4]
	subs r7, r4, #1
	bls _0804F2F6
	ldrb r1, [r0, #0xb]
	subs r1, r1, r7
	mov r2, r8
	muls r2, r1, r2
	adds r5, r5, r2
_0804F2F6:
	str r5, [sp, #8]
	ldr r6, _0804F314 @ =0x00000630
	ldr r3, _0804F308 @ =0x03000389
	bx r3
	.align 2, 0
_0804F300: .4byte 0x03007FF0
_0804F304: .4byte 0x68736D53
_0804F308: .4byte 0x03000389
_0804F30C: .4byte 0x04000006
_0804F310: .4byte 0x00000350
_0804F314: .4byte 0x00000630

	thumb_func_start SoundMainRAM
SoundMainRAM: @ 0x0804F318
	ldrb r3, [r0, #5]
	cmp r3, #0
	beq sub_0804F378
	add r1, pc, #0x4 @ =sub_0804F324
	bx r1
	.align 2, 0

	arm_func_start sub_0804F324
sub_0804F324: @ 0x0804F324
	cmp r4, #2
	addeq r7, r0, #0x350
	addne r7, r5, r8
	mov r4, r8
_0804F334:
	ldrsb r0, [r5, r6]
	ldrsb r1, [r5]
	add r0, r0, r1
	ldrsb r1, [r7, r6]
	add r0, r0, r1
	ldrsb r1, [r7], #1
	add r0, r0, r1
	mul r1, r0, r3
	asr r0, r1, #9
	tst r0, #0x80
	addne r0, r0, #1
	strb r0, [r5, r6]
	strb r0, [r5], #1
	subs r4, r4, #1
	bgt _0804F334
	add r0, pc, #0x2F @ =sub_0804F3A6
	bx r0

	thumb_func_start sub_0804F378
sub_0804F378: @ 0x0804F378
	movs r0, #0
	mov r1, r8
	adds r6, r6, r5
	lsrs r1, r1, #3
	blo _0804F386
	stm r5!, {r0}
	stm r6!, {r0}
_0804F386:
	lsrs r1, r1, #1
	blo _0804F392
	stm r5!, {r0}
	stm r6!, {r0}
	stm r5!, {r0}
	stm r6!, {r0}
_0804F392:
	stm r5!, {r0}
	stm r6!, {r0}
	stm r5!, {r0}
	stm r6!, {r0}
	stm r5!, {r0}
	stm r6!, {r0}
	stm r5!, {r0}
	stm r6!, {r0}
	subs r1, #1
	bgt _0804F392

	non_word_aligned_thumb_func_start sub_0804F3A6
sub_0804F3A6: @ 0x0804F3A6
	ldr r4, [sp, #0x18]
	ldr r0, [r4, #0x18]
	mov ip, r0
	ldrb r0, [r4, #6]
	adds r4, #0x50
_0804F3B0:
	str r0, [sp, #4]
	ldr r3, [r4, #0x24]
	ldr r0, [sp, #0x14]
	cmp r0, #0
	beq _0804F3D0
	ldr r1, _0804F3CC @ =0x04000006
	ldrb r1, [r1]
	cmp r1, #0xa0
	bhs _0804F3C4
	adds r1, #0xe4
_0804F3C4:
	cmp r1, r0
	blo _0804F3D0
	b _0804F6A2
	.align 2, 0
_0804F3CC: .4byte 0x04000006
_0804F3D0:
	ldrb r6, [r4]
	movs r0, #0xc7
	tst r0, r6
	bne _0804F3DA
	b sub_0804F698
_0804F3DA:
	movs r0, #0x80
	tst r0, r6
	beq _0804F40A
	movs r0, #0x40
	tst r0, r6
	bne _0804F41A
	movs r6, #3
	strb r6, [r4]
	adds r0, r3, #0
	adds r0, #0x10
	str r0, [r4, #0x28]
	ldr r0, [r3, #0xc]
	str r0, [r4, #0x18]
	movs r5, #0
	strb r5, [r4, #9]
	str r5, [r4, #0x1c]
	ldrb r2, [r3, #3]
	movs r0, #0xc0
	tst r0, r2
	beq _0804F462
	movs r0, #0x10
	orrs r6, r0
	strb r6, [r4]
	b _0804F462
_0804F40A:
	ldrb r5, [r4, #9]
	movs r0, #4
	tst r0, r6
	beq _0804F420
	ldrb r0, [r4, #0xd]
	subs r0, #1
	strb r0, [r4, #0xd]
	bhi _0804F470
_0804F41A:
	movs r0, #0
	strb r0, [r4]
	b sub_0804F698
_0804F420:
	movs r0, #0x40
	tst r0, r6
	beq _0804F440
	ldrb r0, [r4, #7]
	muls r5, r0, r5
	lsrs r5, r5, #8
	ldrb r0, [r4, #0xc]
	cmp r5, r0
	bhi _0804F470
_0804F432:
	ldrb r5, [r4, #0xc]
	cmp r5, #0
	beq _0804F41A
	movs r0, #4
	orrs r6, r0
	strb r6, [r4]
	b _0804F470
_0804F440:
	movs r2, #3
	ands r2, r6
	cmp r2, #2
	bne _0804F45E
	ldrb r0, [r4, #5]
	muls r5, r0, r5
	lsrs r5, r5, #8
	ldrb r0, [r4, #6]
	cmp r5, r0
	bhi _0804F470
	adds r5, r0, #0
	beq _0804F432
	subs r6, #1
	strb r6, [r4]
	b _0804F470
_0804F45E:
	cmp r2, #3
	bne _0804F470
_0804F462:
	ldrb r0, [r4, #4]
	adds r5, r5, r0
	cmp r5, #0xff
	blo _0804F470
	movs r5, #0xff
	subs r6, #1
	strb r6, [r4]
_0804F470:
	strb r5, [r4, #9]
	ldr r0, [sp, #0x18]
	ldrb r0, [r0, #7]
	adds r0, #1
	muls r0, r5, r0
	lsrs r5, r0, #4
	ldrb r0, [r4, #2]
	muls r0, r5, r0
	lsrs r0, r0, #8
	strb r0, [r4, #0xa]
	ldrb r0, [r4, #3]
	muls r0, r5, r0
	lsrs r0, r0, #8
	strb r0, [r4, #0xb]
	movs r0, #0x10
	ands r0, r6
	str r0, [sp, #0x10]
	beq _0804F4A4
	adds r0, r3, #0
	adds r0, #0x10
	ldr r1, [r3, #8]
	adds r0, r0, r1
	str r0, [sp, #0xc]
	ldr r0, [r3, #0xc]
	subs r0, r0, r1
	str r0, [sp, #0x10]
_0804F4A4:
	ldr r5, [sp, #8]
	ldr r2, [r4, #0x18]
	ldr r3, [r4, #0x28]
	add r0, pc, #0x4 @ =sub_0804F4B0
	bx r0
	.align 2, 0

	arm_func_start sub_0804F4B0
sub_0804F4B0: @ 0x0804F4B0
	str r8, [sp]
	ldrb sl, [r4, #0xa]
	ldrb fp, [r4, #0xb]
	lsl sl, sl, #0x10
	lsl fp, fp, #0x10
	ldrb r0, [r4, #1]
	tst r0, #8
	beq _0804F5F0
_0804F4D0:
	cmp r2, #4
	ble _0804F540
	subs r2, r2, r8
	movgt lr, #0
	bgt _0804F4FC
	mov lr, r8
	add r2, r2, r8
	sub r8, r2, #4
	sub lr, lr, r8
	ands r2, r2, #3
	moveq r2, #4
_0804F4FC:
	ldr r6, [r5]
	ldr r7, [r5, #0x630]
_0804F504:
	ldrsb r0, [r3], #1
	mul r1, sl, r0
	bic r1, r1, #0xff0000
	add r6, r1, r6, ror #8
	mul r1, fp, r0
	bic r1, r1, #0xff0000
	add r7, r1, r7, ror #8
	adds r5, r5, #0x40000000
	blo _0804F504
	str r7, [r5, #0x630]
	str r6, [r5], #4
	subs r8, r8, #4
	bgt _0804F4FC
	adds r8, r8, lr
	beq _0804F684
_0804F540:
	ldr r6, [r5]
	ldr r7, [r5, #0x630]
_0804F548:
	ldrsb r0, [r3], #1
	mul r1, sl, r0
	bic r1, r1, #0xff0000
	add r6, r1, r6, ror #8
	mul r1, fp, r0
	bic r1, r1, #0xff0000
	add r7, r1, r7, ror #8
	subs r2, r2, #1
	beq _0804F5B8
_0804F56C:
	adds r5, r5, #0x40000000
	blo _0804F548
	str r7, [r5, #0x630]
	str r6, [r5], #4
	subs r8, r8, #4
	bgt _0804F4D0
	b _0804F684
_0804F588:
	ldr r0, [sp, #0x18]
	cmp r0, #0
	beq _0804F5AC
	ldr r3, [sp, #0x14]
	rsb sb, r2, #0
_0804F59C:
	adds r2, r0, r2
	bgt _0804F654
	sub sb, sb, r0
	b _0804F59C
_0804F5AC:
	pop {r4, ip}
	mov r2, #0
	b _0804F5C8
_0804F5B8:
	ldr r2, [sp, #0x10]
	cmp r2, #0
	ldrne r3, [sp, #0xc]
	bne _0804F56C
_0804F5C8:
	strb r2, [r4]
	lsr r0, r5, #0x1e
	bic r5, r5, #0xc0000000
	rsb r0, r0, #3
	lsl r0, r0, #3
	ror r6, r6, r0
	ror r7, r7, r0
	str r7, [r5, #0x630]
	str r6, [r5], #4
	b _0804F68C
_0804F5F0:
	push {r4, ip}
	ldr lr, [r4, #0x1c]
	ldr r1, [r4, #0x20]
	mul r4, ip, r1
	ldrsb r0, [r3]
	ldrsb r1, [r3, #1]!
	sub r1, r1, r0
_0804F60C:
	ldr r6, [r5]
	ldr r7, [r5, #0x630]
_0804F614:
	mul sb, lr, r1
	add sb, r0, sb, asr #23
	mul ip, sl, sb
	bic ip, ip, #0xff0000
	add r6, ip, r6, ror #8
	mul ip, fp, sb
	bic ip, ip, #0xff0000
	add r7, ip, r7, ror #8
	add lr, lr, r4
	lsrs sb, lr, #0x17
	beq _0804F660
	bic lr, lr, #0x3f800000
	subs r2, r2, sb
	ble _0804F588
	subs sb, sb, #1
	addeq r0, r0, r1
_0804F654:
	ldrsbne r0, [r3, sb]!
	ldrsb r1, [r3, #1]!
	sub r1, r1, r0
_0804F660:
	adds r5, r5, #0x40000000
	blo _0804F614
	str r7, [r5, #0x630]
	str r6, [r5], #4
	subs r8, r8, #4
	bgt _0804F60C
	sub r3, r3, #1
	pop {r4, ip}
	str lr, [r4, #0x1c]
_0804F684:
	str r2, [r4, #0x18]
	str r3, [r4, #0x28]
_0804F68C:
	ldr r8, [sp]
	add r0, pc, #0x1 @ =sub_0804F698
	bx r0

	thumb_func_start sub_0804F698
sub_0804F698: @ 0x0804F698
	ldr r0, [sp, #4]
	subs r0, #1
	ble _0804F6A2
	adds r4, #0x40
	b _0804F3B0
_0804F6A2:
	ldr r0, [sp, #0x18]
	ldr r3, _0804F6B8 @ =0x68736D53
	str r3, [r0]
	add sp, #0x1c
	pop {r0, r1, r2, r3, r4, r5, r6, r7}
	mov r8, r0
	mov sb, r1
	mov sl, r2
	mov fp, r3
	pop {r3}

	non_word_aligned_thumb_func_start sub_0804F6B6
sub_0804F6B6: @ 0x0804F6B6
	bx r3
	.align 2, 0
_0804F6B8: .4byte 0x68736D53

	thumb_func_start SoundMainBTM
SoundMainBTM: @ 0x0804F6BC
	mov ip, r4
	movs r1, #0
	movs r2, #0
	movs r3, #0
	movs r4, #0
	stm r0!, {r1, r2, r3, r4}
	stm r0!, {r1, r2, r3, r4}
	stm r0!, {r1, r2, r3, r4}
	stm r0!, {r1, r2, r3, r4}
	mov r4, ip
	bx lr
	.align 2, 0

	thumb_func_start MP2KClearChain
MP2KClearChain: @ 0x0804F6D4
	ldr r3, [r0, #0x2c]
	cmp r3, #0
	beq _0804F6F2
	ldr r1, [r0, #0x34]
	ldr r2, [r0, #0x30]
	cmp r2, #0
	beq _0804F6E6
	str r1, [r2, #0x34]
	b _0804F6E8
_0804F6E6:
	str r1, [r3, #0x20]
_0804F6E8:
	cmp r1, #0
	beq _0804F6EE
	str r2, [r1, #0x30]
_0804F6EE:
	movs r1, #0
	str r1, [r0, #0x2c]
_0804F6F2:
	bx lr

	thumb_func_start MP2K_event_fine
MP2K_event_fine: @ 0x0804F6F4
	push {r4, r5, lr}
	adds r5, r1, #0
	ldr r4, [r5, #0x20]
	cmp r4, #0
	beq _0804F718
_0804F6FE:
	ldrb r1, [r4]
	movs r0, #0xc7
	tst r0, r1
	beq _0804F70C
	movs r0, #0x40
	orrs r1, r0
	strb r1, [r4]
_0804F70C:
	adds r0, r4, #0
	bl MP2KClearChain
	ldr r4, [r4, #0x34]
	cmp r4, #0
	bne _0804F6FE
_0804F718:
	movs r0, #0
	strb r0, [r5]
	pop {r4, r5}
	pop {r0}
	bx r0
	.align 2, 0

	thumb_func_start MPlayJumpTableCopy
MPlayJumpTableCopy: @ 0x0804F724
	mov ip, lr
	movs r1, #0x24
	ldr r2, _0804F754 @ =gMPlayJumpTableTemplate
_0804F72A:
	ldr r3, [r2]
	bl _0804F73E
	stm r0!, {r3}
	adds r2, #4
	subs r1, #1
	bgt _0804F72A
	bx ip
	.align 2, 0

	thumb_func_start sub_0804F73C
sub_0804F73C: @ 0x0804F73C
	ldrb r3, [r2]
_0804F73E:
	push {r0}
	lsrs r0, r2, #0x19
	bne _0804F750
	ldr r0, _0804F754 @ =gMPlayJumpTableTemplate
	cmp r2, r0
	blo _0804F74E
	lsrs r0, r2, #0xe
	beq _0804F750
_0804F74E:
	movs r3, #0
_0804F750:
	pop {r0}
	bx lr
	.align 2, 0
_0804F754: .4byte gMPlayJumpTableTemplate

	thumb_func_start sub_0804F758
sub_0804F758: @ 0x0804F758
	ldr r2, [r1, #0x40]

	non_word_aligned_thumb_func_start sub_0804F75A
sub_0804F75A: @ 0x0804F75A
	adds r3, r2, #1
	str r3, [r1, #0x40]
	ldrb r3, [r2]
	b _0804F73E
	.align 2, 0

	thumb_func_start MP2K_event_goto
MP2K_event_goto: @ 0x0804F764
	push {lr}
_0804F766:
	ldr r2, [r1, #0x40]
	ldrb r0, [r2, #3]
	lsls r0, r0, #8
	ldrb r3, [r2, #2]
	orrs r0, r3
	lsls r0, r0, #8
	ldrb r3, [r2, #1]
	orrs r0, r3
	lsls r0, r0, #8
	bl sub_0804F73C
	orrs r0, r3
	str r0, [r1, #0x40]
	pop {r0}
	bx r0

	thumb_func_start MP2K_event_patt
MP2K_event_patt: @ 0x0804F784
	ldrb r2, [r1, #2]
	cmp r2, #3
	bhs _0804F79C
	lsls r2, r2, #2
	adds r3, r1, r2
	ldr r2, [r1, #0x40]
	adds r2, #4
	str r2, [r3, #0x44]
	ldrb r2, [r1, #2]
	adds r2, #1
	strb r2, [r1, #2]
	b MP2K_event_goto
_0804F79C:
	b MP2K_event_fine
	.align 2, 0

	thumb_func_start MP2K_event_pend
MP2K_event_pend: @ 0x0804F7A0
	ldrb r2, [r1, #2]
	cmp r2, #0
	beq _0804F7B2
	subs r2, #1
	strb r2, [r1, #2]
	lsls r2, r2, #2
	adds r3, r1, r2
	ldr r2, [r3, #0x44]
	str r2, [r1, #0x40]
_0804F7B2:
	bx lr

	thumb_func_start MP2K_event_rept
MP2K_event_rept: @ 0x0804F7B4
	push {lr}
	ldr r2, [r1, #0x40]
	ldrb r3, [r2]
	cmp r3, #0
	bne _0804F7C4
	adds r2, #1
	str r2, [r1, #0x40]
	b _0804F766
_0804F7C4:
	ldrb r3, [r1, #3]
	adds r3, #1
	strb r3, [r1, #3]
	mov ip, r3
	bl sub_0804F758
	cmp ip, r3
	bhs _0804F7D6
	b _0804F766
_0804F7D6:
	movs r3, #0
	strb r3, [r1, #3]
	adds r2, #5
	str r2, [r1, #0x40]
	pop {r0}
	bx r0
	.align 2, 0

	thumb_func_start MP2K_event_prio
MP2K_event_prio: @ 0x0804F7E4
	mov ip, lr
	bl sub_0804F758
	strb r3, [r1, #0x1d]
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_tempo
MP2K_event_tempo: @ 0x0804F7F0
	mov ip, lr
	bl sub_0804F758
	lsls r3, r3, #1
	strh r3, [r0, #0x1c]
	ldrh r2, [r0, #0x1e]
	muls r3, r2, r3
	lsrs r3, r3, #8
	strh r3, [r0, #0x20]
	bx ip

	thumb_func_start MP2K_event_keysh
MP2K_event_keysh: @ 0x0804F804
	mov ip, lr
	bl sub_0804F758
	strb r3, [r1, #0xa]
	ldrb r3, [r1]
	movs r2, #0xc
	orrs r3, r2
	strb r3, [r1]
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_voice
MP2K_event_voice: @ 0x0804F818
	mov ip, lr
	ldr r2, [r1, #0x40]
	ldrb r3, [r2]
	adds r2, #1
	str r2, [r1, #0x40]
	lsls r2, r3, #1
	adds r2, r2, r3
	lsls r2, r2, #2
	ldr r3, [r0, #0x30]
	adds r2, r2, r3
	ldr r3, [r2]
	bl _0804F73E
	str r3, [r1, #0x24]
	ldr r3, [r2, #4]
	bl _0804F73E
	str r3, [r1, #0x28]
	ldr r3, [r2, #8]
	bl _0804F73E
	str r3, [r1, #0x2c]
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_vol
MP2K_event_vol: @ 0x0804F848
	mov ip, lr
	bl sub_0804F758
	strb r3, [r1, #0x12]
	ldrb r3, [r1]
	movs r2, #3
	orrs r3, r2
	strb r3, [r1]
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_pan
MP2K_event_pan: @ 0x0804F85C
	mov ip, lr
	bl sub_0804F758
	subs r3, #0x40
	strb r3, [r1, #0x14]
	ldrb r3, [r1]
	movs r2, #3
	orrs r3, r2
	strb r3, [r1]
	bx ip

	thumb_func_start MP2K_event_bend
MP2K_event_bend: @ 0x0804F870
	mov ip, lr
	bl sub_0804F758
	subs r3, #0x40
	strb r3, [r1, #0xe]
	ldrb r3, [r1]
	movs r2, #0xc
	orrs r3, r2
	strb r3, [r1]
	bx ip

	thumb_func_start MP2K_event_bendr
MP2K_event_bendr: @ 0x0804F884
	mov ip, lr
	bl sub_0804F758
	strb r3, [r1, #0xf]
	ldrb r3, [r1]
	movs r2, #0xc
	orrs r3, r2
	strb r3, [r1]
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_lfodl
MP2K_event_lfodl: @ 0x0804F898
	mov ip, lr
	bl sub_0804F758
	strb r3, [r1, #0x1b]
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_modt
MP2K_event_modt: @ 0x0804F8A4
	mov ip, lr
	bl sub_0804F758
	ldrb r0, [r1, #0x18]
	cmp r0, r3
	beq _0804F8BA
	strb r3, [r1, #0x18]
	ldrb r3, [r1]
	movs r2, #0xf
	orrs r3, r2
	strb r3, [r1]
_0804F8BA:
	bx ip

	thumb_func_start MP2K_event_tune
MP2K_event_tune: @ 0x0804F8BC
	mov ip, lr
	bl sub_0804F758
	subs r3, #0x40
	strb r3, [r1, #0xc]
	ldrb r3, [r1]
	movs r2, #0xc
	orrs r3, r2
	strb r3, [r1]
	bx ip

	thumb_func_start MP2K_event_port
MP2K_event_port: @ 0x0804F8D0
	mov ip, lr
	ldr r2, [r1, #0x40]
	ldrb r3, [r2]
	adds r2, #1
	ldr r0, _0804F8E4 @ =0x04000060
	adds r0, r0, r3
	bl sub_0804F75A
	strb r3, [r0]
	bx ip
	.align 2, 0
_0804F8E4: .4byte 0x04000060

	thumb_func_start m4aSoundVSync
m4aSoundVSync: @ 0x0804F8E8
	ldr r0, _0804FB94 @ =0x03007FF0
	ldr r0, [r0]
	ldr r2, _0804FB98 @ =0x68736D53
	ldr r3, [r0]
	subs r3, r3, r2
	cmp r3, #1
	bhi _0804F928
	ldrb r1, [r0, #4]
	subs r1, #1
	strb r1, [r0, #4]
	bgt _0804F928
	ldrb r1, [r0, #0xb]
	strb r1, [r0, #4]
	ldr r2, _0804F92C @ =0x040000BC
	ldr r1, [r2, #8]
	lsls r1, r1, #7
	blo _0804F90E
	ldr r1, _0804F930 @ =0x84400004
	str r1, [r2, #8]
_0804F90E:
	ldr r1, [r2, #0x14]
	lsls r1, r1, #7
	blo _0804F918
	ldr r1, _0804F930 @ =0x84400004
	str r1, [r2, #0x14]
_0804F918:
	movs r1, #4
	lsls r1, r1, #8
	strh r1, [r2, #0xa]
	strh r1, [r2, #0x16]
	movs r1, #0xb6
	lsls r1, r1, #8
	strh r1, [r2, #0xa]
	strh r1, [r2, #0x16]
_0804F928:
	bx lr
	.align 2, 0
_0804F92C: .4byte 0x040000BC
_0804F930: .4byte 0x84400004

	thumb_func_start MP2KPlayerMain
MP2KPlayerMain: @ 0x0804F934
	ldr r2, _0804FB98 @ =0x68736D53
	ldr r3, [r0, #0x34]
	cmp r2, r3
	beq _0804F93E
	bx lr
_0804F93E:
	adds r3, #1
	str r3, [r0, #0x34]
	push {r0, lr}
	ldr r3, [r0, #0x38]
	cmp r3, #0
	beq _0804F950
	ldr r0, [r0, #0x3c]
	bl call_r3
_0804F950:
	pop {r0}
	push {r4, r5, r6, r7}
	mov r4, r8
	mov r5, sb
	mov r6, sl
	mov r7, fp
	push {r4, r5, r6, r7}
	adds r7, r0, #0
	ldr r0, [r7, #4]
	cmp r0, #0
	bge _0804F968
	b _0804FB7C
_0804F968:
	ldr r0, _0804FB94 @ =0x03007FF0
	ldr r0, [r0]
	mov r8, r0
	adds r0, r7, #0
	bl FadeOutBody
	ldr r0, [r7, #4]
	cmp r0, #0
	bge _0804F97C
	b _0804FB7C
_0804F97C:
	ldrh r0, [r7, #0x22]
	ldrh r1, [r7, #0x20]
	adds r0, r0, r1
	b _0804FACC
_0804F984:
	ldrb r6, [r7, #8]
	ldr r5, [r7, #0x2c]
	movs r3, #1
	movs r4, #0
_0804F98C:
	ldrb r0, [r5]
	movs r1, #0x80
	tst r1, r0
	bne _0804F996
	b _0804FAA8
_0804F996:
	mov sl, r3
	orrs r4, r3
	mov fp, r4
	ldr r4, [r5, #0x20]
	cmp r4, #0
	beq _0804F9CA
_0804F9A2:
	ldrb r1, [r4]
	movs r0, #0xc7
	tst r0, r1
	beq _0804F9BE
	ldrb r0, [r4, #0x10]
	cmp r0, #0
	beq _0804F9C4
	subs r0, #1
	strb r0, [r4, #0x10]
	bne _0804F9C4
	movs r0, #0x40
	orrs r1, r0
	strb r1, [r4]
	b _0804F9C4
_0804F9BE:
	adds r0, r4, #0
	bl ClearChain
_0804F9C4:
	ldr r4, [r4, #0x34]
	cmp r4, #0
	bne _0804F9A2
_0804F9CA:
	ldrb r3, [r5]
	movs r0, #0x40
	tst r0, r3
	beq _0804FA48
	adds r0, r5, #0
	bl Clear64byte
	movs r0, #0x80
	strb r0, [r5]
	movs r0, #2
	strb r0, [r5, #0xf]
	movs r0, #0x40
	strb r0, [r5, #0x13]
	movs r0, #0x16
	strb r0, [r5, #0x19]
	movs r0, #1
	adds r1, r5, #6
	strb r0, [r1, #0x1e]
	b _0804FA48
_0804F9F0:
	ldr r2, [r5, #0x40]
	ldrb r1, [r2]
	cmp r1, #0x80
	bhs _0804F9FC
	ldrb r1, [r5, #7]
	b _0804FA06
_0804F9FC:
	adds r2, #1
	str r2, [r5, #0x40]
	cmp r1, #0xbd
	blo _0804FA06
	strb r1, [r5, #7]
_0804FA06:
	cmp r1, #0xcf
	blo _0804FA1C
	mov r0, r8
	ldr r3, [r0, #0x38]
	adds r0, r1, #0
	subs r0, #0xcf
	adds r1, r7, #0
	adds r2, r5, #0
	bl call_r3
	b _0804FA48
_0804FA1C:
	cmp r1, #0xb0
	bls _0804FA3E
	adds r0, r1, #0
	subs r0, #0xb1
	strb r0, [r7, #0xa]
	mov r3, r8
	ldr r3, [r3, #0x34]
	lsls r0, r0, #2
	ldr r3, [r3, r0]
	adds r0, r7, #0
	adds r1, r5, #0
	bl call_r3
	ldrb r0, [r5]
	cmp r0, #0
	beq _0804FAA4
	b _0804FA48
_0804FA3E:
	ldr r0, _0804FB90 @ =gClockTable
	subs r1, #0x80
	adds r1, r1, r0
	ldrb r0, [r1]
	strb r0, [r5, #1]
_0804FA48:
	ldrb r0, [r5, #1]
	cmp r0, #0
	beq _0804F9F0
	subs r0, #1
	strb r0, [r5, #1]
	ldrb r1, [r5, #0x19]
	cmp r1, #0
	beq _0804FAA4
	ldrb r0, [r5, #0x17]
	cmp r0, #0
	beq _0804FAA4
	ldrb r0, [r5, #0x1c]
	cmp r0, #0
	beq _0804FA6A
	subs r0, #1
	strb r0, [r5, #0x1c]
	b _0804FAA4
_0804FA6A:
	ldrb r0, [r5, #0x1a]
	adds r0, r0, r1
	strb r0, [r5, #0x1a]
	adds r1, r0, #0
	subs r0, #0x40
	lsls r0, r0, #0x18
	bpl _0804FA7E
	lsls r2, r1, #0x18
	asrs r2, r2, #0x18
	b _0804FA82
_0804FA7E:
	movs r0, #0x80
	subs r2, r0, r1
_0804FA82:
	ldrb r0, [r5, #0x17]
	muls r0, r2, r0
	asrs r2, r0, #6
	ldrb r0, [r5, #0x16]
	eors r0, r2
	lsls r0, r0, #0x18
	beq _0804FAA4
	strb r2, [r5, #0x16]
	ldrb r0, [r5]
	ldrb r1, [r5, #0x18]
	cmp r1, #0
	bne _0804FA9E
	movs r1, #0xc
	b _0804FAA0
_0804FA9E:
	movs r1, #3
_0804FAA0:
	orrs r0, r1
	strb r0, [r5]
_0804FAA4:
	mov r3, sl
	mov r4, fp
_0804FAA8:
	subs r6, #1
	ble _0804FAB4
	movs r0, #0x50
	adds r5, r5, r0
	lsls r3, r3, #1
	b _0804F98C
_0804FAB4:
	ldr r0, [r7, #0xc]
	adds r0, #1
	str r0, [r7, #0xc]
	cmp r4, #0
	bne _0804FAC6
	movs r0, #0x80
	lsls r0, r0, #0x18
	str r0, [r7, #4]
	b _0804FB7C
_0804FAC6:
	str r4, [r7, #4]
	ldrh r0, [r7, #0x22]
	subs r0, #0x96
_0804FACC:
	strh r0, [r7, #0x22]
	cmp r0, #0x96
	blo _0804FAD4
	b _0804F984
_0804FAD4:
	ldrb r2, [r7, #8]
	ldr r5, [r7, #0x2c]
_0804FAD8:
	ldrb r0, [r5]
	movs r1, #0x80
	tst r1, r0
	beq _0804FB72
	movs r1, #0xf
	tst r1, r0
	beq _0804FB72
	mov sb, r2
	adds r0, r7, #0
	adds r1, r5, #0
	bl TrkVolPitSet
	ldr r4, [r5, #0x20]
	cmp r4, #0
	beq _0804FB68
_0804FAF6:
	ldrb r1, [r4]
	movs r0, #0xc7
	tst r0, r1
	bne _0804FB06
	adds r0, r4, #0
	bl ClearChain
	b _0804FB62
_0804FB06:
	ldrb r0, [r4, #1]
	movs r6, #7
	ands r6, r0
	ldrb r3, [r5]
	movs r0, #3
	tst r0, r3
	beq _0804FB24
	bl ChnVolSetAsm
	cmp r6, #0
	beq _0804FB24
	ldrb r0, [r4, #0x1d]
	movs r1, #1
	orrs r0, r1
	strb r0, [r4, #0x1d]
_0804FB24:
	ldrb r3, [r5]
	movs r0, #0xc
	tst r0, r3
	beq _0804FB62
	ldrb r1, [r4, #8]
	movs r0, #8
	ldrsb r0, [r5, r0]
	adds r2, r1, r0
	bpl _0804FB38
	movs r2, #0
_0804FB38:
	cmp r6, #0
	beq _0804FB56
	mov r0, r8
	ldr r3, [r0, #0x30]
	adds r1, r2, #0
	ldrb r2, [r5, #9]
	adds r0, r6, #0
	bl call_r3
	str r0, [r4, #0x20]
	ldrb r0, [r4, #0x1d]
	movs r1, #2
	orrs r0, r1
	strb r0, [r4, #0x1d]
	b _0804FB62
_0804FB56:
	adds r1, r2, #0
	ldrb r2, [r5, #9]
	ldr r0, [r4, #0x24]
	bl MidiKeyToFreq
	str r0, [r4, #0x20]
_0804FB62:
	ldr r4, [r4, #0x34]
	cmp r4, #0
	bne _0804FAF6
_0804FB68:
	ldrb r0, [r5]
	movs r1, #0xf0
	ands r0, r1
	strb r0, [r5]
	mov r2, sb
_0804FB72:
	subs r2, #1
	ble _0804FB7C
	movs r0, #0x50
	adds r5, r5, r0
	bgt _0804FAD8
_0804FB7C:
	ldr r0, _0804FB98 @ =0x68736D53
	str r0, [r7, #0x34]
	pop {r0, r1, r2, r3, r4, r5, r6, r7}
	mov r8, r0
	mov sb, r1
	mov sl, r2
	mov fp, r3
	pop {r3}

	thumb_func_start call_r3
call_r3: @ 0x0804FB8C
	bx r3
	.align 2, 0
_0804FB90: .4byte gClockTable
_0804FB94: .4byte 0x03007FF0
_0804FB98: .4byte 0x68736D53

	thumb_func_start TrackStop
TrackStop: @ 0x0804FB9C
	push {r4, r5, r6, lr}
	adds r5, r1, #0
	ldrb r1, [r5]
	movs r0, #0x80
	tst r0, r1
	beq _0804FBD4
	ldr r4, [r5, #0x20]
	cmp r4, #0
	beq _0804FBD2
	movs r6, #0
_0804FBB0:
	ldrb r0, [r4]
	cmp r0, #0
	beq _0804FBCA
	ldrb r0, [r4, #1]
	movs r3, #7
	ands r0, r3
	beq _0804FBC8
	ldr r3, _0804FBDC @ =0x03007FF0
	ldr r3, [r3]
	ldr r3, [r3, #0x2c]
	bl call_r3
_0804FBC8:
	strb r6, [r4]
_0804FBCA:
	str r6, [r4, #0x2c]
	ldr r4, [r4, #0x34]
	cmp r4, #0
	bne _0804FBB0
_0804FBD2:
	str r4, [r5, #0x20]
_0804FBD4:
	pop {r4, r5, r6}
	pop {r0}
	bx r0
	.align 2, 0
_0804FBDC: .4byte 0x03007FF0

	thumb_func_start ChnVolSetAsm
ChnVolSetAsm: @ 0x0804FBE0
	ldrb r1, [r4, #0x12]
	movs r0, #0x14
	ldrsb r2, [r4, r0]
	movs r3, #0x80
	adds r3, r3, r2
	muls r3, r1, r3
	ldrb r0, [r5, #0x10]
	muls r0, r3, r0
	asrs r0, r0, #0xe
	cmp r0, #0xff
	bls _0804FBF8
	movs r0, #0xff
_0804FBF8:
	strb r0, [r4, #2]
	movs r3, #0x7f
	subs r3, r3, r2
	muls r3, r1, r3
	ldrb r0, [r5, #0x11]
	muls r0, r3, r0
	asrs r0, r0, #0xe
	cmp r0, #0xff
	bls _0804FC0C
	movs r0, #0xff
_0804FC0C:
	strb r0, [r4, #3]
	bx lr

	thumb_func_start MP2K_event_nxx
MP2K_event_nxx: @ 0x0804FC10
	push {r4, r5, r6, r7, lr}
	mov r4, r8
	mov r5, sb
	mov r6, sl
	mov r7, fp
	push {r4, r5, r6, r7}
	sub sp, #0x18
	str r1, [sp]
	adds r5, r2, #0
	ldr r1, _0804FE08 @ =0x03007FF0
	ldr r1, [r1]
	str r1, [sp, #4]
	ldr r1, _0804FE0C @ =gClockTable
	adds r0, r0, r1
	ldrb r0, [r0]
	strb r0, [r5, #4]
	ldr r3, [r5, #0x40]
	ldrb r0, [r3]
	cmp r0, #0x80
	bhs _0804FC56
	strb r0, [r5, #5]
	adds r3, #1
	ldrb r0, [r3]
	cmp r0, #0x80
	bhs _0804FC54
	strb r0, [r5, #6]
	adds r3, #1
	ldrb r0, [r3]
	cmp r0, #0x80
	bhs _0804FC54
	ldrb r1, [r5, #4]
	adds r1, r1, r0
	strb r1, [r5, #4]
	adds r3, #1
_0804FC54:
	str r3, [r5, #0x40]
_0804FC56:
	movs r0, #0
	str r0, [sp, #0x14]
	adds r4, r5, #0
	adds r4, #0x24
	ldrb r2, [r4]
	movs r0, #0xc0
	tst r0, r2
	beq _0804FCA8
	ldrb r3, [r5, #5]
	movs r0, #0x40
	tst r0, r2
	beq _0804FC76
	ldr r1, [r5, #0x2c]
	adds r1, r1, r3
	ldrb r0, [r1]
	b _0804FC78
_0804FC76:
	adds r0, r3, #0
_0804FC78:
	lsls r1, r0, #1
	adds r1, r1, r0
	lsls r1, r1, #2
	ldr r0, [r5, #0x28]
	adds r1, r1, r0
	mov sb, r1
	mov r6, sb
	ldrb r1, [r6]
	movs r0, #0xc0
	tst r0, r1
	beq _0804FC90
	b _0804FDF6
_0804FC90:
	movs r0, #0x80
	tst r0, r2
	beq _0804FCAC
	ldrb r1, [r6, #3]
	movs r0, #0x80
	tst r0, r1
	beq _0804FCA4
	subs r1, #0xc0
	lsls r1, r1, #1
	str r1, [sp, #0x14]
_0804FCA4:
	ldrb r3, [r6, #1]
	b _0804FCAC
_0804FCA8:
	mov sb, r4
	ldrb r3, [r5, #5]
_0804FCAC:
	str r3, [sp, #8]
	ldr r6, [sp]
	ldrb r1, [r6, #9]
	ldrb r0, [r5, #0x1d]
	adds r0, r0, r1
	cmp r0, #0xff
	bls _0804FCBC
	movs r0, #0xff
_0804FCBC:
	str r0, [sp, #0x10]
	mov r6, sb
	ldrb r0, [r6]
	movs r6, #7
	ands r6, r0
	str r6, [sp, #0xc]
	beq _0804FCFC
	ldr r0, [sp, #4]
	ldr r4, [r0, #0x1c]
	cmp r4, #0
	bne _0804FCD4
	b _0804FDF6
_0804FCD4:
	subs r6, #1
	lsls r0, r6, #6
	adds r4, r4, r0
	ldrb r1, [r4]
	movs r0, #0xc7
	tst r0, r1
	beq _0804FD50
	movs r0, #0x40
	tst r0, r1
	bne _0804FD50
	ldrb r1, [r4, #0x13]
	ldr r0, [sp, #0x10]
	cmp r1, r0
	blo _0804FD50
	beq _0804FCF4
	b _0804FDF6
_0804FCF4:
	ldr r0, [r4, #0x2c]
	cmp r0, r5
	bhs _0804FD50
	b _0804FDF6
_0804FCFC:
	ldr r6, [sp, #0x10]
	adds r7, r5, #0
	movs r2, #0
	mov r8, r2
	ldr r4, [sp, #4]
	ldrb r3, [r4, #6]
	adds r4, #0x50
_0804FD0A:
	ldrb r1, [r4]
	movs r0, #0xc7
	tst r0, r1
	beq _0804FD50
	movs r0, #0x40
	tst r0, r1
	beq _0804FD24
	cmp r2, #0
	bne _0804FD28
	adds r2, #1
	ldrb r6, [r4, #0x13]
	ldr r7, [r4, #0x2c]
	b _0804FD42
_0804FD24:
	cmp r2, #0
	bne _0804FD44
_0804FD28:
	ldrb r0, [r4, #0x13]
	cmp r0, r6
	bhs _0804FD34
	adds r6, r0, #0
	ldr r7, [r4, #0x2c]
	b _0804FD42
_0804FD34:
	bhi _0804FD44
	ldr r0, [r4, #0x2c]
	cmp r0, r7
	bls _0804FD40
	adds r7, r0, #0
	b _0804FD42
_0804FD40:
	blo _0804FD44
_0804FD42:
	mov r8, r4
_0804FD44:
	adds r4, #0x40
	subs r3, #1
	bgt _0804FD0A
	mov r4, r8
	cmp r4, #0
	beq _0804FDF6
_0804FD50:
	adds r0, r4, #0
	bl ClearChain
	movs r1, #0
	str r1, [r4, #0x30]
	ldr r3, [r5, #0x20]
	str r3, [r4, #0x34]
	cmp r3, #0
	beq _0804FD64
	str r4, [r3, #0x30]
_0804FD64:
	str r4, [r5, #0x20]
	str r5, [r4, #0x2c]
	ldrb r0, [r5, #0x1b]
	strb r0, [r5, #0x1c]
	cmp r0, r1
	beq _0804FD76
	adds r1, r5, #0
	bl clear_modM
_0804FD76:
	ldr r0, [sp]
	adds r1, r5, #0
	bl TrkVolPitSet
	ldr r0, [r5, #4]
	str r0, [r4, #0x10]
	ldr r0, [sp, #0x10]
	strb r0, [r4, #0x13]
	ldr r0, [sp, #8]
	strb r0, [r4, #8]
	ldr r0, [sp, #0x14]
	strb r0, [r4, #0x14]
	mov r6, sb
	ldrb r0, [r6]
	strb r0, [r4, #1]
	ldr r7, [r6, #4]
	str r7, [r4, #0x24]
	ldr r0, [r6, #8]
	str r0, [r4, #4]
	ldrh r0, [r5, #0x1e]
	strh r0, [r4, #0xc]
	bl ChnVolSetAsm
	ldrb r1, [r4, #8]
	movs r0, #8
	ldrsb r0, [r5, r0]
	adds r3, r1, r0
	bpl _0804FDB0
	movs r3, #0
_0804FDB0:
	ldr r6, [sp, #0xc]
	cmp r6, #0
	beq _0804FDDE
	mov r6, sb
	ldrb r0, [r6, #2]
	strb r0, [r4, #0x1e]
	ldrb r1, [r6, #3]
	movs r0, #0x80
	tst r0, r1
	bne _0804FDCA
	movs r0, #0x70
	tst r0, r1
	bne _0804FDCC
_0804FDCA:
	movs r1, #8
_0804FDCC:
	strb r1, [r4, #0x1f]
	ldrb r2, [r5, #9]
	adds r1, r3, #0
	ldr r0, [sp, #0xc]
	ldr r3, [sp, #4]
	ldr r3, [r3, #0x30]
	bl call_r3
	b _0804FDE8
_0804FDDE:
	ldrb r2, [r5, #9]
	adds r1, r3, #0
	adds r0, r7, #0
	bl MidiKeyToFreq
_0804FDE8:
	str r0, [r4, #0x20]
	movs r0, #0x80
	strb r0, [r4]
	ldrb r1, [r5]
	movs r0, #0xf0
	ands r0, r1
	strb r0, [r5]
_0804FDF6:
	add sp, #0x18
	pop {r0, r1, r2, r3, r4, r5, r6, r7}
	mov r8, r0
	mov sb, r1
	mov sl, r2
	mov fp, r3
	pop {r0}
	bx r0
	.align 2, 0
_0804FE08: .4byte 0x03007FF0
_0804FE0C: .4byte gClockTable

	thumb_func_start MP2K_event_endtie
MP2K_event_endtie: @ 0x0804FE10
	push {r4, r5}
	ldr r2, [r1, #0x40]
	ldrb r3, [r2]
	cmp r3, #0x80
	bhs _0804FE22
	strb r3, [r1, #5]
	adds r2, #1
	str r2, [r1, #0x40]
	b _0804FE24
_0804FE22:
	ldrb r3, [r1, #5]
_0804FE24:
	ldr r1, [r1, #0x20]
	cmp r1, #0
	beq _0804FE4C
	movs r4, #0x83
	movs r5, #0x40
_0804FE2E:
	ldrb r2, [r1]
	tst r2, r4
	beq _0804FE46
	tst r2, r5
	bne _0804FE46
	ldrb r0, [r1, #0x11]
	cmp r0, r3
	bne _0804FE46
	movs r0, #0x40
	orrs r2, r0
	strb r2, [r1]
	b _0804FE4C
_0804FE46:
	ldr r1, [r1, #0x34]
	cmp r1, #0
	bne _0804FE2E
_0804FE4C:
	pop {r4, r5}
	bx lr

	thumb_func_start clear_modM
clear_modM: @ 0x0804FE50
	movs r2, #0
	strb r2, [r1, #0x16]
	strb r2, [r1, #0x1a]
	ldrb r2, [r1, #0x18]
	cmp r2, #0
	bne _0804FE60
	movs r2, #0xc
	b _0804FE62
_0804FE60:
	movs r2, #3
_0804FE62:
	ldrb r3, [r1]
	orrs r3, r2
	strb r3, [r1]
	bx lr
	.align 2, 0

	thumb_func_start sub_0804FE6C
sub_0804FE6C: @ 0x0804FE6C
	ldr r2, [r1, #0x40]
	adds r3, r2, #1
	str r3, [r1, #0x40]
	ldrb r3, [r2]
	bx lr
	.align 2, 0

	thumb_func_start MP2K_event_lfos
MP2K_event_lfos: @ 0x0804FE78
	mov ip, lr
	bl sub_0804FE6C
	strb r3, [r1, #0x19]
	cmp r3, #0
	bne _0804FE88
	bl clear_modM
_0804FE88:
	bx ip
	.align 2, 0

	thumb_func_start MP2K_event_mod
MP2K_event_mod: @ 0x0804FE8C
	mov ip, lr
	bl sub_0804FE6C
	strb r3, [r1, #0x17]
	cmp r3, #0
	bne _0804FE9C
	bl clear_modM
_0804FE9C:
	bx ip
	.align 2, 0
