;	Copyright (c) 2003-2004 Eli-Jean Leyssens
;	
;	Permission is hereby granted, free of charge, to any person obtaining a copy of 
;	this software and associated documentation files (the "Software"), to deal in 
;	the Software without restriction, including without limitation the rights to 
;	use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of 
;	the Software, and to permit persons to whom the Software is furnished to do so, 
;	subject to the following conditions:
;	
;	The above copyright notice and this permission notice shall be included in all 
;	copies or substantial portions of the Software.
;	
;	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
;	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS 
;	FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR 
;	COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER 
;	IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN 
;	CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

; FILE:	macros
;	Contains a list of useful macros for your perusal.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
;	Many thanks to Arjan Bakker for providing the basis for many of the routines
;	in this module. Thanks to Marcel Delorme for the HEXTOASCII macro.
		
; MACRO:	INCNZ	REG
;	Increase a register(pair) if #FZ# is clear (NZ).
;
; ENTRY:
;	REG - Register(pair) to be conditionally increased
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was clear:
;	REG - #Entry REG# + 1
;	F - As for a normal INC of the given register(pair)
;	:
;	If #Entry FZ# was set:
;	REG - #Entry REG#
;	FZ - #Entry FZ#
;
; MODIFIES:
;	#None#
		MACRO	INCNZ REG
		jr	z,.done
		inc	REG
.done
		ENDM

; MACRO:	INCZ	REG
;	Increase a register(pair) if #FZ# is set (Z).
;
; ENTRY:
;	REG - Register(pair) to be conditionally increased
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was clear:
;	REG - #Entry REG#
;	FZ - #Entry FZ#
;	:
;	If #Entry FZ# was set:
;	REG - #Entry REG# + 1
;	F - As for a normal INC of the given register(pair)
;
; MODIFIES:
;	#None#
		MACRO	INCZ REG
		jr	nz,.done
		inc	REG
.done
		ENDM

; MACRO:	INCNC	REG
;	Increase a register(pair) if #FC# is clear (NC).
;
; ENTRY:
;	REG - Register(pair) to be conditionally increased
;	FC - 
;
; EXIT:
;	If #Entry FC# was clear:
;	REG - #Entry REG# + 1
;	F - As for a normal INC of the given register(pair)
;	:
;	If #Entry FC# was set:
;	REG - #Entry REG#
;	FC - #Entry FC#
;
; MODIFIES:
;	#None#
		MACRO	INCNC REG
		jr	c,.done
		inc	REG
.done
		ENDM

; MACRO:	INCC	REG
;	Increase a register(pair) if #FC# is set (C).
;
; ENTRY:
;	REG - Register(pair) to be conditionally increased
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was set:
;	REG - #Entry REG# + 1
;	F - As for a normal INC of the given register(pair)
;	:
;	If #Entry FZ# was clear:
;	REG - #Entry REG#
;	FC - #Entry FC#
;
; MODIFIES:
;	#None#
		MACRO	INCC REG
		jr	nc,.done
		inc	REG
.done
		ENDM

; MACRO:	DECNZ	REG
;	Decrease a register(pair) if #FZ# is clear (NZ).
;
; ENTRY:
;	REG - Register(pair) to be conditionally decreased
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was clear:
;	REG - #Entry REG# - 1
;	F - As for a normal DEC of the given register(pair)
;	:
;	If #Entry FZ# was set:
;	REG - #Entry REG#
;	FZ - #Entry FZ#
;
; MODIFIES:
;	#None#
		MACRO	DECNZ REG
		jr	z,.done
		dec	REG
.done
		ENDM

; MACRO:	DECZ	REG
;	Decrease a register(pair) if #FZ# is set (Z).
;
; ENTRY:
;	REG - Register(pair) to be conditionally decreased
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was clear:
;	REG - #Entry REG#
;	FZ - #Entry FZ#
;	:
;	If #Entry FZ# was set:
;	REG - #Entry REG# - 1
;	F - As for a normal DEC of the given register(pair)
;
; MODIFIES:
;	#None#
		MACRO	DECZ REG
		jr	nz,.done
		dec	REG
