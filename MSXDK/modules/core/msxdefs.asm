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

; FILE:	msxdefs
;	This module contains lots of constants for various MSX specific
;	things like BIOS- and BDOS calls, addresses in the BIOS workspace
;	and I/O Ports.
;
;	It would go too far, and be pretty pointless, to describe every
;	constant that's declared in msxdefs. Just open the msxdefs.asm
;	some time and have a look at what's in there and maybe even add
;	some entries.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.

	MACRO	EXPDEF	DEFNAME, DEFVAL
DEFNAME		equ	DEFVAL
	ENDM
	
	; Available under DOS only
	EXPDEF	RESET			,$0000
	EXPDEF	STARTUP			,$0000

	; BIOS calls available under DOS as well
	EXPDEF	RDSLT			,$000c
	EXPDEF	WRSLT			,$0014
	EXPDEF	CALSLT			,$001c
	EXPDEF	ENASLT			,$0024

	; BIOS calls NOT available under DOS.
	; That is, you have to issue a CALSLT to call these
	EXPDEF	CHKRAM			,$0000
	EXPDEF	ROMID			,$002d
	EXPDEF	ROMID_MSX2		,1
	EXPDEF	ROMID_MSX2PLUS		,2
	EXPDEF	ROMID_TURBOR		,3
	EXPDEF	CHGMOD			,$005f
	EXPDEF	NMI			,$0066
	EXPDEF	EXTROM			,$015f
	EXPDEF	CHGCPU			,$0180
	EXPDEF	GETCPU			,$0183
	EXPDEF	Z80_MODE		,0
	EXPDEF	R800_ROM_MODE		,1
	EXPDEF	R800_DRAM_MODE		,2
	
	; SUBROM BIOS calls NOT available under DOS.
	; That is, you have to issue a CALSLT to call these
	EXPDEF	CHGMDP			,$01b5

	; DOS workspace area
	EXPDEF	RAM_P0			,$f341
	EXPDEF	RAM_P1			,$f342
	EXPDEF	RAM_P2			,$f343
	EXPDEF	RAM_P3			,$f344
	
	; BIOS workspace area
	EXPDEF	SX			,$f562
	EXPDEF	SY			,$f564
	EXPDEF	SPAGE			,$f565
	EXPDEF	DX			,$f566
	EXPDEF	DY			,$f568
	EXPDEF	DPAGE			,$f569
	EXPDEF	NX			,$f56a
	EXPDEF	NY			,$f56c
	EXPDEF	VCMCOL			,$f56e
	EXPDEF	VCMARG			,$f56f
	EXPDEF	LOGOP			,$f570	
	EXPDEF	VOICAQ			,$f975
	EXPDEF	VOICBQ			,$f9f5
	EXPDEF	VOICCQ			,$fa75
	EXPDEF	EXBRSA			,$faf8
	EXPDEF	MODE			,$fafc
	EXPDEF	MODE_VRAMSIZE_128K	,%00000100
	EXPDEF	LSTMOD			,$fb20
	EXPDEF	VCBA			,$fb41
	EXPDEF	VCBB			,$fb66
	EXPDEF	VCBC			,$fb8b
	EXPDEF	EXPTBL			,$fcc1
	EXPDEF	SLTATR			,$fcc9
	EXPDEF	SLTATR_STATEMENT_HANDLER ,%00100000
	EXPDEF	HNMI			,$fdd6
	EXPDEF	EXTBIO			,$ffca
	EXPDEF	HKEYI			,$fd9a
	EXPDEF	HTIMI			,$fd9f

	; BDOS calls
	EXPDEF	BDOS			,$0005
	EXPDEF	BDOS_CONSOLEOUTPUT	,$02
	EXPDEF	BDOS_STRINGOUTPUT	,$09
	EXPDEF	BDOS_OPENFILE		,$0f
	EXPDEF	BDOS_CLOSEFILE		,$10
	EXPDEF	BDOS_CREATEFILE		,$16
	EXPDEF	BDOS_SETDTA		,$1a
	EXPDEF	BDOS_RANDOMBLOCKWRITE	,$26
	EXPDEF	BDOS_RANDOMRECORDREAD	,$27

	; DOS2 Mapper calls
	EXPDEF	DOSMAPPER_ALL_SEG 	,#00
	EXPDEF	DOSMAPPER_FRE_SEG	,#03     
	EXPDEF	DOSMAPPER_RD_SEG 	,#06     
	EXPDEF	DOSMAPPER_WR_SEG 	,#09     
	EXPDEF	DOSMAPPER_CAL_SEG 	,#0C     
	EXPDEF	DOSMAPPER_CALLS 	,#0f     
	EXPDEF	DOSMAPPER_PUT_PH 	,#12    
	EXPDEF	DOSMAPPER_GET_PH 	,#15    
	EXPDEF	DOSMAPPER_PUT_P0 	,#18    
	EXPDEF	DOSMAPPER_GET_P0 	,#1b    
	EXPDEF	DOSMAPPER_PUT_P1 	,#1e    
	EXPDEF	DOSMAPPER_GET_P1 	,#21    
	EXPDEF	DOSMAPPER_PUT_P2 	,#24    
	EXPDEF	DOSMAPPER_GET_P2 	,#27    
	EXPDEF	DOSMAPPER_PUT_P3 	,#2a    
	EXPDEF	DOSMAPPER_GET_P3 	,#2d    

	; FCB structure
	EXPDEF	FCB_DRIVENAME		,#00
	EXPDEF	FCB_FILENAME		,#01
	EXPDEF	FCB_EXTENSION		,#09
	EXPDEF	FCB_CURRENTBLOCK	,#0C
	EXPDEF	FCB_RECORDSIZE		,#0E
	EXPDEF	FCB_FILESIZE		,#10
	EXPDEF	FCB_CURRENTRECORD	,#20
	EXPDEF	FCB_RANDOMRECORD	,#21
	EXPDEF	SIZEOF_FCB		,#25

	; I/O ports
	EXPDEF	DEBUGDEVCMD		,$2e
	EXPDEF	DEBUGDEVDATA		,$2f
	EXPDEF	PPIPRB			,$a9
	EXPDEF	PPIPRC			,$aa
	EXPDEF	MSXAUDIOPORTSTATUS	,$c0
	EXPDEF	MSXAUDIOPORTREG		,$c0
	EXPDEF	MSXAUDIOPORTDATA	,$c1
	EXPDEF	OPL4_WAVE_REG		,$7e
	EXPDEF	OPL4_WAVE_DATA		,$7f
	EXPDEF	OPL4_FM1_REG		,$c4
	EXPDEF	OPL4_FM1_DATA		,$c5
	EXPDEF	OPL4_FM2_REG		,$c4
	EXPDEF	OPL4_FM2_DATA		,$c5
	EXPDEF	OPL4_STATUS		,$c4
	EXPDEF	PSGREG			,$a0
	EXPDEF	PSGWDT			,$a1
	EXPDEF	PSGRDT			,$a2

	; Keyboard matrix. #RRMM, RR = Row, MM = Mask
	EXPDEF	KEY_ESCAPE		,#0704
	EXPDEF	KEY_UP			,#0820
	EXPDEF	KEY_DOWN		,#0840
	EXPDEF	KEY_LEFT		,#0810
	EXPDEF	KEY_RIGHT		,#0880
	EXPDEF	KEY_SPACE		,#0801
	
	EXPDEF	MSXMUSIC_STRING_OPLL	,$401c
	EXPDEF	MSXMUSIC_INITIALISE	,$7ff6

	; VDP Command
	EXPDEF	VCMD_SX			,#00
	EXPDEF	VCMD_SY			,#02
	EXPDEF	VCMD_SPAGE		,#03
	EXPDEF	VCMD_DX			,#04
	EXPDEF	VCMD_DY			,#06
	EXPDEF	VCMD_DPAGE		,#07
	EXPDEF	VCMD_NX			,#08
	EXPDEF	VCMD_NY			,#0a
	EXPDEF	VCMD_COL		,#0c
	EXPDEF	VCMD_ARG		,#0d
	EXPDEF	VCMD_LOGOP		,#0e
	
