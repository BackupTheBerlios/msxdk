;	This module is Copyright (c) 2003 by Remco Schrijvers and has been released as freeware.

		MODULE	moonblaster14

MB14_AUDIO	equ	(1<<0)
MB14_MUSIC	equ	(1<<1)
		
; FILE: moonblaster14
;	The Moonblaster 1.4 module contains load and player code for
;	Moonblaster 1.4 samplekits and musics.
;
; FUNCTION NAMES:
;	Except for <init_moonblaster14> and <kill_moonblaster14> all functions
;	are MODULE local, which means you have to add the prefix "moonblaster14." to
;	their names. This is to prevent possible name clashes with functions
;	from other libraries you might be using. However, if <MAKE_MOONBLASTER14_GLOBAL>
;	is defined then all public functions will be available without the "moonblaster14."
;	prefix as well.
;
; DEPENDANCIES:
;	chips - The <chips> module must be included and the following
;	DEFINEs must be set; INCLUDE_CHIPS_R800, INCLUDE_CHIPS_MSXAUDIO, 
;	INCLUDE_CHIPS_MSXMUSIC.
;
; COPYRIGHT & CREDITS:
;	This module is Copyright (c) 2003 by Remco Schrijvers and has been released as freeware.
;	Thanks to Eli-Jean Leyssens for the documentation and adaptation of the original code 
;	into the MSXDK.

; DEFINE:	MAKE_MOONBLASTER14_GLOBAL
;	Defining this will make all public functions that are normally only
;	available with the "moonblaster14." prefix to also be available without
;	this prefix. See the introduction of <moonblaster14> for more information.

; FUNCTION:	init_moonblaster14
;	Initialises the Moonblaster 1.4 module. It's the responsibility of the
;	callee to determine whether MSX Audio and/or MSX Music is present and
;	to pass this information to this routine.
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
@init_moonblaster14:	EXPORT init_moonblaster14
		ld	c, 0
		ld	a, CHIP_MSXAUDIO
		call	get_chip_info
		or	a
		jr	z, .msxaudio_checked
		ld	c, MB14_AUDIO
.msxaudio_checked:		
		ld	a, CHIP_MSXMUSIC
		call	get_chip_info
		or	a
		ld	a, c
		jr	z, .msxmusic_checked
		or	MB14_MUSIC
.msxmusic_checked:
		ld	(mb14chips),a

		xor	a			; No error
		ret				
				
; FUNCTION:	kill_moonblaster14
;	Kills the Moonblaster 1.4 module.
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
@kill_moonblaster14:	EXPORT kill_moonblaster14
		ret

; FUNCTION:	start_samplekit_upload
;	Prepares the Moonblaster 1.4 module for the upload of a
;	sample kit.
;
; ENTRY:
;	HL - Address of 56 bytes containing the "sample addresses". These
;	   are simply the first 56 bytes from a .MBK file.
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF, BC, DE, HL#
;		
start_samplekit_upload:	EXPORT	moonblaster14.start_samplekit_upload
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@start_samplekit_upload:	EXPORT	start_samplekit_upload
		ENDIF
		ld	a, (mb14chips)		; Is an MSX Audio module present?
		and	MB14_AUDIO		;
		jr	z,.return		; No, then nothing to do
		ld	de, sample_addresses	; Copy the sample addresses to our internal table
		ld	bc, 56			;
		ldir				;
		ld	(upload_address), bc	; Set the sample kit address (in the MSX Audio module) to $00000
.return		
		ret
		
; FUNCTION:	upload_samplekit_chunk
;	Uploads another chunk of sample kit data to the MSX Audio module.
;
; ENTRY:
;	HL - Address of chunk data.
;	DE - Size of chunk in bytes. The lower 2 bits of this size are ignored!
;	     In other words, this should be a multiple of 4!
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
upload_samplekit_chunk:	EXPORT	moonblaster14.upload_samplekit_chunk
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@upload_samplekit_chunk:	EXPORT	upload_samplekit_chunk
		ENDIF
		ld	a, (mb14chips)		; Is an MSX Audio module present?
		and	MB14_AUDIO		;
		jr	z,.return		; No, then nothing to do
		push	hl
		push	de
		call	get_cpu_mode		; Get and store the CPU mode
		push	af
		call	turn_r800_off		; Turn the R800 off, otherwise the following OUT sequences 
						; will be too fast for the poor MSX Audio module.
		pop	af
		pop	de
		pop	hl
		push	af
		di
		ld	a, e			; Clear the lower 2 bits of the size
		and	%11111100		;
		ld	e, a			;
		push	de
		push	hl
		ld	hl, (upload_address)
		ld	a, l
		ld	( adpcm_upload_start_low), a
		ld	a, h
		ld	( adpcm_upload_start_high), a
		srl	d
		rr	e
		srl	d
		rr	e
		add	hl, de
		ld	(upload_address), hl
		dec	hl
		ld	a, l
		ld	( adpcm_upload_stop_low), a
		ld	a, h
		ld	( adpcm_upload_stop_high), a
		ld	b, 12
		call	_send_adpcm_command
		pop	hl
		pop	de
.copy_loop
		ld	a, $0f
		out	(MSXAUDIOPORTREG), a
		ld	a, (hl)
		inc	hl
		out	(MSXAUDIOPORTDATA), a
		ld	a, $04
		out	(MSXAUDIOPORTREG), a
		ld	a, $80
		out	(MSXAUDIOPORTDATA), a
.wait_loop:  	in	a, (MSXAUDIOPORTSTATUS)		; Get the ADPCM status
		bit	7, a				; Ready?
		jr	z, .wait_loop			; No, keep waiting for it to become ready.
		bit	4, a				; EOS?
		jr	nz, .copy_done			; Yes, then done. Normally this should only occur
							; after all the bytes we wanted to upload have been
							; uploaded. That is, the following dec de would make
							; de zero anyway. This really looks like a superfluous
							; check to me, but since I don't have enough techinical
							; documentation on the ADPCM chip I just can't tell.
		dec	de
		ld	a, d
		or	e
		jr	nz,.copy_loop
.copy_done:
		ld	b, 3
		call	_send_adpcm_command
		pop	af				; Restore the CPU mode
		call	set_cpu_mode			;
.return
		ei					; Ensure that in all cases interrupts are enabled
		ret

_send_adpcm_command:
		ld	hl, adpcm_command
.loop:
		ld	a, (hl)
		inc	hl
		out	(MSXAUDIOPORTREG), a
		ld	a, (hl)
		inc	hl
		out	(MSXAUDIOPORTDATA), a
		djnz	.loop
		ret
		
adpcm_command:
		db	$04,$78,$04,$80,$07,$01,$07,$60,$08,$00
		db	$09         	
adpcm_upload_start_low:		db 0
		db	$0a
adpcm_upload_start_high:	db 0
		db	$0b
adpcm_upload_stop_low:		db 0
		db	$0c
adpcm_upload_stop_high:		db 0
		db	$10,$f0
		db	$11,$00
		db	$04,$60