.done
		ENDM

; MACRO:	DECNC	REG
;	Decrease a register(pair) if #FC# is clear (NC).
;
; ENTRY:
;	REG - Register(pair) to be conditionally decreased
;	FC - 
;
; EXIT:
;	If #Entry FC# was clear:
;	REG - #Entry REG# - 1
;	F - As for a normal DEC of the given register(pair)
;	:
;	If #Entry FC# was set:
;	REG - #Entry REG#
;	FC - #Entry FC#
;
; MODIFIES:
;	#None#
		MACRO	DECNC REG
		jr	c,.done
		dec	REG
.done
		ENDM

; MACRO:	DECC	REG
;	Decrease a register(pair) if #FC# is set (C).
;
; ENTRY:
;	REG - Register(pair) to be conditionally decreased
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was set:
;	REG - #Entry REG# - 1
;	F - As for a normal INC of the given register(pair)
;	:
;	If #Entry FZ# was clear:
;	REG - #Entry REG#
;	FC - #Entry FC#
;
; MODIFIES:
;	#None#
		MACRO	DECC REG
		jr	nc,.done
		dec	REG
.done
		ENDM

; MACRO:	LDNZ	REG, VALUE
;	Load a register(pair) with a value if #FZ# is clear (NZ).
;
; ENTRY:
;	REG - Register(pair) to be conditionally loaded
;	VALUE - Value to be conditionally loaded into the register(pair)
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was clear:
;	REG - #VALUE#
;	:
;	If #Entry FZ# was set:
;	REG - #Entry REG#
;
; MODIFIES:
;	#None#
		MACRO	LDNZ REG,VALUE
		jr	z,.done
		ld	REG,VALUE
.done
		ENDM

; MACRO:	LDZ	REG, VALUE
;	Load a register(pair) with a value if #FZ# is set (Z).
;
; ENTRY:
;	REG - Register(pair) to be conditionally loaded
;	VALUE - Value to be conditionally loaded into the register(pair)
;	FZ - 
;
; EXIT:
;	If #Entry FZ# was clear:
;	REG - #Entry REG#
;	:
;	If #Entry FZ# was set:
;	REG - #VALUE#
;
; MODIFIES:
;	#None#
		MACRO	LDZ REG,VALUE
		jr	nz,.done
		ld	REG,VALUE
.done
		ENDM

; MACRO:	LDNC	REG, VALUE
;	Load a register(pair) with a value if #FC# is clear (NC).
;
; ENTRY:
;	REG - Register(pair) to be conditionally loaded
;	VALUE - Value to be conditionally loaded into the register(pair)
;	FC - 
;
; EXIT:
;	If #Entry FC# was clear:
;	REG - #VALUE#
;	:
;	If #Entry FC# was set:
;	REG - #Entry REG#
;
; MODIFIES:
;	#None#
		MACRO	LDNC REG,VALUE
		jr	c,.done
		ld	REG,VALUE
.done
		ENDM

; MACRO:	LDC	REG, VALUE
;	Load a register(pair) with a value if #FC# is set (C).
;
; ENTRY:
;	REG - Register(pair) to be conditionally loaded
;	VALUE - Value to be conditionally loaded into the register(pair)
;	FC - 
;
; EXIT:
;	If #Entry FC# was clear:
;	REG - #Entry REG#
;	:
;	If #Entry FC# was set:
;	REG - #VALUE#
;
; MODIFIES:
;	#None#
		MACRO	LDC REG,VALUE
		jr	nc,.done
		ld	REG,VALUE
.done
		ENDM

