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

		MODULE  MAPPER          
; FILE: mapper
;       The mapper module provides an MSX-DOS version independant way of
;       manipulating the paging of RAM segments. It also checks whether there
;       is enough RAM present in the primary RAM mapper to run the program.
;
; NOTES:
;	Program segment numbers run from 0 to MEMSEGMENTS-1. At program start
;	the mapping of the segments looks like this:
;       
;>	Page | Address range | Program segment number
;>	---------------------------------------------
;>	  3  | $c000-$ffff   |  0
;>	  2  | $8000-$bfff   |  1
;>	  1  | $4000-$7fff   |  2
;>	  0  | $0000-$3fff   |  3
;       
;	Program segment number 3 contains the framework and the main routine.
;	Program segment number 0 contains the stack and the BIOS workspace and
;	the mapper-routines and -workspace. This way any code located at $4000 
;	or above is able to map a segment other than program segment number 3 
;	into page 0. You will probably want to disable the interrupts though ;)
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.
;	Many thanks to Arjan Bakker for providing the basis for many of the routines
;	in this module.
		
; DEFINE:	MEMSEGMENTS
;       Define this to the number of 16K RAM segments that the program needs.
;       Since this value does include the standard 4 segments available to
;       every DOS program the minimum value for MEMSEGMENTS is 4.
;               
		IFNDEF  MEMSEGMENTS
		DEFINE  MEMSEGMENTS     4
		ELSE
		IF      MEMSEGMENTS < 4
		"ERROR: MEMSEGMENTS MUST BE SET TO AT LEAST 4!!!"
		ENDIF
		ENDIF           

;****if* mapper/init_mapper
;
; FUNCTION:	init_mapper
;       Initialises the mapper module. If not enough RAM segments are
;       available an error will be returned. This error will be in the
;	form of:@
;	:
;	"This program needs at least xxxkB of free memory in the primary mapper!"
;	:
;	Where xxx will of course depend on <MEMSEGMENTS>.
;
; ENTRY:
;       #None#
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	EI -
;
; MODIFIES:
;       #ALL#
;
@init_mapper:
		ld      hl, _mapper_routines
		ld      de, mapper_routines
		ld      bc, mapper_routines_end - mapper_routines
		ldir
		call    detect_dos2mapper
		or      a
		jp      z, .nodos2mapper                                                
		
		; Copy the jump to the ALL_SEG routine
		ld      de, .jp_ALL_SEG
		ld      bc, #0003
		ldir
		
		ld      bc, DOSMAPPER_PUT_P0 - 3
		add     hl, bc
		ld      c, #03
		ld      de, put_page0 + 6
		ldir
		CALL_HL                         ; Get the current segment number for $0000-$3fff
		ld      (segments+3), a         ; Store it.

		ld      c, #03
		add     hl, bc          
		ld      de, put_page1 + 6
		ldir                            ; Partially overwrite WriteFD to use the DOS2 mapper routines
		CALL_HL                         ; Get the current segment number for $4000-$7fff
		ld      (segments+2), a         ; Store it.
		
		ld      c, #03
		add     hl, bc
		ld      de, put_page2 + 6
		ldir                            ; Partially overwrite WriteFE to use the DOS2 mapper routines
		CALL_HL                         ; Get the current segment number for $8000-$bfff
		ld      (segments+1), a         ; Store it.
		
		ld      c, #06
		add     hl, bc
		CALL_HL                         ; Get the current segment number for $c000-$ffff
		ld      (segments+0), a         ; Store it.
		
		; Further overwrite the put_page0/1/2 routines to use the DOS2 mapper routines
		; instead of writing to the hardware registers ourselves.
		ld      a, #cd  ; call
		ld      hl, translate_segment_to_dos2
		ld      ( put_page0 + 3),a
		ld      ( put_page0 + 4), hl
		ld      ( put_page1 + 3),a
		ld      ( put_page1 + 4), hl
		ld      ( put_page2 + 3),a
		ld      ( put_page2 + 4), hl
	
		IF MEMSEGMENTS > 4
		; Right, we already have 4 segments, now claim the remaining ones needed.
		ld      b, MEMSEGMENTS - 4
		ld      hl, segments + 4