upload_address:		dw	0		
mb14chips:		db	0






;------- moonblaster playroutine -------
;

playing:	db	0 			;status:  0 = not playing
						;       255 = playing
musadr:		dw	0			; Address of music
pos:		db	0 			; Current position
step:		db	0 			; Current step
status:		db	0,0,0			; 3 Statusbytes
stepbf:		ds	13			;stepdata die in VOLGENDE interrupt
						;wordt gespeeld (of al eerder is af-
						;gespeeld)
		IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT		
;Let op: Om de huidige volumes van de kanalen uit te lezen kan de volgende
;        formule worden gebruikt:
;        LASPL1 + (kanaalnr-1) * 25 + 22  = msx-audio volume
;        LASPL1 + (kanaalnr-1) * 25 + 23  = msx-music volume
		ENDIF

; FUNCTION:	start_music
;	Prepares the Moonblaster 1.4 module for the playing of a music.
;	Note that the actual playing is done by the <music_interrupt> routine
;	which you should call at every VBlank interrupt (i.e. 50 or 60 times
;	per second).
;
; ENTRY:
;	HL - Address of music.
;
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
start_music:	EXPORT	moonblaster14.start_music
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@start_music:	EXPORT	start_music
		ENDIF
	di
	ld	a,(mb14chips)			; At least an MSX Audio or an MSX Music present?	
	or	a				;
	jp	z,.return			; No, then never start playing
	ld	a,(playing)			; Already playing?
	or	a				;
	jp	nz,.return			; Yes, then just keep playing.
	ld	(musadr),hl
	ld	hl,0
	ld	(status),hl
	ld	(status+1),hl
	xor	a
	ld	(spdcnt),a
	ld	(trbpsg),a
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	(fading),a
	ENDIF
	dec	a
	ld	(pos),a
	ld	(playing),a
	ld	hl,(musadr)
	ld	de,xleng
	ld	bc,207
	ldir
	ld	c,41
	add	hl,bc
	ld	c,128
	ldir
	ld	(xpos),hl
	ld	a,(xleng)
	inc	a
	ld	e,a
	ld	d,0
	add	hl,de
	ld	(patadr),hl

	ld	hl,pc2out
	ld	de,mm2out
	ld	bc,pc3out
	ld	ix,mm3out
	ld	a, CHIP_R800
	call	get_chip_info
	or	a
	jr	z, .strms2
	ld	a,03dh
	ld	(trbpsg),a
	ld	(trbpsg+1),a
;xxx	call	0183h
;	or	a
;	jr	z,.strms2
; just assume the worst. r800 might be in z80 mode at the moment
; but it could be turned on again
	ld	hl,parout
	ld	de,mmrout
	ld	bc,parout
	ld	ix,mmrout
.strms2:
	ld	(pacout+1),hl
	ld	(fpcout+1),bc
	ld	(mmout+1),de
	ld	(fmmout+1),ix
	ld	hl,chnwc1
	ld	de,chnwc1+1
	ld	bc,9
	ld	(hl),0
	ldir
	ld	a,(mb14chips)
	and	MB14_AUDIO
	call	nz,setaud
	ld	a,(mb14chips)
	and	MB14_MUSIC
	call	nz,setmus
	ld	a,(mb14chips)
	cp	MB14_AUDIO+MB14_MUSIC
	call	z,setste

	ld	b,9
	ld	iy,xbegvm+8
	ld	ix,xrever
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	hl,laspl1+200+6
	ld	de,-30	
	ELSE	
	ld	hl,laspl1+176+6
	ld	de,-27
	ENDIF
.mrlus:	ld	(hl),0
	inc	hl
	inc	hl
	inc	hl
	ld	a,(ix+0)
	ld	(hl),a
	inc	hl
	ld	a,(iy+0)
	ld	(hl),a
	inc	hl
	ld	a,(iy+9)
	ld	(hl),a
	inc	ix
	dec	iy
	add	hl,de
	djnz	.mrlus

	call	sinsmm
	call	sinspa
	ld	a,(xtempo)
	ld	(speed),a
	ld	a,15
	ld	(step),a
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	(maxpsg),a
	ENDIF	
	ld	a,48
	ld	(tpval),a
.return:
	ei
	ret

;--- Start MSX-Audio instruments

sinsmm:	ld	de,mmrgad
	ld	hl,xbegvm
	ld	iy,laspl1+20

	ld	b,9
sinsm4:	push	bc
	push	hl
	push	de
	ld	b,(hl)
	ld	hl,xmmvoc-9
	ld	de,9
sinsm3:	add	hl,de
	djnz	sinsm3
	pop	de

	push	hl
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	(iy+0),a
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	inc	hl
	ld	a,(hl)
	ld	(iy+2),a
	ld	bc,5
	add	hl,bc
	ld	a,(hl)
	ld	(iy+4),a	
	ENDIF	
	pop	hl

	ld	b,9
sinsm2:	ld	a,(de)
	ld	c,a
	ld	a,(hl)
	inc	de
	inc	hl
	call	mmout
	djnz	sinsm2
	pop	hl
	inc	hl
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	bc,25
	ELSE	
	ld	bc,22
	ENDIF
	add	iy,bc
	pop	bc
	djnz	sinsm4

	ld	b,5
	ld	hl,strreg
sinsm5:	ld	c,(hl)
	inc	hl
	ld	a,(hl)
	inc	hl
	call	fmmout
	djnz	sinsm5
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	a,255
	ld	(smpvlm),a
	ENDIF	
	ld	a,(xsust)
	and	011000000b
	ld	c,0bdh
	jp	fmmout



;--- Start MSX-Music instruments

sinspa:	di
	ld	hl,xbegvp
	ld	iy,laspl1+21
	ld	b,6
	ld	a,(xsust)
	bit	5,a
	push	af
	jr	nz,sinsp2
	ld	b,9
sinsp2:	ld	c,30h
sinspi:	push	bc
	push	hl
	ld	a,(hl)
	ld	hl,xpasti-2
	add	a,a
	ld	c,a
	ld	b,0
	add	hl,bc
	ld	a,(hl)
	cp	16
	call	nc,sinspo
	rlca
	rlca
	rlca
	rlca
	inc	hl
	ld	b,(hl)
	add	a,b
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	rlc	b
	rlc	b
	ld	(iy+2),b
	ld	bc,25	
	ELSE	
	ld	bc,22
	ENDIF
	add	iy,bc
	pop	hl
	pop	bc
	call	pacout
	inc	hl
	inc	c
	djnz	sinspi
	xor	a
	ld	c,0eh
	call	fpcout
	pop	af
	ret	z

	ld	de,xdrvol
	ld	hl,drmreg
	ld	b,9
setdrm:	ld	c,(hl)
	ld	a,(de)
	call	pacout
	inc	hl
	inc	de
	djnz	setdrm
	
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	a,(xdrvol)
	and	01111b
	rlca
	rlca
	ld	(laspl1+6*25+23),a
	ld	a,(xdrvol+1)
	and	011110000b
	rrca
	rrca
	ld	(laspl1+7*25+23),a
	ld	a,(xdrvol+2)
	and	01111b
	rlca
	rlca
	ld	(laspl1+8*25+23),a
	ENDIF		
	ret