; MACRO:	NEG_HL
;	Negate #HL#. In pseudo code:@
;: HL = -HL
;
; ENTRY:
;	HL - 
;
; EXIT:
;	HL - -#Entry HL#
;
; MODIFIES:
;	#AF#
		MACRO	NEG_HL
		xor 	a 			; a = 0 
		sub 	l 			; a = 0-L
		ld 	l,a 
		sbc 	a,a 			; a = (a-a)-carry = -carry 
		sub 	h 			; a = -H-carry 
		ld 	h,a
		ENDM

; MACRO:	NEG_DE
;	Negate #DE#. In pseudo code:@
;: DE = -DE
;
; ENTRY:
;	DE - 
;
; EXIT:
;	DE - -#Entry DE#
;
; MODIFIES:
;	#AF#
		MACRO	NEG_DE
		xor 	a 			; a = 0 
		sub 	e 			; a = 0-E
		ld 	e,a 
		sbc 	a,a 			; a = (a-a)-carry = -carry 
		sub 	d 			; a = -D-carry 
		ld 	d,a
		ENDM

; MACRO:	NEG_BC
;	Negate #BC#. In pseudo code:@
;: BC = -BC
;
; ENTRY:
;	BC - 
;
; EXIT:
;	BC - -#Entry BC#
;
; MODIFIES:
;	#AF#
		MACRO	NEG_BC		
		xor 	a 			; a = 0 
		sub 	c 			; a = 0-C
		ld 	c,a 
		sbc 	a,a 			; a = (a-a)-carry = -carry 
		sub 	b 			; a = -B-carry 
		ld 	b,a
		ENDM
		
; MACRO:	ADD_HL_A
;	Add #A# to #HL#. In pseudo code:@
;:	HL = HL + A
;
; ENTRY:
;	HL - 
;	A -
;
; EXIT:
;	HL - #Entry HL# + #Entry A#
;
; MODIFIES:
;	#AF#
		MACRO	ADD_HL_A
		add     a,l		;4
		jr      nc,.hdone	;7/12
		inc     h		;4
.hdone:		ld      l,a		;4
		ENDM			;19/20 (load=7)


; MACRO:	ADD_DE_A
;	Add #A# to #DE#. In pseudo code:@
;:	DE = DE + A
;
; ENTRY:
;	DE - 
;	A -
;
; EXIT:
;	DE - #Entry DE# + #Entry A#
;
; MODIFIES:
;	#AF#
		MACRO	ADD_DE_A
		add	a,e
		jr	nc,.ddone
		inc	d
.ddone:		ld	e,a
		ENDM

; MACRO:	ADD_BC_A
;	Add #A# to #BC#. In pseudo code:@
;:	BC = BC + A
;
; ENTRY:
;	BC - 
;	A -
;
; EXIT:
;	BC - #Entry BC# + #Entry A#
;
; MODIFIES:
;	#AF#
		MACRO	ADD_BC_A
		add	a,c
		jr	nc,.bdone
		inc	b
.bdone:		ld	c,a
		ENDM

; MACRO:	SLA_HL
;	Shift left arithmetic registerpair #HL#.
;
; ENTRY:
;	HL -
;
; EXIT:
;	HL - #Entry HL# arithmetically shifted left by one bit
;	FC - Contains a copy of bit 15 of #Entry HL#
;	FS - Set if the 16-bit result is negative; reset otherwise
;
; MODIFIES:
;	#F#
		MACRO	SLA_HL
		sla	l
		rl	h
		ENDM

; MACRO:	SLA_DE
;	Shift left arithmetic registerpair #DE#.
;
; ENTRY:
;	DE -
;
; EXIT:
;	DE - #Entry DE# arithmetically shifted left by one bit
;	FC - Contains a copy of bit 15 of #Entry DE#
;	FS - Set if the 16-bit result is negative; reset otherwise
;
; MODIFIES:
;	#F#
		MACRO	SLA_DE		
		sla	e
		rl	d
		ENDM
		