.claimsegments:
		push    bc
		push    hl
		xor     a                       ; Claim a User segment, not a System segment
		ld      b, a                    ; Segment must be in primary mapper
		call    .jp_ALL_SEG             ; Allocate segment
		pop     hl
		pop     bc
		jr      c, .error               ; Failed to allocate another segment -> ERROR
		ld      (hl), a                 ; Store the DOS2 segment code in our segments table
		inc     hl
		djnz    .claimsegments  
		ENDIF

.return:                
		xor	a
		ei
		ret
		
.error:
		ld	a,$ff
		ld      de, error_notenoughmemory
		ei
		ret
		
.jp_ALL_SEG:
		ds      3
		
.nodos2mapper:
		; No DOS2 mapper manager is present, so segments will be mapped
		; by writing directly to ports $FC-$FF. Unfortunately this also
		; means the number of segments in the primary mapper has to be
		; determined by this module.     
		di      
		ld      hl, #ff00               ; Reserve 256 bytes on the stack
		add     hl, sp                  ;
		ld      sp, hl                  ;
		ld      (hl), $00               ; Fill these 256 bytes with $00
		push    hl                      ;
		pop     de                      ;
		inc     de                      ;
		ld      bc, 256-1               ;
		ldir                            ;
		ex      de, hl                  ;
		dec     h                       ; HL=Base of the 256 bytes block

		ld	b, 0			; 0 = 256                
		call    exchange_first_bytes    ; Save the first byte of all 256 (possible) segments into the block
		
		call    count_segments          ; Count the number of real segments (DE=count)

		ld 	b, e                	; Only restore as many segments as there are! Otherwise you'd get
						; funny results as during the first exchange of bytes some segments
						; already contained "replaced" values since they are duplicates
						; of segments that had already had their first byte replaced.
		call    exchange_first_bytes    ; Restore the first byte of all 256 (possible) segments from the block

		inc     h                       ; Release our 256 bytes block from the stack again
		ld      sp, hl                  ;
		
		ld      a, #01                  ; Map in segment 1 again into page 2 ($8000-$bfff)
		out     (#fe),a                 ;
		ld      hl, MEMSEGMENTS-1       ; Are there enough segments in the primary mapper?
		CP_HL_DE
		jr      nc, .error              ; Apparently not.
		jr      .return                 ; Yes, there are enough segments.

;****if* mapper/exchange_first_bytes
; FUNCTION:
;       Exchanges the first byte of every segment with a byte from the given
;       256 byte block.
; ENTRY:
;	B - Numbers of segments to process
;       HL=Address of 256 byte block.
; EXIT:
;       None
; MODIFIES:
;       AF,BC,P2
;******
;
exchange_first_bytes:
		push    hl                
.loop:
		ld      a, b                    ; Page segment into page 2
		out     ($fe),a                 ;
		ld      c, (hl)                 ; Exchange bytes
		ld      a, ($8000)              ;
		ld      (hl), a                 ;
		ld      a, c                    ;
		ld      ($8000),a               ;
		inc     hl
		djnz    .loop
		pop     hl
		ret
		
;****if* mapper/count_segments
; FUNCTION:
;       Count the number of segments. Note that this even correctly counts the
;       number of segments in a 4M mapper. This in contrast to some (mostly
;       older) software that only uses a byte for the segment count which then
;       of course totally fails when there are 256 segments... Anyway, there
;       are no such problems with this segment counter :)
; ENTRY:
;       None
; EXIT:
;       DE=Number of segments (0..256)
; MODIFIES:
;       AF,B,P2
;******
;
count_segments:
		push    hl
		ld      de, 0
		ld      b, 0