sinspo:	push	hl
	sub	15
	ld	b,a
	ld	hl,xorgp1-8
	ld	de,8
sinpo2:	add	hl,de
	djnz	sinpo2
	push	hl
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	(iy+0),a
	pop	hl
	ld	bc,0800h
sinpo3:	ld	a,(hl)
	call	pacout
	inc	c
	inc	hl
	djnz	sinpo3
	pop	hl
	xor	a
	ret


setmus:	ld	a,(xsust)
	and	0100000b
	ld	a,2
	jr	z,setau2
	ld	hl,chnwc1
	ld	de,chnwc1+1
	ld	bc,5
	ld	(hl),a
	ldir
	ld	(chnwc1+9),a
	ret
setaud:	ld	a,1
setau2:	ld	hl,chnwc1
	ld	de,chnwc1+1
	ld	bc,9
	ld	(hl),a
	ldir
	ret
setste:	ld	hl,xstpr
	ld	de,chnwc1
	ld	bc,10
	ldir
	ret

; FUNCTION:	continue_music
;	Tells the Moonblaster 1.4 module to continue playing the music that
;	was stopped before by a call to stop_music.
;
; ENTRY:
;	#None#
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF#
;
; SEE ALSO:
;	<stop_music>
;
moonblaster14.continue_music:	EXPORT	moonblaster14.continue_music
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@continue_music:	EXPORT	continue_music
		ENDIF
	ld	a,(mb14chips)			; At least an MSX Audio or an MSX Music present?	
	or	a				;
	ret	z				; No, then a start will never have occured, and hence not a stop
						; either, so there's nothing to continue.
	ld	a, #ff
	ld	(playing),a
	ret

; FUNCTION:	music_interrupt
;	This routine should be called at every VDP VBlank interrupt (i.e. 50
;	or 60 times per second) after a call to start_music. It processes the
;	music data and writes to the PSG, MSX Audio and MSX Music chips.
;
; ENTRY:
;	#None#
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#AF', BC, BC', DE, DE', HL, HL', IX, IY#
;
; SEE ALSO:
;	<start_music>
;
music_interrupt:	EXPORT	moonblaster14.music_interrupt
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@music_interrupt:	EXPORT	music_interrupt
		ENDIF
	push	af
	ld	a,(playing)
	or	a
	jp	z,endint
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	a,(fading)
	or	a
	call	nz,dofade
	ENDIF
	call	dopsg
	call	dopit
	ld	a,(speed)
	ld	hl,spdcnt
	inc	(hl)
	cp	(hl)
	jp	nz,secint
	ld	(hl),0

	ld	iy,laspl1
	ld	hl,stepbf
	ld	b,9
	ld	de,chnwc1
intl1:	push	bc
	ld	a,(hl)
	or	a
	jp	z,intl3
	push	de
	push	hl
	cp	97
	jp	c,onevn
	jp	z,offevt
	cp	114
	jp	c,chgins
	cp	177
	jp	c,chgvol
	cp	180
	jp	c,chgste
	cp	199
	jp	c,lnkevn
	cp	218
	jp	c,chgpit
	cp	224
	jp	c,chgbr1
	cp	231
	jp	c,chgrev
	cp	237
	jp	c,chgbr2
	cp	238
	jp	c,susevt
	jp	chgmod


intl2:	pop	hl
	pop	de
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
intl3:	ld	bc,25	
	ELSE	
intl3:	ld	bc,22
	ENDIF
	add	iy,bc
	inc	de
	inc	hl
	pop	bc
	djnz	intl1
	call	mmdrum
	ld	a,(de)
	bit	1,a
	call	nz,pacdrm
	inc	hl
	ld	a,(hl)
	or	a
	jr	z,cmdint
	cp	24
	jp	c,chgtmp
	jp	z,endop2
	cp	28
	jp	c,chgdrs
	cp	31
	jp	c,chgsta
	call	chgtrs
cmdint:
endint:	
	pop	af
	ret


secint:	dec	a
	cp	(hl)
	jr	nz,endint
	ld	a,(step)
	inc	a
	and	01111b
	ld	(step),a
	ld	hl,(patpnt)
	call	z,posri3
	ld	de,stepbf
	ld	c,13
dcrstp:	ld	a,(hl)
	cp	243
	jr	nc,crcdat
	ld	(de),a
	inc	de
	dec	c
crcdt2:	inc	hl
	ld	a,c
	or	a
	jr	nz,dcrstp
	ld	(patpnt),hl
	jp	secin2
crcdat:	sub	242
	ld	b,a
	xor	a
crclus:	ld	(de),a
	inc	de
	dec	c
	djnz	crclus
	jr	crcdt2

secin2:	ld	iy,laspl1
	ld	hl,stepbf
	ld	b,9
	ld	de,chnwc1
intl4:	push	bc
	ld	a,(hl)
	or	a
	jr	z,intl5
	cp	97
	jr	nc,intl5
	push	hl
	ld	(iy+6),0
	ld	c,a
	push	de
	push	bc
	ld	a,(de)
	push	af
	and	010b
	call	nz,pacple
	pop	af
	pop	bc
	and	01b
	call	nz,mmple
	pop	de
	pop	hl
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
intl5:	ld	bc,25
	ELSE	
intl5:	ld	bc,22
	ENDIF
	add	iy,bc
	inc	de
	inc	hl
	pop	bc
	djnz	intl4
	jp	endint

mmple:	ld	a,(tpval)
	add	a,c
	cp	96+48+1
	jr	c,mmpl4
	sub	96
	jr	mmpl3
mmpl4:	cp	1+48
	jr	nc,mmpl3
	add	a,96
mmpl3:	sub	48
	ld	(iy+0),a
	ld	hl,pafreq-1
	add	a,a
	add	a,l
	ld	l,a
	jr	nc,mmpl2
	inc	h
mmpl2:	ld	d,(hl)
	dec	hl
	ld	e,(hl)
	ex	de,hl
	add	hl,hl
	ld	a,l
	ld	e,(iy+9)
	add	a,e
	add	a,e
	ld	l,a
	dec	hl
	ld	(iy+16),h
	ld	(iy+17),l
	ret

pacple:	ld	a,(tpval)
	add	a,c
	cp	96+48+1
	jr	c,pacpl4
	sub	96
	jr	pacpl3
pacpl4:	cp	1+48
	jr	nc,pacpl3
	add	a,96
pacpl3:	sub	48
	ld	(iy+5),a
	ld	hl,pafreq-1
	add	a,a
	add	a,l
	ld	l,a
	jr	nc,pacpl2
	inc	h
pacpl2:	ld	a,(hl)
	ld	(iy+18),a
	dec	hl
	ld	a,(hl)
	add	a,(iy+9)
	ld	(iy+19),a
	ret

