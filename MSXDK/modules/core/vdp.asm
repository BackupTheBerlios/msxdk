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

		MODULE	vdp
		
; FILE: vdp
;	The vdp module contains routines related to the VDP. If you use the
;	vdp module then you won't have to worry about switching back to
;	SCREEN 0 at the end of your program, since the vdp module's kill
;	routine already does that.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
;	Many thanks to Arjan Bakker for providing the basis for many of the routines
;	in this module.

; DEFINE: INCLUDE_VDP_128KVRAMCHECK
;	Define this if you want the <init_vdp> routine to check the amount
;	of VRAM as detected by the BIOS and generate an error if it isn't
;	128K.

;****if* vdp/init_vdp
;
; FUNCTION:	init_vdp
;	Initialises the vdp module. If INCLUDE_VDP_128KVRAMCHECK is defined
;	then the amount of VRAM as detected by the BIOS will be checked and an
;	error will be returned if less than 128K of VRAM was detected. Note
;	that the BIOS has encoded this information in bits 1 and 2 of the byte
;	at address $fafc. If for some reason these bits were altered after the
;	BIOS encoded the amount of detected VRAM then this check will ofcourse
;	incorrectly fail or succeed.
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
;******
;				
@init_vdp:	
		IFDEF	INCLUDE_VDP_128KVRAMCHECK
		ld	a, (MODE)
		and	a, MODE_VRAMSIZE_128K
		xor	MODE_VRAMSIZE_128K
		ld	de, error_need_128k_vram
		ELSE
		xor	a
		ENDIF
		ret

;****if* vdp/kill_vdp
;
; FUNCTION:	kill_vdp
;	Kills the vdp module. This restores all VDP registers to the values
;	stored in the BIOS workspace and then switches to SCREEN 0.
;
; ENTRY:
;	#None#
;
; EXIT:
;	DI -
;
; MODIFIES:
;	#ALL#
;
@kill_vdp:
		di
		; Restore VDP registers 0-7
		ld	hl,$F3DF
		ld	de,8 * 256 + 0
		call	.restore_vdpregs_loop
				
		; Restore VDP registers 8-23
		ld	hl,$FFE7 
		ld	de,16 * 256 + 8
		call	.restore_vdpregs_loop
		
		; Restore VDP registers 25-27
		ld	hl,$FFFA
		ld	de,3 * 256 + 25
		call	.restore_vdpregs_loop

		; Return to screen 0
		xor	a
		SUBCALL	CHGMDP
		ret
		
.restore_vdpregs_loop:
		ld	c,e
		ld	b,(hl)
		call	write_vdpreg
		inc	hl
		inc	e
		dec	d
		jr	nz,.restore_vdpregs_loop
		ret		

; FUNCTION:	set_vram_write
;	Prepare the VDP for writing to the given VRAM address.
;
; ENTRY:
;	BHL - Address (17bit)
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF#
;
@set_vram_write:  EXPORT	set_vram_write
		ld	a,h
		and	#c0
		or	b
		rlca
		rlca
		out	($99),a
		ld	a,$8e
		out	($99),a
		ld	a,l
		out	($99),a
		ld	a,h
		and	$3f
		or	$40
		out	($99),a
		ret

; FUNCTION:	set_vram_read
;	Prepare the VDP for reading from the given VRAM address.
;
; ENTRY:
;	BHL - Address (17bit)
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF#
;
@set_vram_read:  EXPORT	set_vram_read
		ld	a,h
		and	#c0
		or	b
		rlca
		rlca
		out	($99),a
		ld	a,$8e
		out	($99),a
		ld	a,l
		out	($99),a
		ld	a,h
		and	$3f
		out	($99),a
		ret

