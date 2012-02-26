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

		MODULE	environment
		
; FILE:	environment
;	The environment module contains routines that have to do with the
;	environment under which the program is running. The most important part
;	of this module is in its initialisation routine: it makes sure that
;	the entire 64K address space is mapped in at the primary RAM mapper,
;	that the stack starts at the top of the TPA and that the BDOS hook at
;	$0005 is present. It also checks whether the current machine satisfies
;	the minimum system type requirement, which you can control with the
;	<MINROMID> define and the minimum TPA requirement, which you can control
;	with the <MINTPA> define.
;
;	In its kill code an eventual error string is printed
;	before either returning to DOS or resetting.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
;	Many thanks to Arjan Bakker for providing the basis for many of the routines 
;	in this module.	The call_subrom routine was taken from an article in MCM 48.

; DEFINE:	MINROMID
;	Define this to the minimum value of the ROM ID that your program
;	requires. Possible values are:@
;	- ROMID_MSX2
;	- ROMID_MSX2PLUS
;	- ROMID_TURBOR
;
;	If MINROMID is not defined then the environment module will define it to
;	the default value ROMID_MSX2.
;
		IFNDEF	MINROMID
		DEFINE	MINROMID	ROMID_MSX2
		ENDIF
		
; DEFINE:	MINTPA
;	Define this to the minimum TPA value. If MINTPA is not defined then
;	the environment module will define it to the default value $d400.
;
		IFNDEF	MINTPA
		DEFINE	MINTPA		$d400
		ENDIF
				
;****if* environment/init_environment
;
; FUNCTION:	init_environment
;	Initialises the environment module.
;	:
;	There are two basic ways in which our .COM could have been loaded:
; 	- at boot time: either replace the MSXDOS.SYS or MSXDOS2.SYS file with
;	  your PROGRAM.COM or in sector 0 of a *DOS1* disk just replace the text 
;	  "MSDOS   SYS" with "PROGRAM COM". Replace PROGRAM by your program's real
;	  name of course.
; 	- from within DOS: either from the command line or from a .BAT file
;	:
;	There are some differences between being loaded at boot time or from
;	within DOS. When loaded at boot time:
; 	- the stack isn't properly setup (it doesn't start at the top of TPA)
; 	- page 1 ($4000-$7fff) doesn't contain RAM, but the DISK ROM is mapped 
;	  in there
; 	- the warmboot and BDOS hooks, at addresses $0000-$0004 and $0005-$0009
;	  are not setup at all and just contain NOPs (zero bytes).
;	:
;	There are also some differences between being loaded on DOS1 or DOS2, but
;	first let's specify what being loaded on DOS1 or DOS2 means. When the
;	program is started from the command line or from a .BAT file the "loaded
;	on DOSx" is of course the DOS version of the DOS that is giving you that
;	command line or that is executing the .BAT file. However, if the program 
;	is loaded at boot time then the following table applies:
;
;:      Way program is loaded | DOS2 ROM?  | Loaded on
;:      ----------------------------------------------
;:      Replaced MSXDOS.SYS   |   NO       | DOS1
;:      Replaced MSXDOS.SYS   |  YES       | DOS1
;:      Replaced MSXDOS2.SYS  |   NO       | ---- doesn't work -----
;:      Replaced MSXDOS2.SYS  |  YES       | DOS2
;
;	The first difference between being loaded on DOS1 and DOS2 is that under
;	DOS2 there is a memory mapper manager. This means you can't just directly
;	page RAM segments through ports $fc-$ff, but instead you have to use the memory
;	mapper manager. However, if you just use the <mapper> module you won't have
;	to worry about this.
;	:
; 	Another difference between running on DOS1 or DOS2 is that when running
;	on DOS1 on a Turbo-R with an external memory mapper that only pages
;	2 and 3 ($8000-$ffff) will be mapped into the internal memory. Pages
;	0 and 1 ($0000-$7fff) will be mapped into the external memory mapper.
; 	Note: when the program was loaded as boot time page 1 will of course be 
;	mapped to the DISK ROM again, see the previous bit on boot time/from within DOS
;	differences.
;	:
; 	Now, by calling init_environment at startup the environment will be
;	setup in a uniform way, indepedant of the way and the DOS on which our
;	.COM was loaded:
; 	- all 4 pages are mapped into the primary memory mapper
; 	- the stack starts at the top of TPA
; 	- the BDOS hook at $0005 is available/set
; 	Note: nothing is done about the warmboot hook.
;	:
;	If the system requirements don't match the <MINROMID> then an error
;	is returned, depending on the value of <MINROMID>:@
;
;	ROMID_MSX2 - "This program needs an MSX2 or better!"
;	ROMID_MSX2PLUS - "This program needs an MSX2+ or better!"
;	ROMID_TURBOR - "This program needs a Turbo-R (or better)!"
;	:
;	If the TPA is less than <MINTPA> then the following error is
;	returned:@
;	
;	"The TPA (Transient Program Area) is too small!".
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	EI -
;
; MODIFIES:
;	#ALL#
;******
;
@init_environment:		
		di
		in	a,($a8)
		ld	(slots+0),a
		ld	a,($ffff)
		cpl
		ld	(slots+1),a
		
		ld	a, ($0005)
		ld	(bdoshook),a		
		or	a			; If loaded from within DOS1 or DOS2
		jr	nz, .stack_bdoshook_done; then at least stack and BDOShook are already OK
		
		; Loaded at boot time from a DOS1 bootsector or as MSXDOS2.SYS from a DOS2 ROM.
		
		; Setup the stackpointer
		pop	hl			; Get the return address from the stack
		ld	sp,($f674)      	; Set stack pointer correctly
		push	hl			; Push the return address on the new stack
		
		; Setup the BDOShook
		ld	hl,$f37d		; System's version of the BDOS hook
		ld	de,$0005		; DOS's version of the BDOS hook
		ld	bc,3
		ldir                   		; Copy from system to DOS
		