onevn:	ld	a,(de)
	push	af
	and	010b
	call	nz,pacpl
	pop	af
	and	01b
	call	nz,mmpl
	jp	intl2

offevt:	ld	(iy+6),0
	ld	a,(de)
	push	af
	and	010b
	call	nz,offpap
offet2:	pop	af
	and	01b
	call	nz,offmmp
	jp	intl2

susevt:	ld	(iy+6),0
	ld	a,(de)
	push	af
	and	010b
	call	nz,suspap
	jr	offet2

chgins:	push	de
	ld	(iy+6),0
	sub	97
	ld	c,a
	ld	a,(de)
	push	af
	push	bc
	and	010b
	call	nz,chpaci
	pop	bc
	pop	af
	and	01b
	call	nz,chmodi
	pop	de
	jp	intl2

chgvol:	push	de
	sub	114
	ld	c,a
	ld	a,(de)
	push	af
	push	bc
	and	010b
	call	nz,chpacv
	pop	bc
	pop	af
	and	01b
	call	nz,chmodv
	pop	de
	jp	intl2

chgste:	ld	c,a
	ld	a,(mb14chips)
	cp	MB14_AUDIO + MB14_MUSIC
	jp	nz,intl2
	call	chstdp
	jp	intl2

lnkevn:	sub	189
	ld	c,a
	push	bc
	ld	a,(de)
	push	af
	and	010b
	call	nz,chlkpa
	pop	af
	pop	bc
	and	01b
	call	nz,chlkmm
	jp	intl2

chgpit:	sub	208
	ld	(iy+6),1
	ld	(iy+14),a
	rlca
	jr	c,chgpi2
	ld	(iy+15),0
	jp	intl2
chgpi2:	ld	(iy+15),0ffh
	ld	a,(de)
	push	af
	and	010b
	call	nz,chpidp
	pop	af
	and	01b
	call	nz,chpidm
	jp	intl2

chgbr1:	sub	224
	jr	chgbr3
chgbr2:	sub	230
chgbr3:	push	de
	ld	c,a
	ld	a,(de)
	push	af
	push	bc
	and	010b
	call	nz,chpcbr
	pop	bc
	pop	af
	and	01b
	call	nz,chmmbr
	pop	de
		jp    intl2

chgrev:	sub	227
	ld	(iy+9),a
	jp	intl2
chgmod:	ld	(iy+6),2
	jp	intl2

posri3:	ld	a,(xleng)
	ld	b,a
	ld	a,(pos)
	cp	b
	jr	nz,posri5
	ld	a,(xloop)
	cp	255
	jr	z,posri4
	dec	a
posri5:	inc	a
	ld	(pos),a
	ld	c,a
	ld	b,0
	ld	hl,(xpos)
	add	hl,bc
	ld	a,(hl)
	dec	a
	add	a,a
	ld	c,a
	ld	hl,(patadr)
	add	hl,bc
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ex	de,hl
	ld	de,(musadr)
	add	hl,de
	ret
posri4:	xor	a
	ld	(playing),a
	dec	a
	jr	posri5



;----- Muziek module (MSX Audio) routines -----

;-- Play a note

mmpl:	ld	a,(iy+17)
	ld	c,(iy+7)
	call	fmmout

	ld  (iy+1),a
	set	4,c
	ld	a,(iy+16)
	call	fmmout
	set	5,a
	ld	(iy+2),a
	jp	fmmout

;-- Turn note off

offmmp:	ld	c,(iy+7)
	ld	a,(iy+1)
	call	fmmout
	ld	a,(iy+2)
	set	4,c
	and	011011111b
	ld	(iy+2),a
	jp	fmmout

;-- Change instrument

chmodi:	ld	a,c
	push	bc
	ld	(iy+10),a
	ld	b,a
	ld	hl,xmmvoc-9
	ld	de,9
chmoi2:	add	hl,de
	djnz	chmoi2
	pop	bc
	push	hl

	inc	hl	;verplaats brightness
	inc	hl
	ld	a,(hl)
	ld    	(iy+20),a

	ld	a,10
	sub	b
	ld	b,a
	ld	hl,mmrgad-9
	ld	de,9
chmoi3:	add	hl,de
	djnz	chmoi3
	pop	de
	ld	b,9
chmoi4:	ld	c,(hl)
	ld	a,(de)
	call	fmmout
	inc	hl
	inc	de
	djnz	chmoi4
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ex	de,hl
	dec	hl
	ld	a,(hl)
	ld	(iy+24),a
	ld	de,-5
	add	hl,de
	ld	b,(iy+22)
	ld	a,(hl)
	ld	(iy+22),a
	ld	a,(fading)
	or	a
	ret	z

	ld	a,(hl)
	and	0111111b
	ld	c,a
	ld	a,b
	and	0111111b
	cp	c
	ret	c
	ld	(iy+22),b
	ld	a,b
	ld	c,(iy+13)
	jp	fmmout	
	ELSE	
	ret
	ENDIF


;-- Change volume

chmodv:	ld	a,c
	ex	af,af'
	ld	b,(iy+10)
	ld	hl,xmmvoc-6
	ld	de,9
chmov2:	add	hl,de
	djnz	chmov2
	ld	a,(hl)
	and	11000000b
	ld	c,(iy+13)
	ex	af,af'
	add	a,b
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	call	fmmout
	ld	c,(iy+22)
	ld	(iy+22),a
	ld	a,(fading)
	or	a
	ret	z

	ld	a,b
	and	0111111b
	ld	b,a
	ld	a,c
	and	0111111b
	cp	b
	ret	c
	ld	(iy+22),c
	ld	a,c
	ld	c,(iy+13)	
	ENDIF	
	jp	fmmout

;-- Change stereo setting

chstdp:	ld	(iy+6),0
	ld	a,c
	sub	176
	ld	(de),a
	push	af
	and	1
	call	z,moduit
	pop	af
	and	010b
	call	z,msxuit
	ret

;-- koppelen van noten --

chlkmm:	ld	a,(iy+0)
	add	a,c
	ld	(iy+0),a
	ld	hl,pafreq-1
	add	a,a
	add	a,l
	ld	l,a
	jr	nc,chlkm2
	inc	h
chlkm2:	push	de
	ld	d,(hl)
	dec	hl
	ld	e,(hl)
	ex	de,hl
	add	hl,hl
	dec	hl
	pop	de
	ld	a,h
	ld	(mmfrqs),a
	ld	a,l
	ld	h,(iy+9)
	add	a,h
	add	a,h
	ld	(iy+1),a
	ld	c,(iy+7)
	call	fmmout

	set	4,c
	ld	a,(mmfrqs)
	or	0100000b
	ld	(iy+2),a
	ld	(iy+6),0
	jp	fmmout

;-- Change brightness

chmmbr:	ld	a,(iy+20)
	and	11000000b
	ld	e,a
	ld	a,(iy+20)
	and	00111111b
	ld	b,a
	ld	a,c
	add	a,b
	add	a,e
	ld	(iy+20),a
	ld	c,(iy+13)
	dec	c
	dec	c
	dec	c
	jp	fmmout


