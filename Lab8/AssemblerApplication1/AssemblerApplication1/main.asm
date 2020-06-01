.include "m2560def.inc"


.equ zero = 0x00
.equ led_count = 0x09
.equ all = 0xFF


.def port_a = r20
.def port_b = r21
.def left_b = r22
.def right_b = r23
.def counter = r19


.CSEG
		;Mapping
		
		.org	0x000
		jmp	setup 

		.org 0x005E
		rjmp Timer5CompAVect

;Timer5 Interupts

Timer5CompAVect:
		cpi counter, 0x00
		BREQ first_skip
		lsl port_a
		inc port_a
		BRTS right_shift
		lsl left_b
		mov port_b, left_b
		set

return:
		inc counter
		sts TCNT5H, r17
		sts TCNT5L, r17
		reti

first_skip:
		cbi PORTB, 1
		mov port_b, left_b
		ldi port_a, 0b00000001
		rjmp return

right_shift:
		cpi counter, 0x01
		mov port_b, right_b
		clt
		BREQ return
		lsr right_b
		mov port_b, right_b
		rjmp return
		

;Setup function
		
setup:
		ldi r16, 0xFF
		out DDRA, r16
		out DDRF, r16
		ldi r17, (1<<DDB1)
		out DDRB, r17
		ldi r17, 0x00
		ldi r18, (1<<DDB2) | (1<<DDB0)
		out PORTA, r17
		out PORTF, r17
		out PORTB, r18
		
		cli
		sts TCNT5H, r17
		sts TCNT5L, r17
		clr r18

		ldi r18, (1<<CS51) | (1<<CS50)
		sts TCCR5B, r18
		clr r18

		ldi r18, 0x7A
		ldi counter, 0x12
		sts OCR5AH, r18
		sts OCR5AL, counter
		clr counter

		ldi counter, (1<<OCIE5A)
		sts TIMSK5, counter
		ldi r21, led_count
		clr counter
		rjmp main

end_timer:
		cli
		ldi counter, zero
		ldi port_a, 0b10000000
		nop
		rjmp main


; Main loop listening for buttons

main:   nop
		nop
		nop
		ldi counter, 0x00
		ldi port_a, 0b00000000
		out PORTF, r17
		out PORTA, r17
		ldi r18, PINB
		sbis PINB, 0
		rjmp first
		sbis PINB, 2
		rjmp second
		rjmp main


;First algorithm setup

first:
		sbi PORTB, 1
		sts TCNT5H, r17
		sts TCNT5L, r17
		nop
		sei

first_loop:
		out PORTA, port_a
		cpi counter, led_count
		BREQ end_timer
		rjmp first_loop


;Second Algorithm setup

second: 
		sbi PORTB, 1
		sts TCNT5H, r17
		sts TCNT5L, r17
		ldi left_b, 0b00000001
		ldi right_b, 0b10000000
		ldi port_b, 0b00000000
		set
		sei

second_loop:
		out PORTF, port_b
		cpi counter, led_count
		BREQ end_timer
		rjmp second_loop
