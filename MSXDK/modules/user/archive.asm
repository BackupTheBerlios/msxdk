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

		MODULE	archive		
; FILE: archive
;	The archive module contains routines for the opening, closing
;	and reading from archives created using the "arcreate" program.
;	Don't forget to correctly define <ARCHIVE_DEPACK_ROUTINE> if you
;	want packed data to be depacked by the archive module.
;	:
; 	Archive format:@
;:	2 bytes		size of TOC (Table Of Content)
;:
;:	TOC:
;:	per file:
;:	3 bytes		offset inside archive to first chunk
;:	2 bytes		number of chunks
;:	1 byte		attributes: 0 = raw, 1 = packed
;:
;:	per chunk:
;:	2 bytes		size of chunk
;:	<size> bytes	chunk data
;
; FUNCTION NAMES:
;	Except for <init_archive> and <kill_archive> all functions
;	are MODULE local, which means you have to add the prefix "archive." to
;	their names. This is to prevent possible name clashes with functions
;	from other libraries you might be using. However, if <MAKE_ARCHIVE_GLOBAL>
;	is defined then all public functions will be available without the "archive."
;	prefix as well.
;
; COPYRIGHT & CREDITS:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.

; DEFINE:	ARCHIVE_DEPACK_ROUTINE
;	You can set this define to point to a depack routine that should
;	be called whenever the archive should load and therefore depack, 
;	packed data. The depack routine is allowed to change all registers,
;	and must return the size of the *depacked* data in #HL#. Needless to
;	say, the depack routine must match the packer used to pack data in 
;	the archive.

; DEFINE:	MAKE_ARCHIVE_GLOBAL
;	Defining this will make all public functions that are normally only
;	available with the "archive." prefix to also be available without
;	this prefix. See the introduction of <archive> for more information.

; FUNCTION:	init_archive
;	Initialises the archive module.
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
@init_archive:	xor	a
		ret

; FUNCTION:	kill_archive
;	Kills the archive module.
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
@kill_archive:	call close_archive
		ret

;****if* archive/_readbytes
; FUNCTION:
; 	Internal helper function. DO NOT USE YOURSELF!
; ENTRY:
;	HL - Number of bytes to read
;	DE - Address to load bytes at
; EXIT:
; 	A=0 if OK, A=1 if ERROR
;	EI
; MODIFIES:
;	#ALL#
;******
;
_readbytes:
		push	hl
		ld	c, BDOS_SETDTA			; Set the Data Transfer Address to the one given in DE
		call	BDOS				;
		pop	hl
		ld	de, fcb				; Now actually load HL bytes to the set DTA
		ld      c, BDOS_RANDOMRECORDREAD	;
		call	BDOS				;
		ret


; FUNCTION:	open_archive
;	Opens an archive.
;
; ENTRY:
;	HL - Address of filename+ext (8+3 = 11 bytes)
;	BC - Address of area to be used to store the TOC
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	EI -
;
; MODIFIES:
;	#ALL#
;
; NOTES:
;	This module only supports one opened archive at a time and it is an
; 	error to try to open an archive whilst another archive is still open.
;
open_archive:	EXPORT	archive.open_archive
		IFDEF	MAKE_ARCHIVE_GLOBAL
@open_archive:	EXPORT	open_archive		
		ENDIF
		ld	a,(busy)
		ld	de, error_archivebusy
		or	a
		ret	nz

		ld	(toc_address),bc

		; Copy the archive name to the error_archiveopenread string
		push	hl
		ld	de, error_archiveopenread_filename
		ld	bc, 8
		ldir
		inc	de
		ld	c, 3
		ldir
		pop	hl

		; Zero-fill the FCB
		push	hl
		ld	hl, fcb
		ld	de, fcb+1
		ld	bc, 37-1
		ld	(hl),b
		ldir
		pop	hl
		; Copy the filename of the archive file to the FCB
		ld	de, fcb + FCB_FILENAME
		ld	bc, 11
		ldir

		; Open the archive-file
		ld      de, fcb
		ld      c, BDOS_OPENFILE
		call    BDOS
		or	a
		jr	nz, .error

		; Set the busy-flag
		ld	a, $ff
		ld	(busy),a

		; Make sure our first read starts at offset 0 and that
		; the recordsize is set to 1, so that we can read with
		; byte accuracy.
		ld      hl, 0
		ld	(fcb + FCB_CURRENTBLOCK), hl
		ld	(fcb + FCB_RANDOMRECORD), hl
		ld	(fcb + FCB_RANDOMRECORD + 2), hl
		ld	(fcb + FCB_CURRENTRECORD), hl ; also sets FCB_RANDOMRECORD to 0
		inc	hl
		ld	(fcb + FCB_RECORDSIZE), hl

		; Now load the TOC
		;

		; First, read the size of the TOC
		ld      de, toc_size
		ld	hl, 2
		call	_readbytes
		or	a
		jr	nz, .error_closefirst
		; Now read the actual TOC
		ld      de, (toc_address)
		ld	hl, (toc_size)
		call	_readbytes
		or	a
		jr	nz, .error_closefirst
		xor	a
		ret