;-- Turn pitch bending on

chpidm:	ld	h,(iy+2)
	bit	1,h
	ret	nz
	ld	a,h
	and	11111100b
	sub	4
	ld	l,(iy+1)
	res	5,h
	add	hl,hl
	ld	(iy+1),l
	ex	af,af'
	ld	a,h
	and	00000011b
	ld	h,a
	ex	af,af'
	or	h
	ld	(iy+2),a
	ret

;--- Drumsamples

mmdrum:	ld	a,(de)
	rrca
	jr	nc,nommdr
	ld	a,(hl)
	or	a
	jp	z,mmdru2
	exx
	ld	hl,mmpdt1-2
	add	a,a
	ld	c,a
	ld	b,0
	rl	b
	add	hl,bc
	ld	a,(hl)
	ld	c,11h
	call	fmmout
	inc	hl
	ld	a,(hl)
	dec	c
	call	fmmout
	exx

mmdru2:	inc	hl
	ld	a,(hl)
	or	a
	jp	z,mmdru3
	scf
	rla
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	b,a
	ld	a,(smpvlm)
	cp	b
	jr	c,mmdru4
	ld	a,b
mmdru4:	
	ENDIF	
	ld	c,012h
	call	fmmout
mmdru3:	inc	hl
	ld	a,(hl)
	and	011110000b
	or	a
	ret	z
	srl	a
	srl	a
	srl	a
	srl	a
	exx
	call	mdrblk
	exx
	ld	c,7
	ld	a,1
	call	fmmout
	ld	a,0a0h
	jp	fmmout
nommdr:	inc	hl
	inc	hl
	ret

mdrblk:	add	a,a
	add	a,a
	ld	hl,sample_addresses-4
	ld	c,a
	ld	b,0
	add	hl,bc
	ld	c,9
	ld	a,(hl)
	call	fmmout
	inc	hl
	ld	a,(hl)
	inc	c
	call	fmmout
	inc	hl
	ld	a,(hl)
	inc	c
	call	fmmout
	inc	hl
	ld	a,(hl)
	inc	c
	jp	fmmout


;----- FM-Pac (MSX Music) routines

pacpl:
;-- Play a note

	ld	a,(iy+19)
	ld	c,(iy+8)
	call	fpcout
	ld	(iy+3),a
	ld	a,c
	add	a,010h
	ld	c,a
	ld	a,(iy+18)
	call	fpcout
	set	4,a
	ld	(iy+4),a
	jp	fpcout

;--- Turn off note with sustain ---

suspap:	ld	l,0100000b
	jr	offpa2

;--- Turn off note ---

offpap:	ld	l,0
offpa2:	ld	c,(iy+8)
	ld	a,(iy+3)
	call	fpcout
	ld	a,c
	add	a,010h
	ld	c,a
	ld	a,(iy+4)
	and	011101111b
	or	l
	ld	(iy+4),a
	jp	fpcout


;-- Change instrument --

chpaci:	ld	a,c
	ld	(iy+11),a
	dec	a
	add	a,a
	ld	c,a
	ld	b,0
	ld	hl,xpasti
	add	hl,bc
	ld	a,(hl)
	cp	16
	jp	nc,chpaco
	rlca
	rlca
	rlca
	rlca
chpai2:	inc	hl
	ld	c,(hl)
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	l,a
	add	a,c
	push	bc
	ld	c,(iy+12)
	call	fpcout
	pop	bc
	rlc	c
	rlc	c
	ld	b,(iy+23)
	ld	(iy+23),c
	ld	a,(fading)
	or	a
	ret	z
	ld	a,b
	cp	c
	ret	c
	ld	(iy+23),b
	srl	b
	srl	b
	ld	a,l
	add	a,b
	ELSE	
	add	a,c
	ENDIF
	ld	c,(iy+12)
	jp	fpcout

chpaco:	exx
	sub	15
	rlca
	rlca
	rlca
	ld	b,0
	ld	c,a
	ld	hl,xorgp1-8
	add	hl,bc

	push	hl	; move brightness
	inc	hl
	inc	hl
	ld	a,(hl)
	ld	(iy+21),a
		pop   hl

	ld	bc,0800h
chpao3:	ld	a,(hl)
	call	fpcout
	inc	c
	inc	hl
	djnz	chpao3
	exx
	xor	a
	jp	chpai2


;-- Change volumes --

chpacv:	ld	a,c
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	push	af
	ENDIF
	srl	a
	srl	a
	ex	af,af'
;xxx	ld	hl,xbegvp
	ld	a,(iy+11)
	ld	b,0
	add	a,a
	ld	c,a
	ld	hl,xpasti-2
	add	hl,bc
	ld	a,(hl)
	cp	16
	jr	c,chpcv2
	xor	a
chpcv2:	rlca
	rlca
	rlca
	rlca
	ld	b,a
	ld	c,(iy+12)
	ex	af,af'
	xor	b
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	call	fpcout
	ld	l,b
	pop	bc
	ld	c,(iy+23)
	ld	(iy+23),b
	ld	a,(fading)
	or	a
	ret	z

	ld	a,c
	cp	b
	ret	c
	ld	(iy+23),c
	ld	a,c
	srl	a
	srl	a
	xor	l
	ld	c,(iy+12)	
	ENDIF	
	jp	fpcout

;-- koppelen van noten --

chlkpa:	ld	a,(iy+5)
	add	a,c
	ld	(iy+5),a
	ld	hl,pafreq-1
	add	a,a
	add	a,l
	ld	l,a
	jr	nc,chlkp2
	inc	h
chlkp2:	ld	a,(hl)
	ld	(mmfrqs),a
	dec	hl
	ld	a,(hl)
	add	a,(iy+9)
	ld	(iy+3),a
	ld	c,(iy+8)
	call	fpcout
	ld	a,c
	add	a,010h
	ld	c,a
	ld	a,(mmfrqs)
	or	010000b
	ld	(iy+4),a
	ld	(iy+6),0
	jp	fpcout


;--- Change brightness ---

chpcbr:	ld	a,(iy+11)
	dec	a
	add	a,a
	ld	e,a
	ld	d,0
	ld	hl,xpasti
	add	hl,de
	ld	a,(hl)
	cp	16
	ret	c
	ld	a,(iy+21)
	and	11000000b
	ld	e,a
	ld	a,(iy+21)
	and	00111111b
	ld	b,a
	ld	a,c
	add	a,b
	add	a,e
	ld	(iy+21),a
	ld	c,2
	jp	fpcout


;-- Change pitch bending --

chpidp:	ld	a,(iy+4)
	bit	0,a
	ret	nz
	dec	a
	ld	(iy+4),a
	ld	a,(iy+3)
	add	a,a
	ld	(iy+3),a
	ret

;-- FM-Pac drum --

