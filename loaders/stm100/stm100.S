.thumb
.syntax unified
.global prestart
	
.section .text
.org 0

@r0 	rampointer
@r1		flashpointer
@r2		statusregister
@r3		temp
@r4		bufferend

_prestart:
	nop
	nop
	b	_start

_start:
	ldrh	r3, [r0]	@load data from ram
	strh 	r3, [r1]	@load data to flash	
	
_wait:
	ldr		r3, [r2]	@load flashstatus register	
	tst.w	r3, #0x01	@test if writing finished
	bne 	_wait

	add		r0,#2		@increment pointers
	add 	r1,#2
						
	cmp  	r0, r4		@compare end of buffer
	bne		_start		@next halfword

_done:
	bkpt
	b _done
