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

		MODULE	debug
		
; FILE: debug
;	Contains some routines for outputting debug information. There are
;	two possible output streams:
;	- openMSX's debugdevice
;	- a cyclic buffer
;	:
;	openMSX debugdevice:@
;	If you load up openMSX with -ext debugdevice or if you enable
;	the debugdevice through one of the xml files then this can be used
;	by the debug module to send debug output to. Define <INCLUDE_DEBUG_OPENMSX>
;	if you want to use this debug output stream.
;	:
;	Cyclic buffer:@
;	Especially useful if you're trying to debug on a real machine or
;	just an emulator that doesn't support the debug device. All debug
;	output is stored into a cyclic buffer, which is written to a file
;	called "DEBUG.TXT" when <debug_kill> is called. It's cyclic in the
;	sense that when it gets full and still more debug output is generated
;	it will wrap back to the start of the buffer and overwrite any debug
;	output that was already stored in it. Correctly define
;	<INCLUDE_DEBUG_BUFFER_SIZE> if you want to use the cyclic buffer as
;	a debug output stream.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.

; DEFINE:	INCLUDE_DEBUG_OPENMSX
;	Define this if you want to use openMSX's debugdevice as an output
;	stream.

; DEFINE:	INCLUDE_DEBUG_BUFFER_SIZE
;	As you might guess from the name this both enables the cyclic buffer
;	and also contains the size of the buffer in bytes. See the <debug>
;	introduction for more information about the cyclic buffer. Since 
;	this buffer resides in the core and thus in the .COM file you really 
;	should not make this buffer too big.

;****if* debug/init_debug
;
; Function:	init_debug
;	Initialises the debug module.
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
@init_debug:
		xor	a
		ret
		
;****if* debug/kill_debug
;
; FUNCTION:	kill_debug
;	Kills the debug module.
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
@kill_debug:
		IFDEF	INCLUDE_DEBUG_BUFFER_SIZE
		ld	c, BDOS_CREATEFILE
		ld	de, fcb
		call	BDOS
		or	a
		ret	nz
		
		ld	de, $0001
		ld	( fcb + FCB_RECORDSIZE), de
		
		ld	de, (buffer_pos)
		ld	a, (de)
		or	a
		jr	z, .write_second_part
		
		ld	c, BDOS_SETDTA
		call	BDOS
		ld	c, BDOS_RANDOMBLOCKWRITE		
		ld	de, fcb
		ld	hl, (buffer_left)
		call	BDOS		
.write_second_part:

		ld	de, buffer
		ld	hl, (buffer_pos)
		CP_HL_DE
		jr	z, .done
		sbc	hl, de
		push	hl
		ld	c, BDOS_SETDTA
		call	BDOS
		pop	hl
		ld	de, fcb
		ld	c, BDOS_RANDOMBLOCKWRITE
		call	BDOS		
.done:
		ld	c, BDOS_CLOSEFILE
		ld	de, fcb
		call	BDOS
		ENDIF
		ret
		
; FUNCTION:	debug_string
;	Outputs a zero-terminated string to the debug stream(s).
;
; ENTRY:
;	HL - Address of zero-terminated string.
;
; EXIT:
;	#None#
;
; MODFIES:
;	#None#
;
@debug_string:
		push	af
		push	hl
		IFDEF	INCLUDE_DEBUG_BUFFER_SIZE
		push	bc
		push	de
		ld	bc, (buffer_left)
		ld	de, (buffer_pos)
		ENDIF
				
		IFDEF	INCLUDE_DEBUG_OPENMSX
		ld	a,%01100011		; Debug mode 2, ASCII output, no forced linefeed
		out	(DEBUGDEVCMD), a
		ENDIF