.error_closefirst:
		call	close_archive
.error:
		ld	de, error_archiveopenread
		ld	a, $ff
		ret

; FUNCTION:	close_archive
;	Closes an archive.
;
; ENTRY:
;	#None#
;
; EXIT:
;	EI
;
; MODIFIES:
;	#ALL#
;
close_archive:	EXPORT	archive.close_archive
		IFDEF	MAKE_ARCHIVE_GLOBAL
@close_archive:	EXPORT	close_archive		
		ENDIF
		ld	a,(busy)
		or	a
		ret	z
		ld	c, BDOS_CLOSEFILE
		ld	de, fcb
		call	BDOS
		xor	a
		ld	(busy),a
		ret

; FUNCTION:	set_packed_buffer
;	Sets the address of the buffer where packed data is to be loaded at. 
;	If a request is	made to load a packed chunk/file from the archive 
;	then it is first loaded at this address before it is depacked.
;
; ENTRY:
;	DE - Address
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#None#
;
; SEE ALSO:
;	<set_load_address>, <set_vram_address>
;
		IFDEF	ARCHIVE_DEPACK_ROUTINE
set_packed_buffer:	EXPORT	archive.set_packed_buffer
		IFDEF	MAKE_ARCHIVE_GLOBAL
@set_packed_buffer:	EXPORT	set_packed_buffer		
		ENDIF
		ld	(packed_buffer_address),de
		ret
		ENDIF

; FUNCTION:	set_load_address
;	Sets the address where data is to be loaded at. If a request is made to
;	load a packed chunk/file from the archive then it will be depacked to
;	this address.
;
; ENTRY:
;	DE - Address
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#None#
;
; SEE ALSO:
;	<set_packed_buffer>, <set_vram_address>
;******
;
set_load_address:	EXPORT	archive.set_load_address
		IFDEF	MAKE_ARCHIVE_GLOBAL
@set_load_address:	EXPORT	set_load_address		
		ENDIF
		ld	(load_address),de
		ret

; FUNCTION:	set_vram_address
;	Sets the VRAM address where data is to be copied to when using
;	load_*_to_vram* routines.
;
; ENTRY:
;	BDE - VRAM address (17 bit)
;
; EXIT:
;	#None#
;
; MODIFIES:
;	#None#
;
; SEE ALSO:
;	<set_packed_buffer>, <set_load_address>
;
set_vram_address:	EXPORT	archive.set_vram_address
		IFDEF	MAKE_ARCHIVE_GLOBAL
@set_vram_address:	EXPORT	set_vram_address		
		ENDIF
		ld	(vram_address + 1), bc
		ld	(vram_address), de
		ret

; FUNCTION:	find_file
;	Finds the info for the n-th file in the archive. The chunk reader is
;	positioned at the first chunk of the file so that the next call to
;	<load_chunk> or <load_chunk_at> will read the first chunk of
;	the found file.
;
; ENTRY:
;	HL - File index
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Number of chunks that the file consists of
;
; MODIFIES:
;	#F, BC, DE#
;
;
find_file:	EXPORT	archive.find_file
		IFDEF	MAKE_ARCHIVE_GLOBAL
@find_file:	EXPORT	find_file		
		ENDIF
		add	hl, hl
		ex	de, hl
		ld	hl, (toc_address)
		add	hl, de
		add	hl, de
		add	hl, de

		; Copy the offset to the FCB
		ld	de, fcb + FCB_RANDOMRECORD
		ld	bc, 3
		ldir
		xor	a
		ld	(fcb + FCB_RANDOMRECORD + 3), a

		; And now return the number of chunks
		ld	e, (hl)
		inc	hl
		ld	d, (hl)
		inc	hl
		ld	a, (hl)
		ld	(attributes),a
		ex	de, hl

		xor	a
		ret

