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

; FILE: loader
;	This module joins all the core modules together and should be
;	the first code in any program. That is, the loader source sets the 
;	ORG to $0100 and starts with a jump to the start code. It automatically 
;	includes, initialises and kills the <environment> and can also be told to do
;	this with the other core modules. See the defines <INCLUDE_CHIPS_MODULE>,
;	<INCLUDE_INTERRUPT_MODULE>, <INCLUDE_INPUTDEVICES_MODULE>, 
;	<INCLUDE_MAPPER_MODULE> and <INCLUDE_VDP_MODULE> on how to get
;	specific core modules automatically included, initialised and killed.
;	Note that this module also always includes <msxdefs> and <macros>, so
;	you'll never need to include these yourself either when using the loader
;	module.
;	:
;	An example of a minimalistic, and completely useless, program using the 
;	loader module:@
;	:
;:		; Do NOT INCLUDE any other files before loader.asm
;:		INCLUDE	<path_to_framework>\loader.asm
;:
;:		; Here you could INCLUDE other files
;:		
;:main:
;:		xor	a	; 0 = no error
;:		ret
;
; COPYRIGHT:
;	This module has been released by Eli-Jean Leyssens under the MIT License;
;	please see the top of the source file(s) for the full copyright statement.

;
; DEFINE: 	INCLUDE_CHIPS_MODULE
;	Define this if you want the loader module to automatically include,
;	initialise and kill the <chips> module.
;
; DEFINE:	INCLUDE_DEBUG_MODULE
;	Define this AND also define DEBUG if you want the loader module to
;	automatically include, initialise and kill the <debug> module. If
;	you only define INCLUDE_DEBUG_MODULE, but not DEBUG then the debug
;	module will not be included, nor initialised and killed.
;
; DEFINE:	INCLUDE_INTERRUPT_MODULE
;	Define this if you want the loader module to automatically include,
;	initialise and kill the <interrupt> module.
;
; DEFINE:	INCLUDE_INPUTDEVICES_MODULE
;	Define this if you want the loader module to automatically include,
;	initialise and kill the <inputdevices> module.
;
; DEFINE:	INCLUDE_MAPPER_MODULE
;	Define this if you want the loader module to automatically include,
;	initialise and kill the <mapper> module.
;
; DEFINE:	INCLUDE_VDP_MODULE
;	Define this if you want the loader module to automatically include,
;	initialise and kill the <vdp> module.
;

		org   	$0100		
		jp	loader.start

		include	msxdefs.asm
		include	macros.asm

		IFDEF	INCLUDE_CHIPS_MODULE
		include	chips.asm
		ENDIF
		
		IFDEF	INCLUDE_DEBUG_MODULE
		IFDEF	DEBUG
		include	debug.asm
		ENDIF
		ENDIF

		include	environment.asm
		
		IFDEF	INCLUDE_INTERRUPT_MODULE
		include	interrupt.asm
		ENDIF
		
		IFDEF	INCLUDE_INPUTDEVICES_MODULE
		include inputdevices.asm
		ENDIF
		
		IFDEF	INCLUDE_MAPPER_MODULE		
		include mapper.asm
		ENDIF
		
		IFDEF	INCLUDE_VDP_MODULE
		include vdp.asm
		ENDIF
		
		MODULE	loader
; FUNCTION: start
;	This is the entry function for the program. Here's what it does:@
;	- Initalise all included modules
;	- Call your <main> routine.
;	- Kill all included modules. The <environment> module is of course
;	  killed last, since that exits the program. See <kill_environment>
;	  for more information about this.
;	
;	If an error is returned it is passed on to <kill_environment> so that
;	it can be printed. Note that not only your <main> routine can return
;	an error, but also the initialisation routine of any of the included
;	modules. Note that if the initialisation routine of a module returns
;	an error then the loader will not anymore initialisation routines, nor
;	the <main> routine, but will instead immediately start to call all the
;	kill routines.
;
; ENTRY:
;	N/A
;
; EXIT:
;	N/A
;
; MODIFIES:
;	N/A
;

; FUNCTION: main
;	If none of the included modules returns an error then the start routine 
;	will make a call to the routine called "main". Normally this will be the 
;	main routine of your program. Make sure this routine's name is available 
;	globally. You can return an error if you want. See the EXIT parameters 
;	for a description of that.
;
; NOTE:
;	There is *no* routine named "main" in the loader module. This description
;	is merely here to let you know what your main routine should look like.
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
start:
		call	init_environment
		or	a
		jr	nz, .error

		IFDEF	INCLUDE_DEBUG_MODULE
		IFDEF	DEBUG
		call	init_debug
		or	a
		jr	nz, .error
		ENDIF
		ENDIF

		IFDEF	INCLUDE_CHIPS_MODULE
		call	init_chips
		or	a
		jr	nz, .error
		ENDIF

		IFDEF	INCLUDE_VDP_MODULE
		call	init_vdp
		or	a
		jr	nz, .error
		ENDIF

		IFDEF	INCLUDE_MAPPER_MODULE
		call	init_mapper
		or	a
		jr	nz, .error
		ENDIF

		IFDEF	INCLUDE_INTERRUPT_MODULE
		call	init_interrupt
		or	a
		jr	nz, .error
		ENDIF
		
		IFDEF	INCLUDE_INPUTDEVICES_MODULE
		call	init_inputdevices
		or	a
		jr	nz, .error
		ENDIF

		call	main
		or	a
		jr	nz, .error

		ld	de, $0000		; No error
.error:
		push	de
		
		IFDEF	INCLUDE_INPUTDEVICES_MODULE
		call	kill_inputdevices
		ENDIF		
		IFDEF	INCLUDE_INTERRUPT_MODULE
		call	kill_interrupt
		ENDIF		
		IFDEF	INCLUDE_MAPPER_MODULE		
		call 	kill_mapper
		ENDIF
		IFDEF	INCLUDE_VDP_MODULE
		call	kill_vdp
		ENDIF
		IFDEF	INCLUDE_CHIPS_MODULE
		call	kill_chips
		ENDIF
		IFDEF	INCLUDE_DEBUG_MODULE
		IFDEF	DEBUG
		call	kill_debug
		ENDIF
		ENDIF
		pop	de
		jp	kill_environment	; This expects DE to contain the address of an error-
						; string or $0000 for no error.
		MODULE
