;	Copyright (c) 2003-2004 Arjan Bakker
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

		MODULE	bitbuster

; File: bitbuster
;
;	The bitbuster module gives you depack support for data packed
;	using the bitbuster algorithm.
;
; FUNCTION NAMES:
;	The <depack> function is MODULE local, which means you have to add the prefix 
;	"bitbuster." to its name. This is to prevent possible name clashes with
;	functions from other libraries you might be using. However, if you define
;	<MAKE_BITBUSTER_GLOBAL> then this function will be available without the 
;	"bitbuster." prefix as well.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Arjan Bakker under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
	
; DEFINE:	MAKE_BITBUSTER_GLOBAL
;	Defining this will make all public functions that are normally only
;	available with the "bitbuster." prefix to also be available without
;	this prefix. See the introduction of <bitbuster> for more information.
	
	
	
		; macro to get a bit from the bitstream
		; carry if bit is set, nocarry if bit is clear
		; must be entered with second registerset switched in!
		MACRO	GET_BIT_FROM_BITSTREAM
		add	a,a		;shift out new bit
		jp	nz,.done	;if remaining value isn't zere, we're done
	
		ld	a,(hl)		;get 8 bits from bitstream
		inc	hl		;increase source data address
	
		rla                     ;(bit 0 will be set!!!!)
.done:
		ENDM

	
; FUNCTION:	depack
;	Depack a blob of data that was packed with Bitbuster.
;
; ENTRY:
;	HL - Address of packed data
;	DE - Address to depack to
;
; EXIT:
;	HL - Size of depacked data
;
; MODIFIES:
;	#AF, BC, BC', DE, DE', HL, HL'#
;
depack:		EXPORT	bitbuster.depack
		IFDEF	MAKE_BITBUSTER_GLOBAL
@depack:	EXPORT	depack		
		ENDIF

		ld	c, (hl)
		inc	hl			;skip original file length
		ld	b, (hl)
		inc	hl			;which is stored in 4 bytes
		inc	hl
		inc	hl
		
		push	bc
		call	reallydepack
		pop	hl
		ret
		
reallydepack:

		ld	a,128
	
		exx
		ld	de,1
		exx
	
depack_loop:
		GET_BIT_FROM_BITSTREAM		;get compression type bit
		jp	c,output_compressed	;if set, we got lz77 compression
		ldi				;copy byte from compressed data to destination (literal byte)
		;unrolled for extra speed
		GET_BIT_FROM_BITSTREAM		;get compression type bit
		jp	c,output_compressed	;if set, we got lz77 compression
		ldi				;copy byte from compressed data to destination (literal byte)
		GET_BIT_FROM_BITSTREAM		;get compression type bit
		jp	c,output_compressed	;if set, we got lz77 compression
		ldi				;copy byte from compressed data to destination (literal byte)
		jp	depack_loop
	

;handle compressed data
output_compressed:
		ld	c,(hl)			;get lowest 7 bits of offset, plus offset extension bit
		inc	hl			;to next byte in compressed data

output_match:
		ld	b,0
		bit	7,c
		jr	z,output_match1		;no need to get extra bits if carry not set

		GET_BIT_FROM_BITSTREAM		;get offset bit 10 
		rl	b
		GET_BIT_FROM_BITSTREAM		;get offset bit 9
		rl	b
		GET_BIT_FROM_BITSTREAM		;get offset bit 8
		rl	b
		GET_BIT_FROM_BITSTREAM		;get offset bit 7

		jp	c,output_match1		;since extension mark already makes bit 7 set 
		res	7,c			;only clear it if the bit should be cleared
output_match1:
		inc	bc
		
; return a gamma-encoded value
; length returned in HL
		exx				;to second register set!
		ld	h,d
		ld	l,e             	;initial length to 1
		ld	b,e			;bitcount to 1

;determine number of bits used to encode value
get_gamma_value_size:
		exx
		GET_BIT_FROM_BITSTREAM			;get more bits
		exx
		jr	nc,get_gamma_value_size_end	;if bit not set, bitlength of remaining is known
		inc	b				;increase bitcount
		jp	get_gamma_value_size		;repeat...

get_gamma_value_bits:
		exx
		GET_BIT_FROM_BITSTREAM			;get next bit of value from bitstream
		exx
		
		adc	hl,hl				;insert new bit in HL
get_gamma_value_size_end:
		djnz	get_gamma_value_bits		;repeat if more bits to go

get_gamma_value_end:
		inc	hl			;length was stored as length-2 so correct this
		exx				;back to normal register set
	
		ret	c
		
		;HL' = length
		push	hl			;address compressed data on stack

		exx
		push	hl			;match length on stack
		exx

		ld	h,d
		ld	l,e			;destination address in HL...
		sbc	hl,bc			;calculate source address

		pop	bc			;match length from stack

		ldir				;transfer data

		pop	hl			;address compressed data back from stack

		GET_BIT_FROM_BITSTREAM		;get compression type bit
		jp	c,output_compressed	;if set, we got lz77 compression
		ldi				;copy byte from compressed data to destination (literal byte)
		GET_BIT_FROM_BITSTREAM		;get compression type bit
		jp	c,output_compressed	;if set, we got lz77 compression
		ldi				;copy byte from compressed data to destination (literal byte)

		jp	depack_loop
		
		MODULE