; MACRO:	SLA_BC
;	Shift left arithmetic registerpair #BC#.
;
; ENTRY:
;	BC -
;
; EXIT:
;	BC - #Entry BC# arithmetically shifted left by one bit
;	FC - Contains a copy of bit 15 of #Entry BC#
;	FS - Set if the 16-bit result is negative; reset otherwise.
;
; MODIFIES:
;	#F#
		MACRO	SLA_BC
		sla	c
		rl	b
		ENDM

; MACRO:	LD_HL_A
;	Load registerpair #HL# with the value in register #A#.
;
; ENTRY:
;	HL -
;	A -
;
; EXIT:
;	H - 0
;	L - #Entry A#
;
; MODIFIES:
;	#None#
		MACRO	LD_HL_A
		ld	l, a
		ld	h, 0
		ENDM

; MACRO:	LD_DE_A
;	Load registerpair #DE# with the value in register #A#.
;
; ENTRY:
;	DE -
;	A -
;
; EXIT:
;	D - 0
;	E - #Entry A#
;
; MODIFIES:
;	#None#
		MACRO	LD_DE_A
		ld	e, a
		ld	d, 0
		ENDM
		
; MACRO:	LD_BC_A
;	Load registerpair #BC# with the value in register #A#.
;
; ENTRY:
;	BC -
;	A -
;
; EXIT:
;	B - 0
;	C - #Entry A#
;
; MODIFIES:
;	#None#
		MACRO	LD_BC_A
		ld	c, a
		ld	b, 0
		ENDM

; MACRO:	SUB_HL_A
;	Subtract #A# from #HL#. In pseudo code:@
;:	HL = HL - A
;
; ENTRY:
;	HL - 
;	A -
;
; EXIT:
;	HL - #Entry HL# - #Entry A#
;
; MODIFIES:
;	#AF#
		MACRO	SUB_HL_A
		neg	a
		jr	z, .done
		add	a, l
		jr	c, .done
		dec	h
.done
		ld	l, a
		ENDM

; MACRO:	SUB_DE_A
;	Subtract #A# from #HL#. In pseudo code:@
;:	DE = DE - A
;
; ENTRY:
;	DE - 
;	A -
;
; EXIT:
;	DE - #Entry DE# - #Entry A#
;
; MODIFIES:
;	#AF#
		MACRO	SUB_DE_A
		neg	a
		jr	z, .done
		add	a, e
		jr	c, .done
		dec	d
.done
		ld	e, a
		ENDM

; MACRO:	SUB_BC_A
;	Subtract #A# from #BC#. In pseudo code:@
;:	BC = BC - A
;
; ENTRY:
;	BC - 
;	A -
;
; EXIT:
;	BC - #Entry BC# - #Entry A#
;
; MODIFIES:
;	#AF#
		MACRO	SUB_BC_A
		neg	a
		jr	z, .done
		add	a, c
		jr	c, .done
		dec	b
.done
		ld	c, a
		ENDM
		
; MACRO:	SUB_HL_DE
;	Subtract #DE# from #HL#. In pseudo code:@
;:	HL = HL - DE
;
; ENTRY:
;	HL - 
;	DE -
;
; EXIT:
;	HL - #Entry HL# - #DE#
;	F - As for SBC HL, DE
;
; MODIFIES:
;	#None#
		MACRO	SUB_HL_DE
		or	a			; Clear the Carry flag
		sbc	hl, de
		ENDM		

; MACRO:	SUB_HL_BC
;	Subtract #BC# from #HL#. In pseudo code:@
;:	HL = HL - BC
;
; ENTRY:
;	HL - 
;	BC -
;
; EXIT:
;	HL - #Entry HL# - #BC#
;	F - As for SBC HL, BC
;
; MODIFIES:
;	#None#
		MACRO	SUB_HL_BC
		or	a			; Clear the Carry flag
		sbc	hl, bc
		ENDM		