.loop:
		ld      a, b
		out     ($fe),a                 ; Select a segment
		ld      a, ($8000)              ; Read the first byte
		or      a                       ; Is it zero?
		jr      nz, .done               ; No, then this segment was already tested
		dec     a                       ; "New" segment
		ld      ($8000),a               ; Mark it as tested
		inc     de                      ; Increase segment count
		djnz    .loop   
.done:          
		pop     hl
		ret             

;****if* mapper/kill_mapper
;
; FUNCTION:	kill_mapper
;       Kills the mapper module.
;
; ENTRY:
;       #None#
;
; EXIT:
;       #None#
;
; MODIFIES:
;       #ALL#
;
@kill_mapper:
		ret

; FUNCTION:	detect_dos2mapper
;	Detects whether the DOS2 mapper manager is present or not.
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Mapper detected flag (0 = not detected)
;	HL - DOS2 mapper function pointer table if A<>0
;	DI -
;
; MODIFIES:
;	#ALL#
;
@detect_dos2mapper:	EXPORT	detect_dos2mapper
		di				; Since the call the EXTBIO (probably) disables interrupts
						; we have to make sure they're disabled as well if the call
						; to EXTBIO isn't made (but rather the ret nc is taken).
		ld	a,(LSTMOD)		; If bit 0 of LSTMOD is set then the EXTBIO hook is initialised
		rrca				; If no hook is present then DOS2 can not be present either
		sbc	a, a
		ret	z			; No DOS2 means no mapper either.

		xor	a			;
		ld	de,$0402        	; DOS2 memory mapper device number = 4                
						; Function "Get mapper function pointer table" = 2
		call	EXTBIO			; Leaves A unaltered (at 0) if no memory mapper present, 
						; sets A to <> 0 otherwise
		ret

; DEFINE:	MAPPER_WORKSPACE_BASE
;       Define this to the address where the mapper-routines and -workspace
;       should be stored. Define <MAPPER_WORKSPACE_SIZE> to the number of bytes
;       available at that address. An assembler error will be generated if the
;       number of bytes is not enough to store the mapper-routines and
;       -workspace at. An assembler error is also generated if 
;	MAPPER_WORKSPACE_BASE is defined to any address below $c000.
;       If MAPPER_WORKSPACE_BASE is not defined then it will be set to VOICAQ
;       with a MAPPER_WORKSPACE_SIZE of 128*3 bytes. This means you can not use
;       any PLAY commands from the BASIC ROM, which any sane person using this
;       framework wouldn't do anyway.
;
; DEFINE:	MAPPER_WORKSPACE_SIZE
;       See <MAPPER_WORKSPACE_BASE> for more information.
;
		IFNDEF          MAPPER_WORKSPACE_BASE
		DEFINE          MAPPER_WORKSPACE_BASE   VOICAQ
		DEFINE          MAPPER_WORKSPACE_SIZE   128*3
		ELSE
		IF		MAPPER_WORKSPACE_BASE < $c000
		ERROR "MAPPER_WORKSPACE_BASE < $c000!"
		ELSE
		IFNDEF          MAPPER_WORKSPACE_SIZE
		ERROR "MAPPER_WORKSPACE_BASE is defined, but MAPPER_WORKSPACE_SIZE is not!"             
		ENDIF
		ENDIF
		ENDIF                
_mapper_routines:
		TEXTAREA        MAPPER_WORKSPACE_BASE