.stack_bdoshook_done:
		; Copy the setupslots routine to $8000 and jump there.
		; We can't remap pages 0-1 into other slots when the
		; code doing that is in page 0 of course...
		ld	hl, _setupslots
		ld	de, setupslots
		ld	bc, setupslots_end - setupslots
		ldir

		ld	hl, (slots+0)
		call	setupslots
.return:
		in	a, ($a8)
		and	$3
		ld	hl, $fcc1
		ld	b, 0
		ld	c, a
		add	hl, bc
		or	(hl)
		jp	p, .slotid_ready
		ld	c, a
		ld	a, ($ffff)
		xor	$ff
		and	$03
		rlca
		rlca
		or	c
.slotid_ready:
		ld	(ramslotid), a
		
		ld	a,(EXPTBL+0)
		ld	hl, ROMID
		call	RDSLT
		cp	a, MINROMID
		ld	de, error_needbettermachine
		jr	c, .error

		ld	hl, 0
		add	hl, sp
		ld	de, MINTPA
		CP_HL_DE
		ld	de, error_tpatoosmall
		jr	c, .error

		xor	a		; No error
		ei
		ret		
.error
		ld	a, $ff
		ei
		ret
		

;****if* environment/setupslots
; FUNCTION:
;	Maps pages 0 and 1 into the same (sub)slot that page 2 and 3 are in. In
;	other words, maps all pages into the primary mapper. If page 0 was
;	indeed in another (sub)slot then its contents will be copied into the
;	new (sub)slot.
;
; ENTRY:
; 	H - Original sub slots
;     	L - Original main slots
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF, BC, DE, HL#
;******
;
_setupslots:
		PHASE	$8000
setupslots:
		push	hl
		ld	a,l
		and	$f0
		ld	c,a
		rrca
		rrca
		rrca
		rrca
		or	c
		ld	c,a
		out	($a8),a
		ld	a,h
		and	$f0
		ld	b,a
		rrca
		rrca
		rrca
		rrca
		or	b
		ld	b,a
		ld	($ffff),a
		pop	hl
		; Now all pages are in the primary memory mapper slot.
		
		; However, if page 0 WAS in another slot before then we
		; still need to copy its original contents to the new page 0.
		ld	a,l
		xor	c
		and	$03
		; We only check the mainslots (i.e. not the expanded subslots).
		; Theoratically speaking this might not be enough, but since
		; this "page 0 not being in the primary mapper" thing is only
		; occuring on a Turbo-R with an external memory mapper this
		; won't be an issue; this is because the primary mapper is an
		; internal mapper which means there's no way an external mapper
		; could be in the same main-slot as the primary mapper.
		jr	z, .continue_at_page0
		
		; Oh crap, have to copy page0 from the old slot to the new one.
		; H = old sub slots, L = old main slots
		; B = new sub slots, C = new main slots
		ex	de,hl
		ld	hl,$0000
.copyloop:
		push	de
		push	hl
		push	bc
		
		; Map in old page 0		
		ld	a,e
		out	($a8),a
		ld	a,d
		ld	($ffff),a
		; Copy data from page 0
		ld	de,$9000
		ld	bc,$1000
		ldir
		
		pop	hl
		pop	de
		push	de
		push	hl

		; Map in new page 0
		ld	a,l
		out	($a8),a
		ld	a,h
		ld	($ffff),a
		; Copy data to page 0
		ld	hl,$9000
		ld	bc,$1000
		ldir
		
		pop	bc
		pop	hl
		pop	de
		ld	a,h
		add	a,$10
		ld	h,a
		cp	$40
		jr	c, .copyloop
.continue_at_page0:
		ret
setupslots_end:
		DEPHASE
				
;****if* environment/kill_environment
;
; FUNCTION:	kill_environment
;	Prints the error-string if one was given. After that it depends on the
;	the way the program was loaded:
;	- at boot time: if an error occurred the computer just stalls so that 
;	  the user can read the printed error message. If no error occurred the 
;	  computer will just reset.
;	- from within DOS: returns to the DOS prompt.
;
; ENTRY:
;	DE - Address of error-string or $0000 if no error.
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#ALL#
;
@kill_environment:
		ld	(error_string), de
		ld	a, d
		or	e
		jr	z, .error_checked
		ld	c, BDOS_STRINGOUTPUT
		call	BDOS