pacdrm:	ld	a,(hl)
	and	01111b
	or	a
	ret	z
	ld	e,a
	ld	d,0
	push	hl
	ld	hl,xdrblk-1
	add	hl,de
	ld	a,(hl)
	cp	0100000b
	ld	c,a
	call	nc,psgdrm
	pop	hl
	ld	a,(xsust)
	and	0100000b
	ret	z
	ld	a,c
	and	011111b
	ld	c,0eh
	call	fpcout
	set	5,a
	jp	fpcout

psgdrm:	rlca
	rlca
	rlca
	and	0111b
	add	a,a
	ld	e,a
	ld	hl,psgadr-2
	add	hl,de
	ld	e,(hl)
	inc	hl
	ld	d,(hl)
	ex	de,hl
	ld	a,(hl)
	ld	(psgcnt),a
	inc	hl
	ld	b,(hl)
	inc	hl
psgl1:	ld	a,(hl)
	out	(0a0h),a
	inc	hl
	ld	a,(hl)
	out	(0a1h),a
	inc	hl
	djnz	psgl1
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	a,(maxpsg)
	ld	b,a	
	ld	a,(hl)
	cp	b
	jr	c,psgdr2
	ld	a,b
psgdr2:	
	ELSE	
	ld	a,(hl)
	ENDIF	
	ld	(psgvol),a
	jp	dopsg2


;---- Command routines ----

chgtmp:	ld	b,a
	ld	a,25
	sub	b
	ld	(speed),a
	jp	cmdint

endop2:	ld	a,15
	ld	(step),a
	jp	cmdint

chgdrs:	sub	25
	add	a,a
	ld	b,a
	add	a,a
	add	a,b
	ld	e,a
	ld	d,0
	ld	hl,xdrfrq
	add	hl,de
	ex	de,hl
	ld	hl,drmreg+3
	ld	b,6
chgdrl:	ld	c,(hl)
	ld	a,(de)
	call	fpcout
	inc	hl
	inc	de
	djnz	chgdrl
	jp	cmdint


;--- Status bytes ---

chgsta:	ld	c,a
	ld	b,0
	ld	hl,status-28
	add	hl,bc
	ld	(hl),255
	jp	cmdint

;--- Change transpose ---

chgtrs:	sub	55-48
	ld	(tpval),a
		ret


;--- 'Every interrupt' routines ----

	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
;--- Music fade ---

dofade:	ld	a,(fade_speed)
	ld	b,a
	ld	a,(fade_counter)
	inc	a
	ld	(fade_counter),a
	cp	b
	ret	nz
	xor	a
	ld	(fade_counter),a
	ld	iy,laspl1
	ld	b,9
	ld	hl,0
dofadl:	push	bc
	ld	a,(iy+22)
	and	11000000b
	ld	b,a
	ld	a,(iy+22)
	and	0111111b
	add	a,2
	cp	64
	jr	c,dofad2
	ld	a,63
dofad2:	ld	c,(iy+13)
	add	a,b
	ld	(iy+22),a
	call	fmmout
	ld	b,a
	ld	a,(iy+24)
	bit	0,a
	call	nz,dofada
	ld	a,b
	and	111111b
	xor	63
	ld	e,a
	ld	d,0
	add	hl,de
	pop	bc
	push	bc
	ld	a,b
	cp	3
	jr	nc,dofad6
	ld	a,(xsust)
	bit	5,a
	jp	nz,dofad8
dofad6:	push	hl
	ld	a,(iy+11)
	dec	a
	add	a,a
	ld	c,a
	ld	b,0
	ld	hl,xpasti
	add	hl,bc
	ld	a,(hl)
	cp	16
	jr	c,dofad5
	xor	a
dofad5:	rlca
	rlca
	rlca
	rlca
	ld	b,a
	ld	a,(iy+23)
	add	a,2
	cp	64
	jr	c,dofad3
	ld	a,63
dofad3:	ld	(iy+23),a
	srl	a
	srl	a
	ld	c,(iy+12)
	add	a,b
	call	fpcout
	pop	hl
	and	1111b
	xor	15
	ld	e,a
	ld	d,0
	add	hl,de
	ld	bc,25
	add	iy,bc
	pop	bc
	dec	b
	jp	nz,dofadl

	ld	a,(smpvlm)
	sub	12
	jr	nc,dofad4
	xor	a
dofad4:	ld	(smpvlm),a
	ld	c,012h
	call	fmmout
	ld	a,(maxpsg)
	or	a
	jr	nz,dofad9
	ld	a,1
dofad9:	dec	a
	ld	(maxpsg),a
	ld	de,0
	CP_HL_DE
	ret	nz
	xor	a
	ld	(playing),a
	ret

dofad8:	push	hl
	ld	a,(iy+23)
	srl	a
	srl	a
	jp	dofad5

dofada:	ld	a,(iy+20)
	and	11000000b
	ld	c,a
	ld	a,(iy+20)
	and	0111111b
	add	a,2
	cp	64
	jr	c,dofadb
	ld	a,63
dofadb:	add	a,c
	ld	(iy+20),a
	ld	c,(iy+13)
	dec	c
	dec	c
	dec	c
	jp	fmmout
	
	ENDIF

;--- PSG drums ---

dopsg:	ld	a,(psgcnt)
	or	a
	ret	z
	dec	a
	ld	(psgcnt),a
	jr	z,endpsg
dopsg2:	ld	a,8
	out	(0a0h),a
	ld	a,(psgvol)
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	cp	4
	jr	nc,dopsg3
	ld	a,4
dopsg3:	
	ENDIF		
	sub	2
	ld	(psgvol),a
trbpsg:	nop
	nop
	out	(0a1h),a
	ret
endpsg:	ld	a,7
	out	(0a0h),a
	ld	a,255
	out	(0a1h),a
	ld	a,8
	out	(0a0h),a
	xor	a
	out	(0a1h),a
	ret

;----- Pitch bending -----


dopit:	ld	iy,laspl1
	ld	de,chnwc1
	exx
	ld	b,9
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	de,25
	ELSE	
	ld	de,22
	ENDIF
dopitl:	exx
	ld	a,(iy+6)
	ld	h,a
	or	a
	jp	z,dopit2
	ld	a,(de)
	and	01b
	call	nz,pitmm
	ld	a,(de)
	and	010b
	call	nz,pitpa
dopit2:	inc	de
	exx
	add	iy,de
	djnz	dopitl
	ret


;--- Pitch bend Muziek module ---

pitmm:	push	hl
	dec	h
	jr	nz,modmm
	ld	l,(iy+14)
	ld	h,(iy+15)
	ld	c,(iy+1)
	ld	b,(iy+2)
	bit	7,h
	jr	nz,pitmm2
	add	hl,hl
	add	hl,bc
	ld	a,b
	and	00000010b
	ld	b,a
pitmm4:	ld	(iy+1),l
	ld	c,(iy+7)
	ld	a,l
	call	fmmout
	ld	a,h
	or	b
	ld	(iy+2),a
	set	4,c
	pop	hl
	jp	fmmout

pitmm2:	add	hl,hl
	add	hl,bc
	bit	1,h
	jr	nz,pitmm3
	dec	h
	dec	h