; FUNCTION: load_chunk_at
; 	Load the next chunk from the file found with <find_file> at the
;	given address. If the file is packed then the disk data is first loaded
;	at the address set with <set_packed_buffer> and then depacked to
;	the given address.
;
; ENTRY:
;	DE - Address to load chunk at
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Size of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;******
;
load_chunk_at:	EXPORT	archive.load_chunk_at
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_chunk_at:	EXPORT	load_chunk_at		
		ENDIF
		push	de
		; The 2 bytes preceding the actual chunk data contain the chunk data size
		ld	de, chunk_size
		ld	hl, 2
		call	_readbytes
		pop	de
		or	a
		jr	nz, .error

		push	de
		IFDEF	ARCHIVE_DEPACK_ROUTINE
		ld	a,(attributes)			; Get the file's attributes
		cp	1				; Is the file packed?
		jr	nz,.load_data			; No, then just load at the given address in DE
		ld	de,(packed_buffer_address)	; If it is packed, load at the set packed address
.load_data
		ENDIF
		ld	hl,(chunk_size)
		call	_readbytes
		or	a				; Did an error occur?
		jr	nz, .error			; Yes, return a nice error string and A <> 0.
		pop	de				; Given address
		ld	hl,(chunk_size)			; Get the loaded chunk size
		IFDEF	ARCHIVE_DEPACK_ROUTINE
		ld	a,(attributes)			; Get the file's attributes
		cp	1				; Is the file packed?
		jr	nz, .done			; No, then there's nothing more to be done
		ld	hl,(packed_buffer_address) 	; Address the data was actually loaded at	
		call	ARCHIVE_DEPACK_ROUTINE		; Depack to the given address (it's in DE)
							; This also returns the depacked size in HL							
.done:
		ENDIF
		xor	a
		ret

.error:		ld	de, error_archiveopenread
		ld	a, $ff
		ret

; FUNCTION: load_chunk
; 	Load the next chunk from the file found with <find_file> at the
;	address set with <set_load_address>. If the file is packed then the
;	disk data is first loaded into the buffer set with <set_packed_buffer>
;	and then depacked to the address set with <set_load_address>.
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Size of loaded (or depacked) data
;	EI
;
; MODIFIES:
;	#ALL#
;
load_chunk:	EXPORT	archive.load_chunk
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_chunk:	EXPORT	load_chunk		
		ENDIF
		ld	de,(load_address)
		jr	load_chunk_at

; FUNCTION: load_file_at
;	Find the n-th file in the archive and load all its chunks into memory
;	as one sequential block, with the base address being the given address.
;	See <load_chunk_at> for more information on how each chunk is loaded.
;
; ENTRY:
;	HL - File index
;	DE - Address
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Total size of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;
load_file_at:	EXPORT	archive.load_file_at
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_file_at:	EXPORT	load_file_at		
		ENDIF
		push	de
		push	de
		call	find_file		; Returns the number of chunks in HL
		pop	de
.next_chunk:	push	hl
		push	de
		call	load_chunk_at
		or	a
		jr	nz, .error
		pop	de
		add	hl, de
		ex	de, hl
		pop	hl
		dec	hl
		ld	a, h
		or	l
		jr	nz, .next_chunk
		ex	de, hl
		pop	de
		; Carry still clear from the "or l"
		sbc	hl, de
		; A still 0 from the loop above
		ret
.error:
		; A <> 0 from all paths leading to this point
		pop	hl
		pop	hl
		pop	hl
		ret

; FUNCTION: load_file
;	Find the n-th file in the archive and load all its chunks into memory
;	as one sequential block, with the base address being the address set
;	with <set_load_address>.
;	See <load_chunk_at> for more information on how each chunk is loaded.
;
; ENTRY:
;	HL - File index
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Total size of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;
load_file:	EXPORT	archive.load_file
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_file:	EXPORT	load_file		
		ENDIF
		ld	de,(load_address)
		jp	load_file_at

; FUNCTION:	load_chunk_to_vram_at
; 	Loads the next chunk from the file found with <find_file> at the
;	address set with <set_load_address> and then copies the data to
;	VRAM at the given address.
;	See <load_chunk_at> for more information on how the chunk is loaded.
;
; ENTRY:
;	BDE - VRAM address (17 bit)
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Size of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;
load_chunk_to_vram_at:	EXPORT	archive.load_chunk_to_vram_at
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_chunk_to_vram_at:	EXPORT	load_chunk_to_vram_at		
		ENDIF
		push	de
		push	bc
		call	load_chunk
		pop	bc
		ex	(sp),hl
		di
		or	a
		jr	nz,.error
		call	set_vram_write
		pop	de
		push	de
		ld	hl,(load_address)
		call	ram_to_vram
		pop	hl
		xor	a
		ei
		ret
.error:
		; A <> 0 from all paths leading to this point
		pop	bc
		ret

; FUNCTION:	load_chunk_to_vram
; 	Load the next chunk from the file found with <find_file> at the
;	address set with <set_load_address> and then copy the data to the
;	VRAM address set with <set_vram_address>.
;	See <load_chunk_at> for more information on how the chunk is loaded.
;
; ENTRY:
;	#None#
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	HL - Size of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;
load_chunk_to_vram:	EXPORT	archive.load_chunk_to_vram
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_chunk_to_vram:	EXPORT	load_chunk_to_vram		
		ENDIF
		ld	de,(vram_address)
		ld	bc,(vram_address+1)
		jr	load_chunk_to_vram_at

; FUNCTION:	load_file_to_vram_at
;	Find the n-th file in the archive and load all its chunks into memory
;	one by one, copying them into VRAM as one sequential block, with the
;	base VRAM address being the given VRAM address.
;	See <load_chunk_to_vram_at> for more information on how each chunk is
;	loaded.
;
; ENTRY:
;	HL - File index
;	BDE - VRAM Address (17 bit)
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	CHL - Total size (18 bit) of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;
load_file_to_vram_at:	EXPORT	archive.load_file_to_vram_at
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_file_to_vram_at:	EXPORT	load_file_to_vram_at		
		ENDIF
		push	bc
		push	de
		call	find_file		; Returns the number of chunks in HL
		pop	de
		pop	bc
		ld	ix, $0000
		ld	c, $0
.next_chunk:	push	hl

		push	ix
		push	de
		push	bc
		call	load_chunk_to_vram_at
		or	a
		jr	nz,.error
		pop	bc
		pop	de
		push	hl
		add	hl, de
		INCC	b
		pop	de
		pop	ix
		add	ix, de
		INCC	c
		ex	de, hl

		pop	hl
		dec	hl
		ld	a, h
		or	l
		jr	nz,.next_chunk
		; A = 0 from the loop above
		ret
.error:
		; A <> 0 from all paths leading to this point
		pop	bc
		pop	bc
		pop	bc
		pop	bc
		ret
		
; FUNCTION: load_file_to_vram
;	Find the n-th file in the archive and load all its chunks into memory
;	one by one, copying them into VRAM as one sequential block, with the
;	base VRAM address being the VRAM address set with <set_vram_address>.
;	See <load_chunk_to_vram_at> for more information on how each chunk is
;	loaded.
;
; ENTRY:
;	HL - File index
;
; EXIT:
;	A - Error indicator (0 if no error)
;	DE - Address of error string if #A# <> 0
;	CHL - Total size (18 bit) of loaded (or depacked) data
;	EI -
;
; MODIFIES:
;	#ALL#
;
load_file_to_vram:	EXPORT	archive.load_file_to_vram
		IFDEF	MAKE_ARCHIVE_GLOBAL
@load_file_to_vram:	EXPORT	load_file_to_vram		
		ENDIF
		ld	de,(vram_address)
		ld	bc,(vram_address+1)
		jr	load_file_to_vram_at


;=================================================================================================
;
		IFDEF DEBUG
error_archivebusy:		db	"Failed to open archive: can only handle one opened archive at a time!",10,13,"$"
		ELSE
		; This error really should only occur whilst developing a program.
		; So, the release version of the error string is much shorter (takes up fewer bytes)
error_archivebusy:		db	"Archive busy!",10,13,"$"
		ENDIF
error_archiveopenread:		db	"Failed to open or read archive \""
error_archiveopenread_filename:	db	"FILENAME.EXT\"!",10,13,"$"
;
;=================================================================================================
;
;
busy:				db	0
fcb:				ds	SIZEOF_FCB
chunk_size:			dw	0
attributes:			db	0
		IFDEF	ARCHIVE_DEPACK_ROUTINE
packed_buffer_address:		dw	0
		ENDIF
load_address:			dw	0
vram_address:			ds	3,0
toc_size:			ds	2
toc_address:			dw	0
;
;=================================================================================================
		MODULE
