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

; FILE: interrupt
;	The interrupt module contains routines related to interrupts.
;
; COPYRIGHT:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.

;****if* interrupt/init_interrupt
;
; FUNCTION:	init_interrupt
;	Initialises the interrupt module.
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;
; MODIFIES:
;	#ALL#
;
init_interrupt:		
		xor	a
		ret
		
;****if* interrupt/kill_interrupt
;
; FUNCTION:	kill_interrupt
;	Kills the interrupt module.
;
; ENTRY:
;	#None#
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#ALL#
;
;******
;
kill_interrupt:
		ret

; CONSTANT:	SIZEOF_HOOKCHAIN
;	This contains the size, in bytes, of a hookchain structure. 
;	Hookchain structures are used by the <add_to_hookchain>
;	and <remove_from_hookchain> routines. To reserve a hookchain
;	structure somewhere in your program, just insert the following:@
;	:
;:my_hookchain_structure:	ds	SIZEOF_HOOKCHAIN
;
SIZEOF_HOOKCHAIN	equ	10

; FUNCTION:	add_to_hookchain
;	Adds some code to the chain of the given hook.
;	:
;	What is a hook-chain? Well, a hook is a piece of code that is
;	called by some routine that allows other programs/routines to
;	hook in on its execution. One of the most famous of these hooks
;	is probably the hook HTIMI which is located at $fd9f. This is called
;	by the interrupt routine that handles VDP VBlank interrupts (i.e.
;	normally called 50 or 60 times per second). Each hook is 5 bytes
;	and normally either just starts with a RET instruction or an RST 30
;	(4 bytes) followed by a RET. This does of course make it impossible
;	for two routines to both claim the hook. I mean, two RST 30 instructions
;	and one RET would be 9 bytes, but there's only 5 bytes for the hook.
;	Luckily there's a way around this problem: if a second routine wants
;	to claim the hook it first copies the code already at the hook and then
;	pokes an RST 30 to his hookroutine in the hook. This hookroutine should,
;	just before returning, make a call to the old, copied, contents of the hook.
;	In effect this creates a chain of hookroutines.
;	:
;	See <SIZEOF_HOOKCHAIN> for more information on hookchain structures.
;
; WARNING:
;	If you think a bit about the way this process works it's easy to see that
;	entries in the hookchain should be removed in the reverse order that they
;	were added. So, if you make two calls to <add_to_hookchain> for the *same*
;	hook then your *first* call to <remove_from_hookchain> must be for the
;	hookchain structure used on the *second* call to <add_to_hookchain>.
;
; ENTRY:
;	HL - Address of hookchain structure
;	DE - Hook address
;	BC - Address of hook code to be called
;	DI -
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#F, BC, DE, HL, IX#
;
add_to_hookchain:
		push	hl
		push	de
		
		ld	(hl), e			; Store the hook address in the chain structure
		inc	hl			;
		ld	(hl), d			;
		inc	hl			;
		
		ld	(hl), $cd		; Build a CALL <hook code> in the chain structure
		inc	hl			;
		ld	(hl), c			;
		inc	hl			;
		ld	(hl), b			;
		inc	hl			;
		
		ex	de, hl			; Copy the original hook code into the chain structure
		ld	bc, $0005		;
		ldir				;
		
		pop	hl
		pop	de
		
		ld	(hl), $f7		; Build a RST $30,<slotid>,<chain structure + 2>,RET in the hook
		inc	hl			;
		call	get_ramslotid		;
		ld	(hl),a			;
		inc	hl			;
		inc	de			;
		inc	de			;
		ld	(hl), e			;
		inc	hl			;
		ld	(hl), d			;
		inc	hl			;
		ld	(hl), $c9		;
		
		ret
		
; FUNCTION:	remove_from_hookchain
;	Removes a given hookchain from the chain of the given hook. See the
;	description of <add_to_hookchain> for more information about the hook
;	chain concept.
;
;	See <SIZEOF_HOOKCHAIN> for more information on hookchain structures.
;
; ENTRY:
;	HL - Address of chain structure
;	DI -
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#F, BC, DE, HL#
;
remove_from_hookchain:
		ld	e, (hl)			; Get the hook address
		inc	hl			;
		ld	d, (hl)			;
		inc	hl			;
		
		inc	hl			; Skip the CALL <hook code>
		inc	hl			;
		inc	hl			;
		
		ld	bc, $0005		; Copy the original hook code into the hook
		ldir				;
		
		ret

; CONSTANT:	SIZEOF_INTHANDLER
;	This contains the size, in bytes, of an inthandler structure. 
;	Inthandler structures are used by the <set_interrupt_handler>
;	and <reset_interrupt_handler> routines. To reserve an inthandler 
;	structure somewhere in your program, just insert the following:@
;	:
;:my_inthandler_structure:	ds	SIZEOF_INTHANDLER
;
SIZEOF_INTHANDLER	equ	5

; FUNCTION:	set_interrupt_handler
;	Copies the code at $0038 to the given inthandler structure and
;	then writes a JP to the given handler code at $0038. See 
;	<SIZEOF_INTHANDLER> for more information on inthandler structures.
;
; ENTRY:
;	HL - Address of inthandler structure
;	DE - Address of interrupt handler code to be called
;	DI -
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF, BC, DE, HL#
;
set_interrupt_handler:
		push	de
		push	hl
		pop	de
		ld	hl, $0038
		ld	bc, $0005
		ldir
		pop	de
		ld	a, $c3
		ld	($0038), a
		ld	($0039), de
		ret

; FUNCTION:	reset_interrupt_handler
;	Copies the code stored in the given inthandler structure
;	to address $0038. Do not call this routine if you have
;	not made a call to <set_interrupt_handler> with the given
;	inthandler structure before. See <SIZEOF_INTHANDLER> for 
;	more information on inthandler structures.
;
; ENTRY:
;	HL - Address of inthandler structure
;	DI -
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#F, BC, DE, HL#
;
reset_interrupt_handler:
		ld	de, $0038
		ld	bc, $0005
		ldir
		ret