; FUNCTION:	ram_to_vram
;	Copy the RAM contents to VRAM. The VDP must already be in VRAM write
;	mode. This can be achieved by first calling <set_vram_write>.
;	
; ENTRY:
;	DI -
;	HL - Address of contents in RAM
;	DE - Size in bytes
;
; EXIT:
;	HL - #Entry HL# + #Entry DE#. In other words, points to right after the last
;	     byte of the copied contents.
;	B - 0
;	D - 0
;	VW - The VRAM write-address points to right after the last byte copied into
;	the VRAM.
;
; MODIFIES:
;	#AF, C, E#
;
@ram_to_vram:	EXPORT	ram_to_vram
		ld	c,$98			; Data port
		
		ld	b,e             	; Move LSB size to B
		
		ld	a,b
		or	a
		jr	z,.loop			; If LSB is zero then there is no need to increase MSB
		
		inc	d               	; Increase MSB

		; Since we're outputting 8 bytes per loop
		; we need to change the entry point in the loop
		neg				; Negate A, so for example 00000001 becomes 11111111
		add	a,a     		; Double it, (result: 11111110)
		and	15      		; Mask lower four bits (00001110)
		ld	(.jump_relative+1),a	; The jr will point to correct outi now
.jump_relative:	jr	.loop
.loop:		outi		         	; Output 8 bytes to VRAM
		outi
		outi
		outi
		outi
		outi
		outi
		outi
		jp	nz,.loop	  	; If b<>0 then output more data to vram
		dec	d               	; Decrease MSB
		jp	nz,.loop		; If d<>0 then output more data to vram
		ret

; FUNCTION:	vram_to_ram
;	Copy the VRAM contents to RAM. The VDP must already be in VRAM read
;	mode. This can be achieved by first calling <set_vram_read>.
;	
; ENTRY:
;	DI -
;	HL - Address of contents in RAM
;	DE - Size in bytes
;
; EXIT:
;	HL - #Entry HL# + #Entry DE#. In other words, points to right after the last
;	     byte of the read contents.
;	B - 0
;	D - 0
;	VR - The VRAM read-address points to right after the last byte copied from
;	the VRAM.
;
; MODIFIES:
;	#AF, C, E#
;
@vram_to_ram:	EXPORT	vram_to_ram
		ld	c,$98			; Data port
		
		ld	b,e             	; Move LSB size to B
		
		ld	a,b
		or	a
		jr	z,.loop			; If LSB is zero then there is no need to increase MSB
		
		inc	d               	; Increase MSB

		; Since we're inputting 8 bytes per loop
		; we need to change the entry point in the loop
		neg				; Negate A, so for example 00000001 becomes 11111111
		add	a,a     		; Double it, (result: 11111110)
		and	15      		; Mask lower four bits (00001110)
		ld	(.jump_relative+1),a	; The jr will point to correct ini now
.jump_relative:	jr	.loop
.loop:		ini		         	; Input 8 bytes from VRAM
		ini
		ini
		ini
		ini
		ini
		ini
		ini
		jp	nz,.loop	  	; If b<>0 then input more data from vram
		dec	d               	; Decrease MSB
		jp	nz,.loop		; If d<>0 then input more data from vram
		ret

