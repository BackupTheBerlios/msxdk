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

		MODULE	CHIPS		
		
; FILE: chips
;	The chips module contains routines related to detecting and
;	manipulating chips like the R800, MSX Audio, MSX Music and
;	SCC chips.
;	:
;	You need to specifically turn on support for each chip that
;	the <chips> module includes. See below for a list of DEFINES
;	and a description for what chips they are. Any routines in the
;	<chips> module that have anything to do with a certain chip
;	won't be assembled in if support for that chip isn't specifically
;	set through the corresponding define.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
;	Thanks go to Remco Schrijver for the MSX-Music/MSX-Audio detection code and to
; 	Keiichi Kuroda & Masarun for the SCC detection code.

; DEFINE: INCLUDE_CHIPS_R800
;	Include support for the R800 CPU as found inside Turbo-R machines.
;
; DEFINE: INCLUDE_CHIPS_SCC
;	Include support for Konami's SCC sound chip.
;
; DEFINE: INCLUDE_CHIPS_MSXAUDIO
;	Include support for the MSX Audio chip, as found in the Philips Music 
;	Module (NMS-1205).
;
; DEFINE: INCLUDE_CHIPS_MSXMUSIC
;	Include support for the MSX Music chip, as found in the FM-PAC.
;	
; DEFINE: INCLUDE_CHIPS_MOONSOUND
;	Include support for the MoonSound.
;

;****if* chips/init_chips
;
; FUNCTION:	init_chips
;	Initialises the chips module.
;
; ENTRY:
;	None
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	EI -
;
; MODIFIES:
;	#ALL#
;
@init_chips:
		IFDEF	INCLUDE_CHIPS_R800
		; Detect whether the cpu is an R800 and if yes what mode
		; it's currently running in.
r800_detection:
		ld	a,(EXPTBL)
		ld	hl, ROMID
		call	RDSLT
		cp	ROMID_TURBOR
		jr	c,.done
		ld	a,(chips)
		or	1<<CHIP_R800
		ld	(chips),a
		ROMCALL GETCPU
		ld	(cpumode),a
		cp	a, Z80_MODE
		LDZ	a, R800_ROM_MODE
		ld	(r800_mode),a
.done:
		ENDIF

		IFDEF	INCLUDE_CHIPS_MSXMUSIC
		DEFINE	INCLUDE_CHIPS_SLOT_SEARCH
		ELSE
		IFDEF	INCLUDE_CHIPS_SCC
		DEFINE	INCLUDE_CHIPS_SLOT_SEARCH
		ENDIF
		ENDIF

		IFDEF	INCLUDE_CHIPS_SLOT_SEARCH
		; Detect MSX-Music (for example FM-PAC)
		; Detect SCC
		di
		ld	a, $00			; Not expanded, PS = 0, SS = 0
		ld	hl, EXPTBL		; Expanded table
sub_slot_loop:
		or	(hl)			; Set the Expanded bit in the Slot ID if the
						; current slot is expanded.
		push	hl
		push	af
		
		IFDEF	INCLUDE_CHIPS_SCC
		; SCC detection
scc_detection:
		ld	a,(scc_slotid)
		or	a
		jr	nz, .done
		pop	af
		push	af
		ld	h, $80			; Enable slot PS,SS at page 2
		CALL	ENASLT			;
		ld 	hl,$9000
		ld 	de,$9800
		ld 	c,(hl)
		ld 	(hl),$3f
		ex 	de,hl
		ld 	a,(hl)
		cpl 
		ld 	(hl),a
		cp 	(hl)
		cpl 
		ld 	(hl),a
		ex 	de,hl
		jr 	nz,.not_an_scc
		
		ld 	(hl),0
		ex 	de,hl
		ld 	a,(hl)
		cpl 
		ld 	(hl),a
		cp 	(hl)
		cpl 
		ld 	(hl),a
		ex 	de,hl
		jr 	z,.not_an_scc

		;ld	a, (chips)		; SCC detected
		;or	1<<CHIP_SCC		;
		;ld	(chips),a		;		
		pop	af			;
		push	af			;
		ld	(scc_slotid),a		;	
.not_an_scc:
		ld 	(hl),c
.done: 
		ENDIF
		
		IFDEF	INCLUDE_CHIPS_MSXMUSIC