pitmm3:	ld	b,0
	jp	pitmm4


;--- Pitch bend FM-PAC ---

pitpa:	dec	h
	jr	nz,modpa
	ld	l,(iy+14)
	ld	h,(iy+15)
	ld	c,(iy+3)
	ld	b,(iy+4)
	bit	7,h
	jr	nz,pitpa2
	add	hl,bc
	ld	a,b
	and	00000001b
	ld	b,a
pitpa4:	ld	(iy+3),l
	ld	c,(iy+8)
	ld	a,l
	call	fpcout
	ld	a,c
	add	a,010h
	ld	c,a
	ld	a,h
	or	b
	ld	(iy+4),a
	jp	fpcout

pitpa2:	add	hl,bc
	bit	0,h
	jr	nz,pitpa3
	dec	h
pitpa3:	ld	b,0
	jp	pitpa4

;---- Modulation Muziek module (MSX Audio) ----

modmm:	ld	a,h
	add	a,2
	cp	12
	jr	nz,modmm3
	ld	a,2
modmm3:	ld	(iy+6),a
	ld	a,h
	add	a,a
	ld	c,a
	ld	b,0
	ld	hl,modval-2
	add	hl,bc
	ld	c,(hl)
	sla	c
	inc	hl
	ld	b,(hl)
	ld	l,(iy+1)
	ld	h,(iy+2)
	add	hl,bc
	ld	b,0
	jp	pitmm4

;---- Modulation FM-PAC (MSX Music) ----

modpa:	ld	a,(de)
	cp	2
	jr	nz,modpa2
	ld	a,h
	add	a,2
	cp	12
	jr	nz,modpa3
	ld	a,2
modpa3:	ld	(iy+6),a
modpa2:	ld	a,h
	add	a,a
	ld	c,a
	ld	b,0
	ld	hl,modval-2
	add	hl,bc
	ld	c,(hl)
	inc	hl
	ld	b,(hl)
	ld	l,(iy+3)
	ld	h,(iy+4)
	add	hl,bc
	ld	b,0
	jp	pitpa4


; Out-routines

mmout:	jp	mm2out
fmmout:	jp	mm3out
pacout:	jp	pc2out
fpcout:	jp	pc3out

mm2out:	ex	af,af'
	ld	a,c
	out	(0c0h),a
	ex	af,af'
	out	(0c1h),a
	ex	(sp),hl
	ex	(sp),hl
	ret
pc2out:	ex	af,af'
	ld	a,c
	out	(7ch),a
	ex	af,af'
	out	(7dh),a
	ex	(sp),hl
	ex	(sp),hl
	ret
mm3out:	ex	af,af'
	ld	a,c
	out	(0c0h),a
	ex	af,af'
	out	(0c1h),a
	ret
pc3out:	ex	af,af'
	ld	a,c
	out	(7ch),a
	ex	af,af'
	out	(7dh),a
	ret
mmrout:	ex	af,af'
	call	trbwt
	ld	a,c
	out	(0c0h),a
	in	a,(0e6h)
	ld	(rtel),a
	ex	af,af'
	out	(0c1h),a
	ret
parout:	ex	af,af'
	call	trbwt
	ld	a,c
	out	(7ch),a
	in	a,(0e6h)
	ld	(rtel),a
	ex	af,af'
	out	(7dh),a
	ret
trbwt:	push	bc
	ld	a,(rtel)
	ld	b,a
trbwl:	in	a,(0e6h)
	sub	b
	cp	7
	jr	c,trbwl
	pop	bc
	ret
	
; FUNCTION:	music_playing
;	Find out whether the Moonblaster 1.4 module is actually playing a
;	music at the moment or not. The music will not be playing if:
;	- neither an MSX Audio nor an MSX Music chip was detected
;	- stop_music was called (and no call to start_music or continue_music
;				 has been made since)
;	- the music reached its end and is non-looping
;	- the fade-out has finished
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Music playing flag (0 is NOT playing)
;
; MODIFIES:
;	#None#
;
; SEE ALSO:
;	<fade_music>
;
music_playing:	EXPORT	moonblaster14.music_playing
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@music_playing:	EXPORT	music_playing
		ENDIF
		ld	a, (playing)
		ret

	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT	
; FUNCTION:	fade_music
;	Tells the Moonblaster 1.4 module to start fading out the music that's
;	being played. You need to keep calling the music_interrupt routine 
;	though. That is, this routine really only tells the module that the
;	music should be	faded, but it doens't do the actual fading.
;
; ENTRY:
;	A - Fade speed. 1 = fastest, 255 = slowest
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#A#
;
; SEE ALSO:
;	<music_playing>
;
fade_music:	EXPORT	moonblaster14.fade_music
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@fade_music:	EXPORT	fade_music
		ENDIF
		ld	(fade_speed),a
		ld	a, $ff
		ld	(fading),a
		ret
	ENDIF

; FUNCTION:	stop_music
;	Tells the Moonblaster 1.4 module to stop playing the music. All PSG,
;	MSX Audio and MSX Music channels are immediately silenced. Any more
;	calls to the music_interrupt routine won't have any effect anymore.
;
; ENTRY:
;	#None#
; EXIT:
;	EI -
;
; MODIFIES:
;	#ALL#
;
; SEE ALSO:
;	<continue_music>
;
stop_music:	EXPORT	moonblaster14.stop_music
		IFDEF	MAKE_MOONBLASTER14_GLOBAL
@stop_music:	EXPORT	stop_music
		ENDIF
	di
	ld	a,(playing)			; Really playing?
	or	a				;
	jr	z,.return			; No, then don't stop what we're not doing ;)
	IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
	ld	de,25
	ELSE	
	ld	de,22
	ENDIF
	ld	iy,laspl1
	ld	b,9
.channel_loop:
	call	msxuit
	call	moduit
	add	iy,de
	djnz	.channel_loop
	xor	a
	ld	(playing),a
	call	endpsg
.return:	
	ei
	ret
	
msxuit:	ld	a,(xsust)
	bit	5,a
	jr	z,msxui2
	ld	a,b
	cp	4
	ret	c
msxui2:	ld	c,(iy+8)
	xor	a
	call	pacout
	ld	a,c
	add	a,10h
	ld	c,a
	xor	a
	jp	fpcout
moduit:	ld	c,(iy+7)
	xor	a
	call	mmout
	set	4,c
	jp	fmmout



;--- psg drum data ---

pbddat:		db	5,3,0,179,1,6,7,0feh,17
ps1dat:		db	6,2,6,013h,7,0f7h,15
ps2dat:		db	6,2,6,009h,7,0f7h,15
pb1dat:		db	4,3,0,173,1,1,7,0feh,15
pb2dat:		db	4,3,0,72,1,0,7,0feh,15
ph1dat:		db	5,2,6,006h,7,0f7h,15
ph2dat:		db	5,2,6,001h,7,0f7h,14
psgadr:		dw	pbddat,ps1dat,ps2dat,pb1dat,pb2dat,ph1dat,ph2dat
psgcnt:		db	0
psgvol:		db	0
		IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
