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

		MODULE	keyboard
; File: keyboard
;	The keyboard module contains routines related to the keyboard.
;
; COPYRIGHT:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.

;****if* keyboard/init_keyboard
;
; Function:	init_keyboard
;	Initialises the keyboard module.
;
; Entry:
;	None
;
; Exit:
;	A - Error flag (0 if no error)
;	DE - Address of error string if #A# <> 0
;
; Modifies:
;	#ALL#
;
@init_keyboard:
		xor	a
		ret
		
;****if* keyboard/kill_keyboard
;
; FUNCTION:	kill_keyboard
;	Kills the keyboard module.
;
; ENTRY:
;	None
;
; EXIT:
;	None
;
; MODIFIES:
;	#ALL#
;
@kill_keyboard:
		ret

; FUNCTION:	read_keyrow
;	Read a keyboard row and mask out any unwanted keys. If only 1 bit in
;	the mask is set then the status of the Z flag on exit indicates whether
;	the key is down (NZ) or up (Z).
;
; ENTRY:
;	B - Keyboard row number
;	C - Mask
;	DI - or interrupt handler that doesn't corrupt the PPIPRC port
;	DI -
;
; EXIT:
;	A - (keyboard row XOR $ff) AND mask
;	FZ - Set if #A# = 0, clear otherwise
;
; MODIFIES:
;	#None#
;
@read_keyrow:	EXPORT	read_keyrow
		in	a,(PPIPRC)
		and	%11110000
		or	b
		out	(PPIPRC),a
		; If an interrupt occurs at this point and the interrupt
		; handler changes (and doesn't restore) PPIPRC then of
		; course the following read of port PPIPRB will very
		; likely not be from the requested row. That's why the
		; interrupts should be disabled or the interrupt handler
		; that's in place should leave the PPIPRC port alone (or
		; restore it to the current value after it's done with it).
		in	a,(PPIPRB)
		xor	$ff
		and	c
		ret

		MODULE
		