; MACRO:	CP_HL_DE
;	Compare #HL# with #DE#.
;
; ENTRY:
;	HL - 
;	DE -
;
; EXIT:
;	FC - Set if HL < DE; reset otherwise.
;	FZ - Set if HL = DE; reset otherwise.
;
; MODIFIES:
;	#AF#
		MACRO	CP_HL_DE
		ld      a, h
		sub     d
		jr      nz,.done		; Not equal
		ld      a, l
		sub     e
.done:
		ENDM

; MACRO:	CP_HL_BC
;	Compare #HL# with #BC#.
;
; ENTRY:
;	HL - 
;	BC -
;
; EXIT:
;	FC - Set if HL < BC; reset otherwise.
;	FZ - Set if HL = BC; reset otherwise.
;
; MODIFIES:
;	#AF#
		MACRO	CP_HL_BC
		ld      a, h
		sub     b
		jr      nz,.done		; Not equal
		ld      a, l
		sub     c
.done:
		ENDM

; MACRO:	CALL_HL
;	Calls the address contained in registerpair HL.
;
; ENTRY:
;	HL - Address to CALL
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#None#
		MACRO	CALL_HL
		call	.jphl
		jr	.done
.jphl:		jp	(hl)
.done
		ENDM		

; MACRO:	WAIT
;	Wait for an interrupt.
;
; ENTRY:
;	#None#
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#None#
		MACRO	WAIT
		ei
		nop
		halt
		ENDM
		
; MACRO:	WRITEVDP
;	Write to a VDP register.
;
; ENTRY:
;	VDPREG - The VDP register to write to
;	VALUE - The value to be written
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#A#
		MACRO	WRITEVDP VDPREG,VALUE
		ld	a, VALUE
		out	(#99),a
		ld	a, (VDPREG) + 128
		out	(#99),a
		ENDM
										
; MACRO:	ROMCALL	ADDRESS
;	Calls an address in the normal BIOS ROM in slot 0.
;
; ENTRY:
;	ADDRESS - Address to be called
;
; EXIT:
;	DI -
;
; MODIFIES:
;	#AF', BC', DE', HL', IX, IY#
		MACRO	ROMCALL ADDRESS
		ld	ix, ADDRESS
		ld	iy, (EXPTBL - 1)
		call	CALSLT
		ENDM

; MACRO:	SUBCALL	ADDRESS
;	Calls an address in the SUB-ROM.
;
; ENTRY:
;	ADDRESS - Address to be called
;
; EXIT:
;	DI -
;
; MODIFIES:
;	#AF', BC', DE', HL', IX, IY#
;
; DEPENDANCIES:
;	<environment> module
		MACRO	SUBCALL ADDRESS
		ld	ix, ADDRESS
		call	call_subrom
		ENDM

; MACRO:	HEXTOASCII
;	Converts the hexadecimal value in register #A# to its
;	ASCII representation.
;
; ENTRY:
;	A - Hexadecimal value. Only the lower 4 bits (nibble)
;	    are used. The upper 4 bits are ignored.
;	
; EXIT:
;	A - ASCII representation of the #Entry A#.
;
; MODIFIES:
;	#F#
		MACRO	HEXTOASCII
		and	$0f
		cp	10
		sbc	a,$69
		daa
		ENDM

; MACRO:	JP_PC_2A
;	Jump to the #A#'th address in the word table immediately
;	following this macro. The 2A in the macro name refers to
;	the fact that to get to the #A#'th entry we need to
;	multiply A by 2, since each entry is a word, i.e. 2 bytes.
;
; ENTRY:
;	A - Index into table. Bit 7 is ignored.
;	
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF#
		MACRO	JP_PC_2A
		push	hl
		ld	hl, .table
		add	a, a
		ADD_HL_A
		ld	a, (hl)
		inc	hl
		ld	h, (hl)
		ld	l, a
		ex	(sp), hl
		ret
.table:
		ENDM