.loop:
		ld	a, (hl)
		or	a
		jr	z, .done
		IFDEF	INCLUDE_DEBUG_OPENMSX
		out	(DEBUGDEVDATA), a	; Output character to debug device
		ENDIF
		IFDEF	INCLUDE_DEBUG_BUFFER_SIZE
		ld	(de), a
		inc	de
		dec	bc
		ld	a, b
		or	c
		jr	nz, .next
		ld	de, buffer
		ld	bc, INCLUDE_DEBUG_BUFFER_SIZE
.next:
		ENDIF
		inc	hl
		jp	.loop				
.done:		
		IFDEF	INCLUDE_DEBUG_OPENMSX
		ld	a,%01100011		; Debug mode 2, ASCII output, no forced linefeed
		out	(DEBUGDEVCMD), a
		ENDIF
		
		IFDEF	INCLUDE_DEBUG_BUFFER_SIZE
		ld	(buffer_pos),de
		ld	(buffer_left), bc
		pop	de
		pop	bc
		ENDIF
		pop	hl
		pop	af
		ret

; FUNCTION:	debug_byte
;	 Outputs a byte to the debug stream(s). The format is:@ $xx\r\n.
;
; ENTRY:
;	A - Byte
;
; EXIT:
;	#None#
;
; MODFIES:
;	#None#
;
@debug_byte:
		push	af
		push	hl

		ld	h,a
		rrca
		rrca
		rrca
		rrca
		HEXTOASCII
		ld	(byte+1),a
		ld	a, h
		HEXTOASCII
		ld	(byte+2),a

		ld	hl, byte		
		call	debug_string

		pop	hl
		pop	af
		ret

; FUNCTION:	debug_word
;	 Outputs a word to the debug stream(s). The format is:@ $xxxx\r\n.
;
; ENTRY:
;	HL - Word
;
; EXIT:
;	#None#
;
; MODFIES:
;	#None#
;
@debug_word:
		push	af
		push	bc
		push	hl

		ld	a,h
		rrca
		rrca
		rrca
		rrca
		HEXTOASCII
		ld	(word+1),a
		ld	a, h
		HEXTOASCII
		ld	(word+2),a
		ld	a,l
		rrca
		rrca
		rrca
		rrca
		HEXTOASCII
		ld	(word+3),a
		ld	a, l
		HEXTOASCII
		ld	(word+4),a
			
		ld	hl, word	
		call	debug_string

		pop	hl
		pop	bc		
		pop	af
		ret
		ret
		
; FUNCTION:	debug_block
;	 Outputs a block of bytes to the debug stream(s). The format is:@ $xx,$xx,$xx...,$xx\r\n.
;
; ENTRY:
;	HL - Address of block.
;	BC - Length of block in bytes.
;
; EXIT:
;	#None#
;
; MODFIES:
;	#None#
;
@debug_block:
		push	af
		push	bc
		push	hl
	
		ld	a, b
		or	c
		jr	z, .return		
		dec	bc
		ld	a, b
		or	c
		jr	z, .done
.loop:
		ld	a, (hl)
		rrca
		rrca
		rrca
		rrca
		HEXTOASCII
		ld	(block+1),a
		ld	a, (hl)
		HEXTOASCII
		ld	(block+2),a

		push	hl
		ld	hl, block
		call	debug_string
		pop	hl
				
		inc	hl
		dec	bc
		ld	a, b
		or	c
		jp	nz, .loop
.done:
		ld	a, (hl)
		call	debug_byte		
.return:
		pop	hl
		pop	bc
		pop	af
		ret
	
byte:		db	"$xx\r\n",0	
word:		db	"$xxxx\r\n",0
block:		db	"$xx,",0
		
		IFDEF	INCLUDE_DEBUG_BUFFER_SIZE
fcb:		db	0
		db	"DEBUG   TXT"
		ds	SIZEOF_FCB-12,0
		
buffer_pos:	dw	buffer
buffer_left:	dw	INCLUDE_DEBUG_BUFFER_SIZE
buffer:		ds	INCLUDE_DEBUG_BUFFER_SIZE,0
		ENDIF
		
		MODULE

		