mapper_routines:
; FUNCTION:	put_page0
;       Map the given segment in at page 0 ($0000-$3fff).
;
; ENTRY:
;       A - Program segment number
;
; EXIT:
;       #None#
;
; MODIFIES:
;       #AF#
;
@put_page0:     EXPORT  put_page0
		ld      (page0),a
		out     (#fc),a         ; The "out (#fc),a", ret and "defs 3" will be overwritten with
		ret                     ; "call translate_segment_to_dos2", "jp <DOSMAPPER_PUT_P0>"
		ds      3               ; when running on DOS2
; FUNCTION:	put_page1
;       Map the given segment in at page 1 ($4000-$7fff).
;
; ENTRY:
;       A - Program segment number
;
; EXIT:
;       #None#
;
; MODIFIES:
;       #AF#
;
@put_page1:     EXPORT  put_page1
		ld      (page1),a
		out     (#fd),a         ; The "out (#fd),a", ret and "defs 3" will be overwritten with
		ret                     ; "call translate_segment_to_dos2", "jp <DOSMAPPER_PUT_P1>"
		ds      3               ; when running on DOS2
; FUNCTION:	put_page2
;       Map the given segment in at page 2 ($8000-$bfff).
;
; ENTRY:
;       A - Program segment number
;
; EXIT:
;       #None#
;
; MODIFIES:
;       #AF#
;
@put_page2:     EXPORT  put_page2
		ld      (page2),a
		out     (#fe),a         ; The "out (#fe),a", ret and "defs 3" will be overwritten with
		ret                     ; "call translate_segment_to_dos2", "jp <DOSMAPPER_PUT_P2>"
		ds      3               ; when running on DOS2
		
; FUNCTION:	get_page0
;       Get the program segment number of the segment currently mapped in at
;       page 0 ($0000-$3fff).
;
; ENTRY:
;       #None#
;
; EXIT:
;       A - Program segment number
;
; MODIFIES:
;       #None#
;
@get_page0:     EXPORT  get_page0
		ld      a,(page0)
		ret
; FUNCTION:	get_page1
;       Get the program segment number of the segment currently mapped in at
;       page 1 ($4000-$7fff).
;
; ENTRY:
;       #None#
;
; EXIT:
;       A - Program segment number
;
; MODIFIES:
;       #None#
;
@get_page1:     EXPORT  get_page1
		ld      a,(page1)
		ret
; FUNCTION:	get_page2
;       Get the program segment number of the segment currently mapped in at
;       page 2 ($8000-$bfff).
;
; ENTRY:
;       #None#
;
; EXIT:
;       A - Program segment number
;
; MODIFIES:
;       #None#
;
@get_page2:     EXPORT  get_page2
		ld      a,(page2)
		ret
; FUNCTION:	get_page3
;       Get the program segment number of the segment currently mapped in at
;       page 3 ($c000-$ffff).
;
; ENTRY:
;       #None#
;
; EXIT:
;       A - Program segment number
;
; MODIFIES:
;       #None#
;
@get_page3:     EXPORT  get_page3
		ld      a,(page3)
		ret

;****if* mapper/translate_segment_to_dos2
; FUNCTION:
;       Translate a program segment number to a DOS2 segment number.
; ENTRY:
;       A - Program segment number
; EXIT:
;       A - DOS2 segment number
; MODIFIES:
;       #F#
;******
;
translate_segment_to_dos2:
		push    hl
		ld      hl, segments
		ADD_HL_A
		ld      h, (hl)                 
		ld      a, h
		pop     hl
		ret

page0:          db      3
page1:          db      2
page2:          db      1
page3:          db      0
;                               
segments:       ds      MEMSEGMENTS
;
mapper_routines_end:
		IF ( mapper_routines_end - mapper_routines) > (MAPPER_WORKSPACE_SIZE)
		ERROR "MEMSEGMENTS too high for the given MAPPER_WORKSPACE_BASE/SIZE!"
		ENDIF
		ENDT
;               
;=================================================================================================
;
error_notenoughmemory:
		db      "This program needs at least "
		IF MEMSEGMENTS > 62
		db      '0' + ((MEMSEGMENTS*16)/1000)
		ENDIF
		IF MEMSEGMENTS > 6
		db      '0' + (((MEMSEGMENTS*16)/100)%10)
		ENDIF
		db      '0' + (((MEMSEGMENTS*16)/10)%10)
		db      '0' + (((MEMSEGMENTS*16)/1)%10)
		db      "kB of free memory in the primary mapper!\r\n$"
;               
;=================================================================================================
;
;               
;=================================================================================================                              
		MODULE
