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

_start:
	ldr		r3, [r0]	@load data from ram
	str 	r3, [r1]	@load data to flash	
	
_wait:
	ldr		r3, [r2]	@load flashstatus register	
	tst.w	r3, #0x01	@test if writing finished
	bne 	_wait

	add		r0,#4		@increment pointers
	add 	r1,#4
						
	cmp  	r0, r4		@compare end of buffer
	bne		_start		@next halfword

_done:
	bkpt
	b _done