.error_checked:
		ld	a, (bdoshook)		
		or	a
		jr	z, .loaded_at_boottime ; Loaded at boot time (dos1 bootsector or as MSXDOS2.SYS)
		
		; Loaded from within DOS1 or DOS2.
		di
		ld	hl, _restoreslots
		ld	de, restoreslots
		ld	bc, restoreslots_end - restoreslots
		ldir
		
		ld	hl, (slots+0)
		jp	restoreslots
		
.loaded_at_boottime:
		ld	de, (error_string)
		ld	a, d
		or	e
		jr	z, .reset
		di				; An error occurred, so we just want to keep the error message
		nop				; displayed.
		halt				; Just halt the CPU...
.reset:		
		ld	ix,RESET		; No error occurred, the user just wanted to leave the program.
		ld	iy,($fcc1 - 1)		; However, since there is "no where" to go,
		jp	CALSLT			; we'll just do a reset.
				

;****if* environment/restoreslots
; FUNCTION:
;	Set the slot selectors back to how they were when the program was first
;	started and then perform a warmboot, returning to DOS1 or DOS2.
;
; ENTRY:
; 	H - Original sub slots
;     	L - Original main slots
;
; EXIT:
;	Warmboot into DOS1 or DOS2
;
; MODIFIES:
;	N/A
;******
;
_restoreslots:
		PHASE	$8000		
restoreslots:
		ld	a,l			; Select the original main slots
		out	($a8),a			;
		ld	a,h			; Select the original sub slots (at least for the main slot
		ld	($ffff),a		; in which page 3 resides). Because, remember, writing to
						; $ffff only changes the subslot selection for the main slot
						; in which page 3 currently resides.
		jp	$0000			; Do a warmboot (return to DOS1/DOS2)
restoreslots_end:
		DEPHASE

; FUNCTION:	get_ramslotid
;	Returns a Slot ID for the slot in which the primary mapper resides.
;	In other words, the slot that contains the RAM used by our program.
;	Note that this routine is in page 0, so if you want to map in some
;	ROM in page 0 and then map our RAM back in, you'll have to call this
;	routine and save the Slot ID /before/ mapping in the ROM in page 0.
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Slot ID (ExxxSSPP)
;
; MODIFIES:
;	#None#
;
@get_ramslotid:
		ld	a, (ramslotid)
		ret
		
		
; FUNCTION:	call_subrom
; Call MSX2 subrom from MSXDOS. Should work with all versions of MSXDOS.
;
; Why this routine? Well, it is illegal to directly call the MSX2 subrom from 
; the MSXDOS environment, since the interslot call of some implementations of 
; MSXDOS can not handle the fact that the MSX2 subrom is in page 0.
;
; Note:@ NMI hook will be changed. This should pose no problem as NMI is
; not supported on the MSX at all.
;
; ENTRY:
;	IX - Address of routine in MSX2 subrom to call
;	AF, HL, DE, BC - Parameters for the routine
;
; EXIT:
;	AF, HL, DE, BC - Depending on the routine
;
; MODIFIES:
;	#IX, IY, AF', BC', DE', HL'#
;
@call_subrom:
		exx
		ex	af,af'       		; Store all registers
		ld	hl,EXTROM
		push	hl
		ld	hl,$c300
		push	hl           		; Push NOP ; JP EXTROM
		push	ix
		ld	hl,$21dd
		push	hl           		; Push LD IX,<entry>
		ld	hl,$3333
		push	hl           		; Push INC SP; INC SP
		ld	hl,0
		add	hl,sp        		; HL = offset of routine
		ld	a,$c3
		ld	(HNMI),a
		ld	(HNMI+1),hl 		; JP <routine> in NMI hook
		
		ex	af,af'
		exx               		; Restore all registers
		ROMCALL	NMI			; Call NMI-hook via NMI entry in ROMBIOS
						; NMI-hook will call SUBROM				
		exx
		ex	af,af'       		; Store all returned registers
		ld	hl,10
		add	hl,sp
		ld	sp,hl        		; Remove routine from stack
		ex	af,af'
		exx				; Restore all returned registers
		ret

;		
;=================================================================================================
;
error_needbettermachine:
		db	"This program needs "
		IF MINROMID = ROMID_TURBOR
		db	"a Turbo-R (or better)"
		ELSE
		IF MINROMID = ROMID_MSX2PLUS
		db	"an MSX2+ or better"
		ELSE
		db	"an MSX2 or better"
		ENDIF
		ENDIF
		db	"!\r\n$"

error_tpatoosmall:
		db	"The TPA (Transient Program Area) is too small!\r\n$"
;
;=================================================================================================
;
error_string:			dw 0
slots:				dw 0	; Offset 0: IN($A8); Offset 1: PEEK($FFFF) XOR $FF
bdoshook:			db 0
ramslotid:			db 0
;		
;=================================================================================================
		MODULE
