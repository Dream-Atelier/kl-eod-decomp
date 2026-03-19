	thumb_func_start m4aSoundVSyncOff
m4aSoundVSyncOff: @ 080505CC
	push {lr}
	add sp, #-0x004
	ldr r0, _0805062C @ =0x03007FF0
	ldr r2, [r0, #0x00]
	ldr r1, [r2, #0x00]
	ldr r3, _08050630 @ =0x978C92AD
	adds r0, r1, r3
	cmp r0, #0x01
	bhi _08050624
	adds r0, r1, #0x0
	adds r0, #0x0A
	str r0, [r2, #0x00]
	ldr r1, _08050634 @ =0x040000C4
	ldr r0, [r1, #0x00]
	movs r3, #0x80
	lsls r3, r3, #0x12
	ands r0, r3
	cmp r0, #0x00
	beq _080505F6
	ldr r0, _08050638 @ =0x84400004
	str r0, [r1, #0x00]
_080505F6:
	ldr r1, _0805063C @ =0x040000D0
	ldr r0, [r1, #0x00]
	ands r0, r3
	cmp r0, #0x00
	beq _08050604
	ldr r0, _08050638 @ =0x84400004
	str r0, [r1, #0x00]
_08050604:
	ldr r0, _08050640 @ =0x040000C6
	movs r3, #0x80
	lsls r3, r3, #0x03
	adds r1, r3, #0x0
	strh r1, [r0, #0x00]
	adds r0, #0x0C
	strh r1, [r0, #0x00]
	movs r0, #0x00
	str r0, [sp, #0x000]
	movs r0, #0xD4
	lsls r0, r0, #0x02
	adds r1, r2, r0
	ldr r2, _08050644 @ =0x05000318
	mov r0, sp
	bl BitUnPack
_08050624:
	add sp, #0x004
	pop {r0}
	bx r0
	lsls r0, r0, #0x00
_0805062C: .4byte 0x03007FF0
_08050630: .4byte 0x978C92AD
_08050634: .4byte 0x040000C4
_08050638: .4byte 0x84400004
_0805063C: .4byte 0x040000D0
_08050640: .4byte 0x040000C6
_08050644: .4byte 0x05000318
