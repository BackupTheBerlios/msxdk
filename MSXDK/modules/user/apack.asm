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

		MODULE	apack
; FILE:	apack
;
;	The apack module gives you depack support for data packed
;	using the aPACK algorithm. See http://www.ibsensoftware.com/products_aPLib.html
;	for more info on aPACK.
;
; FUNCTION NAMES:
;	The <depack> function is MODULE local, which means you have to add the prefix 
;	"apack." to its name. This is to prevent possible name clashes with
;	functions from other libraries you might be using. However, if you define
;	<MAKE_APACK_GLOBAL> then this function will be available without the "apack."
;	prefix as well.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
;	Thanks go to GuyveR800 for supplying some of the nifty tricks and the GETBIT macro
;	used in this code.

; DEFINE:	MAKE_APACK_GLOBAL
;	Defining this will make all public functions that are normally only
;	available with the "apack." prefix to also be available without
;	this prefix. See the introduction of <apack> for more information.

; DEFINE:	APACK_DEPACK_OPTIMISE_CODESIZE
;	Define this if you want the smallest possible code for the apack.depack routine.
; 	This does ofcourse cost you execution performance...
		
;		DEFINE	APACK_DEPACK_OPTIMISE_CODESIZE
		
		IFDEF	APACK_DEPACK_OPTIMISE_CODESIZE
		
		MACRO	JUMP LABEL
		jr	@LABEL
		ENDM	
		MACRO	JUMPCC COND,LABEL
		jr	COND,@LABEL
		ENDM
		
		MACRO	GETBIT
		call	getbit
		ENDM
getbit:
		add	a, a
		ret	nz
		ld	a,(hl)
		inc	hl
		adc	a, a
		ret
	
		MACRO	GETGAMMABCIS0001
		call	getgamma_bc_is_0001
		ENDM	
		MACRO	GETGAMMA
		call	getgamma
		ENDM
getgamma:
		ld	bc, $0001
getgamma_bc_is_0001:
getgammaloop:
		GETBIT
		rl	c
		rl	b
		GETBIT
		jr	c, getgammaloop
		ret
	
		ELSE
	
		MACRO	JUMP LABEL
		jp	@LABEL
		ENDM	
		MACRO	JUMPCC COND,LABEL
		jp	COND,@LABEL
		ENDM	
		MACRO	GETBIT
		add	a, a
		jp	nz, .done
		ld	a,(hl)
		inc	hl
		adc	a, a
.done:
		ENDM

		MACRO	GETGAMMABCIS0001
.loop:
		GETBIT	
		rl	c
		rl	b
		GETBIT		
		jp	c, .loop
		ENDM	
		MACRO	GETGAMMA
		ld	bc, $0001
		GETGAMMABCIS0001
		ENDM
		
		ENDIF
	
; FUNCTION: depack
;	Depack a blob of data that was packed using aPLib's aP_pack call.
;	That is, the packed data should not be preceded by any kind of extra
;	header, be it x86 depack code or a crc or the original size etc. In
;	other words, just packed data only.
;
; ENTRY:
;	HL - Address of packed data
;	DE - Address to depack to
;
; EXIT:
;	HL - Size of depacked data
;
; MODIFIES:
;	#AF, AF', BC, DE, IX#
;
depack:		EXPORT	apack.depack
		IFDEF	MAKE_APACK_GLOBAL
@depack:	EXPORT	depack		
		ENDIF
		push	de
		ld	a,$80
		
		IFDEF	APACK_DEPACK_OPTIMISE_CODESIZE
literal:
		ex	af, af'
		ld	a, $fe
		ex	af, af'
		ldi
nexttag:
		GETBIT
		jr	nc, literal
		ld	bc, $0000
		ELSE
literal1:
		ex	af, af'
		ld	a, $fe
		ex	af, af'
literal2:
		ldi
		GETBIT
		jp	nc, literal2
		ld	bc, $0000
		jp	nonliteral
nexttag:
		GETBIT
		jp	nc, literal1
nonliteral:
		; bc = $0000, either explicitly set above, or from the ldir in "domatch"
		ENDIF
		
		push	de
		GETBIT
		JUMPCC	nc, codepair
		ld	de, $0000
		GETBIT
		JUMPCC	nc, shortmatch
		ex	af, af'
		ld	a, $fe
		ex	af, af'
		inc	c
		IFDEF	APACK_DEPACK_OPTIMISE_CODESIZE
		ld	e, $10
getmorebits:
		GETBIT
		rl	e
		JUMPCC	nc, getmorebits
		ccf			; Clear (by complementing) the carry for the "sbc hl, de" later on in domatch
		ELSE
		GETBIT
		rl	e
		GETBIT
		rl	e
		GETBIT
		rl	e
		GETBIT
		rl	e		; Carry always clear since register e was zero before these 4 rl's
		ENDIF
		JUMPCC	nz, domatch
		pop	de
		ex	de, hl
		ld	(hl),b
		inc	hl
		ex	de, hl
		dec	c		; Have to make sure BC equals $0000 again
		JUMP	nexttag

codepair:
		inc	c
		GETGAMMABCIS0001
		ex	af, af'
		add	a, c
		ld	c, a
		JUMPCC	nz, normalcodepair
		ex	af, af'
		GETGAMMA
		ex	af, af'
		JUMP	domatch_lastpos

shortmatch:
		ld	e,(hl)
		inc	hl
		srl	e
		jr	z, donedepacking	; Only true once per depack session, so JR is faster than JP
		rl	c			; Always clears the carry since register c was zero.
		ex	af,af'
		JUMP	domatch_with_2inc

normalcodepair:
		ex	af, af'
		ld	d, c
		dec	d
		ld	e,(hl)
		inc	hl
		GETGAMMA
		ex	af, af'
		ld	a, d
		cp	$7d
		jr	nc, domatch_with_2inc		; This case doesn't occur that often, so JR will be faster than JP
		cp	$05
		JUMPCC	nc, domatch_with_inc
		or	a
		JUMPCC	nz, domatch_new_lastpos
		or	e
		jp	m, domatch_new_lastpos    
domatch_with_2inc:
		inc	bc
domatch_with_inc:
		inc	bc
domatch_new_lastpos:
		ld	ixl, e
		ld	ixh, d
		IFNDEF 	APACK_DEPACK_OPTIMISE_CODESIZE
		jp	domatch_lastposgotten
		ENDIF
domatch_lastpos:
		ld	e, ixl
		ld	d, ixh
domatch_lastposgotten:
		ld	a, $ff
		ex	af, af'
domatch:
		ex	(sp),hl
		; This "or a" isn't necessary since all paths leading to this code point
		; clear the carry already.
		; or	a		; clear the carry
		sbc	hl, de
		ex	de, hl
		add	hl, de
		ex	de, hl
		ldir
		pop	hl
		JUMP	nexttag

donedepacking:
		pop	hl
		pop	de
		; Carry will always be clear when execution reaches this point.
		sbc	hl, de
		ret
	
		MODULE