; FUNCTION:	write_vdpreg
;	Write to a VDP register.
;
; ENTRY:
;	DI -
;	C - VDP register
;	B - Data
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF#
;
@write_vdpreg: 	EXPORT	write_vdpreg
		ld	a,b
		out	(#99),a
		ld	a,c
		or	128
		out	(#99),a
		ret

; FUNCTION:	vdp_command
;	Execute VDP-command (COPY, PSET etc.). Note that it first waits for any
;	previous command to finish. 
;
; ENTRY:
;	HL - Address of command-data.
;	S2 -
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF, BC, HL, V17, V32-V46#
;
@vdp_command:	EXPORT vdp_command
		WRITEVDP 17, 32			; Set the indirect register update register
		ld	c, $9b			; The port to use for indirect register updates
.wait:		in	a,($99)			; Is the VDP ready to accept a new command?
		rra				;
		jr	c,.wait			;
		outi				; 15 outi's
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		outi				;
		ret

; FUNCTION:	change_color
;	Change a color in the palette.
;
; ENTRY:
;	DI -
;	B - Color number
;	DE - RGB
;
; EXIT:
;	None
;
; MODIFIES:
;	#AF, C#
;
@change_color:	EXPORT	change_color
		ld      c,$99
		out     (c),b
		ld      a,$90
		out     (c),a
		inc     c
		out     (c),d
		out     (c),e
		ret

; FUNCTION:	change_palette
;	Change the entire palette.
;
; ENTRY:
;	DI -
;	HL - Address of palette-data
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF, BC, HL#
;
@change_palette:	EXPORT change_palette
		ld	c,$99
		xor     a
		out     (c),a
		ld      a,$90
		out     (c),a
		inc     c
		ld      b,$20
		otir
		ret		

; FUNCTION:	set_screen_mode
;	Sets the screen mode as specified by the given screen defintion.
;	This is basically just a 13 byte block containing the values to be
;	written to VDP registers 0-11 and 25.
;
;>The normal screen definitions for SCREEN 0-12:
;>
;>	db $00,$70,$00,$80,$01,$36,$07,$f4,$08,$00,$00,$00,$00	; SCREEN 0
;>	db $00,$60,$06,$80,$00,$36,$07,$07,$08,$00,$00,$00,$00	; SCREEN 1
;>	db $00,$60,$06,$ff,$03,$36,$07,$07,$08,$00,$00,$00,$00	; SCREEN 2
;>	db $00,$68,$02,$80,$00,$36,$07,$07,$08,$00,$00,$00,$00	; SCREEN 3
;>	db $04,$60,$06,$ff,$03,$3f,$07,$07,$08,$00,$00,$00,$00	; SCREEN 4
;>	db $06,$60,$1f,$ff,$01,$ef,$0f,$07,$08,$80,$00,$00,$00	; SCREEN 5
;>	db $08,$60,$1f,$ff,$01,$ef,$0f,$07,$08,$80,$00,$00,$00	; SCREEN 6
;>	db $0a,$60,$1f,$ff,$01,$f7,$1e,$07,$08,$80,$00,$01,$00	; SCREEN 7
;>	db $0e,$60,$1f,$ff,$01,$f7,$1e,$07,$08,$80,$00,$01,$00	; SCREEN 8
;>	db $0e,$60,$1f,$ff,$01,$f7,$1e,$07,$08,$80,$00,$01,$18	; SCREEN 10/11
;>	db $0e,$60,$1f,$ff,$01,$f7,$1e,$07,$08,$80,$00,$01,$08	; SCREEN 12
;
; ENTRY:
;	DI -
;	HL - Address of screen definition.
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF, BC, HL, V0-V11, V17, V25#
;
@set_screen_mode:	EXPORT	set_screen_mode
		WRITEVDP        17,0                
		ld      bc,12 * 256 + $9b
		otir
		ld	b, (hl)
		ld	c, 25
		call	write_vdpreg
		ret
				
;****f* vdp/clear_screen_page
; FUNCTION
;	Clear a screen page by filling it with zeros.
; ENTRY
;	A=Screen page number
; EXIT
;	EI
;	S0
; MODIFIES
;	AF,BC,HL,V17,V32-V46
;******
;
@clear_screen_page:	EXPORT	clear_screen_page
		di
		ld	(clear_screen_data + 3),a
		ld	(clear_screen_data + 7),a
		WRITEVDP 15,2
.wait:		in	a,(#99)
		rra
		jr	c,.wait
		WRITEVDP 15,0
		ld	hl,clear_screen_data
		WRITEVDP 17, 32
		ld	bc, $0f9b		
		otir
		ei
		ret
		
clear_screen_data:
		db	0,0,0,0,0,0,0,0,0,1,0,1,$00,0,%11000000    

error_need_128k_vram:
		db	"This program needs 128K VRAM!\r\n$"

		MODULE