msx_music_detection:
		ld	a, (msxmusic_slotid)
		or	a
		jr	nz,.done
		pop	af
		push	af		
		ld	hl, SLTATR + 1		; Attributes of page 1 for PS = 0, SS = 0
		ld	b, a
		and	$0c
		add	a, l			; SLTATR + $00,$04,$08 or $0C will not result
		ld	l, a			; in an increase in the high byte of the address
		ld	a, b
		and	$03
		add	a,a
		add	a,a
		add	a,a
		add	a,a
		ADD_HL_A			; HL now points to the attribute byte of page 1 for the current PS,SS
		ld	a, (hl)			; Does this subslot contain a ROM with a Statement Handler in page 1?
		jr	z,.done			; No, then there can't be an MSX Music cartridge in this subslot.
		pop	af
		push	af
		ld	h, $40			; Enable slot PS,SS at page 1
		CALL	ENASLT			;
		ld	hl, MSXMUSIC_STRING_OPLL; The address where the string "OPLL" is stored in the MSX-Music module
		ld	de, string_opll		; String "OPLL"
		ld	c, $04			; Compare 4 bytes
.compare_loop:
		ld	a, (de)
		cp	(hl)
		jr	nz, .done		; Bytes don't match
		inc	de
		inc	hl
		dec	c			; Still more bytes to compare?
		jr	nz,.compare_loop	; Yes
		; MSX-Music detected
		ld	a, (MSXMUSIC_INITIALISE); Initialise the MSX-Music module
		or	$1			; (or is this just activating it, does anybody know?)
		ld	(MSXMUSIC_INITIALISE),a	;
		
		;ld	a, (chips)		; MSX-Music detected
		;or	1<<CHIP_MSXMUSIC	;
		;ld	(chips),a		;		
		pop	af			;
		push	af			;
		ld	(msxmusic_slotid),a	;
.done		
		ENDIF

		pop	af
		pop	hl
		or	a
		jp	p, not_expanded
		; Expanded
		add	a, $04			; Next subslot (SS = SS + 1)
		bit	4, a			; Still a subslot left in this slot?
		jr	z, sub_slot_next	; Yes
		inc	a			; No, so go to next slot (PS = PS + 1)
		and	$03			; Clear bit 4 and set subslot zero (SS = 0)		
		inc	hl			; Next entry in the EXPTBL
		jp	sub_slot_next
not_expanded:
		inc	a			; Next slot (PS = PS + 1)
		inc	hl			; Next entry in the EXPTBL
sub_slot_next:
		and	$0f
		jp	nz, sub_slot_loop
		
		IFDEF	INCLUDE_CHIPS_SCC
		call	get_ramslotid
		ld	h, $80
		call	ENASLT
		ENDIF

		IFDEF	INCLUDE_CHIPS_MSXMUSIC
		call	get_ramslotid
		ld	h, $40
		call	ENASLT
		ENDIF
		
		ENDIF

		IFDEF	INCLUDE_CHIPS_MSXAUDIO
		; Detect MSX-Audio (for example NMS-1205)
msx_audio_detection:		
		in    	a,(MSXAUDIOPORTREG)	; Read a byte from the MSX Audio port
		cp    	$ff			; If an MSX Audio is present this will never return a $ff
		jr	z, .done		; So, if a $ff is returned this means no MSX Audio present.
		ld	a, (chips)		; MSX Audio found
		or	1<<CHIP_MSXAUDIO	;
		ld	(chips), a		;
.done:
		ENDIF

		IFDEF	INCLUDE_CHIPS_MOONSOUND
		; Detect MoonSound
moonsound_detection:
		in	a,(OPL4_STATUS)		; Read a byte from opl4 status port
		inc	a                       ; If an MoonSound is present this will never return $ff
		jr	z, .done		; So, if a $ff is returned this means no MoonSound present.
		ld	a, (chips)		; MoonSound found
		or	1<<CHIP_MOONSOUND	;
		ld	(chips),a		;
.done:
		ENDIF

.return:
		ei
		xor	a
		ret

string_opll:	db	"OPLL"

;****if* chips/kill_chips
;
; FUNCTION:	kill_chips
;	Kills the chips module.
;
; ENTRY:
;	#None#
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
@kill_chips:
		ei
		ld	a,(chips)
		and	1<<CHIP_R800
		ret	z

		ld	a,(cpumode)
		ROMCALL	CHGCPU	; Return to the original CPU setting. CHGPU does an EI as well
		ret

; FUNCTION:	get_cpu_mode
; 	Gets the current CPU mode. Possible modes are:@
;	- Z80_MODE
;	- R800_ROM_MODE
;	- R800_DRAM_MODE
;
; ENTRY:
;	None
;
; EXIT:
;	A - CPU Mode
;	EI -
;
; MODIFIES:
;	#ALL#
;
		IFDEF	INCLUDE_CHIPS_R800
@get_cpu_mode:	EXPORT	get_cpu_mode
		ei
		ld	a,(chips)
		and	1<<CHIP_R800
		LDZ	a,Z80_MODE
		ret	z
		ROMCALL	GETCPU
		ei
		ret
		ENDIF
		
		