maxpsg:		db	0
		ENDIF

;--- MSX Audio data ---

mmpdt1: 	db    	005h,022h,005h,06ah,005h,0bah,006h,012h,006h,073h,006h,0d3h
		db    	007h,03bh,007h,0abh,008h,01bh,008h,09ch,009h,024h,009h,0ach
		db    	00ah,03dh,00ah,0d5h,00bh,07dh,00ch,02dh,00ch,0e6h,00dh,0a6h
		db    	00eh,07fh,00fh,057h,010h,03fh,011h,038h,012h,040h,013h,051h
		db    	014h,07ah,015h,0aah,016h,0fbh,018h,05bh,019h,0c4h,01bh,04dh
		db    	01ch,0feh,01eh,0b6h,020h,077h,022h,070h,024h,081h,026h,0aah
		db    	028h,0fch,02bh,05dh,02dh,0f6h,030h,0afh,033h,089h,036h,0a2h
		db    	039h,0f4h,03dh,066h,040h,0f7h,044h,0e1h,049h,00bh,04dh,04dh
		db    	051h,0f0h,056h,0b2h,05bh,0ech,061h,05fh,067h,01ah,06dh,045h
		db    	073h,0e8h,07ah,0cbh,081h,0f0h,089h,0c4h,092h,018h,09ah,0a4h
mmrgad: 	db    	20h,23h,40h,43h,60h,63h,80h,83h,0c0h ;k1
		db    	21h,24h,41h,44h,61h,64h,81h,84h,0c1h ;k2
		db    	22h,25h,42h,45h,62h,65h,82h,85h,0c2h ;k3
		db    	28h,2bh,48h,4bh,68h,6bh,88h,8bh,0c3h ;k4
		db    	29h,2ch,49h,4ch,69h,6ch,89h,8ch,0c4h ;k5
		db    	2ah,2dh,4ah,4dh,6ah,6dh,8ah,8dh,0c5h ;k6
		db    	30h,33h,50h,53h,70h,73h,90h,93h,0c6h ;k7
		db    	31h,34h,51h,54h,71h,74h,91h,94h,0c7h ;k8
		db    	32h,35h,52h,55h,72h,75h,92h,95h,0c8h ;k9
sample_addresses:	ds	56
strreg: 	db    	16,0f0h,17,051h,18,255,018h,8,019h,8

;--- MSX Music data ---

pafreq: 	db    	0adh,000h,0b7h,000h,0c2h,000h,0cdh,000h,0d9h,000h,0e6h,000h
		db    	0f4h,000h,003h,001h,012h,001h,022h,001h,034h,001h,046h,001h
		db    	0adh,002h,0b7h,002h,0c2h,002h,0cdh,002h,0d9h,002h,0e6h,002h
		db    	0f4h,002h,003h,003h,012h,003h,022h,003h,034h,003h,046h,003h
		db    	0adh,004h,0b7h,004h,0c2h,004h,0cdh,004h,0d9h,004h,0e6h,004h
		db    	0f4h,004h,003h,005h,012h,005h,022h,005h,034h,005h,046h,005h
		db    	0adh,006h,0b7h,006h,0c2h,006h,0cdh,006h,0d9h,006h,0e6h,006h
		db    	0f4h,006h,003h,007h,012h,007h,022h,007h,034h,007h,046h,007h
		db    	0adh,008h,0b7h,008h,0c2h,008h,0cdh,008h,0d9h,008h,0e6h,008h
		db    	0f4h,008h,003h,009h,012h,009h,022h,009h,034h,009h,046h,009h
		db    	0adh,00ah,0b7h,00ah,0c2h,00ah,0cdh,00ah,0d9h,00ah,0e6h,00ah
		db    	0f4h,00ah,003h,00bh,012h,00bh,022h,00bh,034h,00bh,046h,00bh
		db    	0adh,00ch,0b7h,00ch,0c2h,00ch,0cdh,00ch,0d9h,00ch,0e6h,00ch
		db    	0f4h,00ch,003h,00dh,012h,00dh,022h,00dh,034h,00dh,046h,00dh
		db    	0adh,00eh,0b7h,00eh,0c2h,00eh,0cdh,00eh,0d9h,00eh,0e6h,00eh
		db    	0f4h,00eh,003h,00fh,012h,00fh,022h,00fh,034h,00fh,046h,00fh
drmreg: 	db    	36h,37h,38h,16h,26h,17h,27h,18h,28h

;--- General data ---

chnwc1: 	dw    	0,0,0,0,0
modval: 	dw     	1,2,2,-2,-2,-1,-2,-2,2,2
mmfrqs: 	db    	0
tpval:		db	0	
speed:  	db    	0
spdcnt: 	db    	0
rtel:   	db    	0
patadr: 	dw    	0
patpnt: 	dw    	0
xpos:   	dw	0
		IFDEF	INCLUDE_MOONBLASTER14_FADE_SUPPORT
smpvlm: 	db    	0
fading: 	db    	0
fade_speed: 	db    	12
fade_counter: 	db    	0

laspl1: 	db    	0,0,0,0,0,0,0,0a0h,10h,0,0,0,030h,043h,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a1h,11h,0,0,0,031h,044h,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a2h,12h,0,0,0,032h,045h,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a3h,13h,0,0,0,033h,04bh,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a4h,14h,0,0,0,034h,04ch,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a5h,15h,0,0,0,035h,04dh,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a6h,16h,0,0,0,036h,053h,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a7h,17h,0,0,0,037h,054h,0,0,0,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a8h,18h,0,0,0,038h,055h,0,0,0,0,0,0,0,0,0,0,0	
		ELSE
laspl1: 	db    	0,0,0,0,0,0,0,0a0h,10h,0,0,0,030h,043h,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a1h,11h,0,0,0,031h,044h,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a2h,12h,0,0,0,032h,045h,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a3h,13h,0,0,0,033h,04bh,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a4h,14h,0,0,0,034h,04ch,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a5h,15h,0,0,0,035h,04dh,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a6h,16h,0,0,0,036h,053h,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a7h,17h,0,0,0,037h,054h,0,0,0,0,0,0,0,0
		db    	0,0,0,0,0,0,0,0a8h,18h,0,0,0,038h,055h,0,0,0,0,0,0,0,0
		ENDIF

; Copy of the settings of the music that's being played.
xleng:		ds    	3
xmmvoc:  	ds    	16*9
xmmsti:  	ds    	16
xpasti:  	ds	32
xstpr:   	ds    	10
xtempo:  	ds    	1
xsust:   	ds    	1
xbegvm:  	ds    	9
xbegvp:  	ds    	9
xorgp1:  	ds    	6*8
xorgnr:  	ds    	6
xsmpkt:  	ds    	8
xdrblk:  	ds    	15
xdrvol:  	ds    	3
xdrfrq:  	ds    	20
xrever:  	ds    	9
xloop:   	ds	1

		MODULE
