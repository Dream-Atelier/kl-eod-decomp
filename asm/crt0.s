.include "asm/macros.inc"
.include "constants/gba_constants.inc"
.include "constants/game_constants.inc"

.syntax unified
.arm

	.global start_vector
start_vector: @ 080000C0
	mov r0, #PSR_IRQ_MODE
	msr cpsr_fc, r0
	ldr sp, _sp_irq
	mov r0, #PSR_SYS_MODE
	msr cpsr_fc, r0
	ldr sp, _sp_sys
	ldr r1, _pool_IntrVector
	adr r0, IntrMain
	str r0, [r1]
	ldr r1, _pool_AgbMain
	mov lr, pc
	bx r1
	b start_vector

_sp_sys:  .word IWRAM_END - 0x300
_sp_irq:  .word IWRAM_END - 0x60

	arm_func_start IntrMain
IntrMain: @ 080000FC
	mov r3, #REG_BASE
	add r3, r3, #OFFSET_REG_IE
	ldr r2, [r3]
	and r1, r2, r2, lsr #16
	mov r2, #0
	ands r0, r1, #INTR_FLAG_VBLANK
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_HBLANK
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_VCOUNT
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_TIMER0
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_TIMER1
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_TIMER2
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_TIMER3
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_SERIAL
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_DMA0
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_DMA1
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_DMA2
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_DMA3
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_KEYPAD
	bne IntrMain_FoundIntr
	add r2, r2, #4
	ands r0, r1, #INTR_FLAG_GAMEPAK
IntrMain_FoundIntr:
	strh r0, [r3, #2]
	ldr r1, _pool_gIntrTable
	add r1, r1, r2
	ldr r0, [r1]
	bx r0

_pool_IntrVector:  .word INTR_VECTOR
_pool_AgbMain:     .word AgbMain
_pool_gIntrTable:  .word gIntrTable

	arm_func_start InitHeap
InitHeap: @ 080001D0
	mov r0, #EWRAM_START
	ldr r2, _pool_gHeapPtr_1
	str r0, [r2]
	bx lr

	arm_func_start Alloc
Alloc: @ 080001E0
	mov r2, #11
	add r0, r2, r0, lsl r1
	bic r0, r0, #3
	ldr ip, _pool_gHeapPtr_2
	ldr r2, [ip]
	add r1, r0, r2
	str r0, [r2]
	str r2, [r1, #-4]
	str r1, [ip]
	add r0, r2, #4
	bx lr

	arm_func_start Free
Free: @ 0800020C
	ldr r1, [r0, #-4]
	sub r1, r1, #8
	add r2, r1, r0
	ldr r3, [r2]
	sub r0, r0, #4
	ldr ip, _pool_gHeapPtr_3
	str r0, [ip]
	bx lr

	arm_func_start GetHeapPtr
GetHeapPtr: @ 0800022C
	ldr ip, _pool_gHeapPtr_4
	ldr r0, [ip]
	bx lr

	arm_func_start GetHeapUsed
GetHeapUsed: @ 08000238
	ldr ip, _pool_gHeapPtr_5
	ldr r2, [ip]
	mov r1, #EWRAM_START
	sub r0, r2, r1
	bx lr

	arm_func_start GetHeapFree
GetHeapFree: @ 0800024C
	ldr ip, _pool_gHeapPtr_6
	ldr r2, [ip]
	mov r1, #EWRAM_END
	sub r0, r1, r2
	bx lr

_pool_gHeapPtr_1: .word gHeapPtr
_pool_gHeapPtr_2: .word gHeapPtr
_pool_gHeapPtr_3: .word gHeapPtr
_pool_gHeapPtr_4: .word gHeapPtr
_pool_gHeapPtr_5: .word gHeapPtr
_pool_gHeapPtr_6: .word gHeapPtr

	arm_func_start AdvanceRng
AdvanceRng: @ 08000278
	ldr r1, _pool_rng_count_1
	ldrb r0, [r1]
	add r0, r0, #1
	strb r0, [r1]
	ldr r1, _pool_rng_state_1
	ldrb r0, [r1]
	add r2, r0, r0, lsl #2
	add r2, r2, #17
	strb r2, [r1]
	bx lr

	arm_func_start GetRandomValue
GetRandomValue: @ 080002A0
	ldr r0, _pool_rng_table_1
	ldr r3, _pool_rng_state_2
	ldrb r2, [r3]
	add ip, r2, r0
	ldr r0, [ip]
	and r0, r0, #0xFF
	ldr r1, _pool_rng_count_2
	ldrb ip, [r1]
	orr ip, ip, #1
	add r2, r2, ip
	strb r2, [r3]
	bx lr

	arm_func_start GetRandomValueEx
GetRandomValueEx: @ 080002D0
	ldr r0, _pool_rng_table_2
	ldr r3, _pool_rng_state_3
	ldrb r2, [r3]
	add ip, r2, r0
	ldr r0, [ip]
	and r0, r0, #0xFF
	ldr r1, _pool_rng_count_3
	ldrb ip, [r1]
	orr ip, ip, #1
	add r2, r2, ip
	strb r2, [r3]
	ldrb ip, [r1]
	add ip, ip, #1
	strb ip, [r1]
	add r1, r2, r2, lsl #2
	add r1, r1, #17
	strb r1, [r3]
	bx lr

@ RNG lookup table (256 bytes)
gRngTable: @ 08000318
	.incbin "baserom.gba", 0x318, 256

@ Padding
	.4byte 0x00000000

@ Pool for RNG functions
_pool_rng_count_1: .word gRngCount
_pool_rng_state_1: .word gRngState
_pool_rng_table_1: .word gRngTable
_pool_rng_state_2: .word gRngState
_pool_rng_count_2: .word gRngCount
_pool_rng_table_2: .word gRngTable
_pool_rng_state_3: .word gRngState
_pool_rng_count_3: .word gRngCount