; FUNCTION:	set_cpu_mode
; 	Sets the current CPU mode. Possible modes are:@
;	- Z80_MODE
;	- R800_ROM_MODE
;	- R800_DRAM_MODE
;	Note that R800_DRAM_MODE /should/ only be set if it was returned by a
;	previous call to get_cpu_mode. To explicitly turn the R800 mode on
;	or off use the calls <turn_r800_on> and <turn_r800_off>.
;
; ENTRY:
;	A - CPU mode
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
		IFDEF	INCLUDE_CHIPS_R800
@set_cpu_mode:	EXPORT	set_cpu_mode
		ei
		ld	b,a
		ld	a,(chips)
		and	1<<CHIP_R800
		ret	z

		ld	a,b
		ROMCALL CHGCPU
		ei
		ret
		ENDIF

; FUNCTION:	turn_r800_off
; 	Turns the R800 mode off, switching to Z80 mode.
;
; ENTRY:
;	#None#
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
		IFDEF	INCLUDE_CHIPS_R800
@turn_r800_off:	EXPORT	turn_r800_off
		ld	a, Z80_MODE
		jr	set_cpu_mode
		ENDIF

; FUNCTION:	turn_r800_on
; 	Turns the R800 mode on. If the CPU was in R800 DRAM mode at the	time
;	the chips module was initialised then this routine will set R800 DRAM
;	mode, otherwise it will set R800 ROM mode.
;
; ENTRY:
;	#None#
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
		IFDEF	INCLUDE_CHIPS_R800
@turn_r800_on:	EXPORT	turn_r800_on
		ld	a, (r800_mode)
		jr	set_cpu_mode
		ENDIF

; FUNCTION:	get_chip_info
;	Returns information about the presence and location of a given chip.
;	Note: if you have not explicitly included support for the given
;	chip then you'll always get a not present/detected return value for
;	that chip. See the <chips> introduction for more information about
;	this.
;
; ENTRY:
;	A - Chip identifier
;
; EXIT:
;	The output depends on the value given for the #Entry A#.
;	:
;	CHIP_R800:@
;	A - Presence flag (0 = R800 not present)
;	:
;	CHIP_SCC:@
;	A - Slot ID ($00 = no SCC detected)
;	:
;	CHIP_MSXMUSIC:@
;	A - Slot ID ($00 = no MSX Music detected)
;	:
;	CHIP_MSXAUDIO:@
;	A - Presence flag (0 = no MSX Audio detected)
;	:
;	If any other #Entry A# was given:@
;	A - $00
;
; MODIFIES:
;	#F#
@get_chip_info:
		cp	CHIP_UNKNOWN+1
		LDNC	a, CHIP_UNKNOWN
		
		add	a,a
		ld	(.jump_relative), a
.jump_relative:	equ	$+1
		jr	.jump_relative
		
@CHIP_R800:	equ	0
		EXPORT	CHIP_R800
		jr	get_chip_info_r800
		
@CHIP_SCC:	equ	1
		EXPORT	CHIP_SCC
		jr	get_chip_info_scc
		
@CHIP_MSXAUDIO:	equ	2
		EXPORT	CHIP_MSXAUDIO
		jr	get_chip_info_msxaudio
		
@CHIP_MSXMUSIC:	equ	3
		EXPORT	CHIP_MSXMUSIC
		jr	get_chip_info_msxmusic

@CHIP_MOONSOUND:	equ	4
		EXPORT	CHIP_MOOONSOUND
		jr	get_chip_info_moonsound
				
CHIP_UNKNOWN:	equ	5
		jr	get_chip_info_unknown			
				
get_chip_info_r800:
		ld	a, (chips)
		and	1<<CHIP_R800
		ret
get_chip_info_scc:
		ld	a, (scc_slotid)
		ret
get_chip_info_msxaudio:
		ld	a, (chips)
		and	1<<CHIP_MSXAUDIO
		ret
get_chip_info_msxmusic:	
		ld	a, (msxmusic_slotid)	
		ret
get_chip_info_moonsound:
		ld	a, (chips)
		and	1<<CHIP_MOONSOUND
		ret		
get_chip_info_unknown:
		xor	a
		ret		
		
		
;=================================================================================================
;
		IFDEF	INCLUDE_CHIPS_R800
cpumode:	db	0			; The cpu mode at the time init_cpu was called,
						; NOT the CURRENT mode.
r800_mode:	db	0			; The prefered R800 mode.
		ENDIF
		
chips:			db	0		; Detected-flags
scc_slotid:		db	$00		; Slot ID where the first SCC was found
msxmusic_slotid:	db	$00		; Slot ID where the first MSX Music was found
;
;=================================================================================================

		MODULE
