;************************************************************************
;*									*
;*		 The software found in this file is the			*
;*		      Copyright of Sean MacLennan			*
;*			 All rights reserved.				*
;*									*
;************************************************************************

; This file contains the PC assembler routines.
; 	_Outch		output a char with repeat		PC only
;	_Chattr		change bold attr in line		PC only
;	_Tforce		move cursor				PC only
;	_Tclrwind	clear window and home cursor		PC only
;	_Tkbrdy		is a char waiting
;	_Wait		pause for 2 clock ticks
;	_C_error	critical error handler
;	_Getvtype	get the video adapter type
;	_Getcursor	get the cursor shape
;	_Setcursor	set the cursor shape
;	_IBMalloc	DOS malloc
;	_IBMfree	DOS free

; It also contains all the mouse routines
;	_Mouse_init	initialize the mouse
;	_Mouse_bounds	set the mouse bounds
;	_Mouse_cursor	set/reset mouse cursor
;	_Get_mouse	get mouse state
;	_Reset_mouse	reset the mouse when done


CGA		equ	3			; should be same in z.h
INMACRO		equ	2			; should be same in z.h
EOF		equ	-1

ibmasm_text	segment	word public 'code'
ibmasm_text	ends
_data		segment	word public 'data'
_data		ends
const		segment	word public 'const'
const		ends
_bss		segment	word public 'bss'
_bss		ends

dgroup		group	const, _bss, _data
		assume  cs:ibmasm_text, ds:dgroup, ss:dgroup

public		_Mouse
public		_Ctype
public		_Vtype
ifdef PC
public		_Attr
endif

extrn		_Border:far
extrn		_Screen:far

extrn		_Colmax:word			; maximum columns
extrn		_Rowmax:word			; maximum rows
extrn		_Tstart:word			; start of row and cols
extrn		_Mstate:word			; macro state
extrn		_Kbrdy:	word			; pushed key
ifdef PC
extrn		_Prow:	word			; current row and column
extrn		_Pcol:	word
extrn		_Lptr:	dword			; Lines
endif

_data      	segment
_Mouse		dw	0			; TRUE if mouse installed
ShowMouse	dw	0			; TRUE if mouse displayed
state		dw	0			; current state of mouse

_Ctype		dw	0b0ch, 010ch		; Ctype[ 2 ]
_Vtype		db	0			; video type
ifdef PC
_Attr		db	0			; current attributes
endif
_data      	ends

const		segment
Mstate_seg	dw seg _Mstate
Kbrdy_seg	dw seg _Kbrdy
T_SEG		dw seg _Prow
ifdef PC
PC_SEG		dw seg _Lptr
endif
const		ends

ibmasm_text	segment
		assume	cs:ibmasm_text

ifdef PC

;*****************************************************************************
;void Outch( char ochar, int repeat )
;	Writes out 'ochar', after adding the current attributes, 'repeat'
;	times to the current video screen position.
;	If CGA, adjust for rescan while writing. Otherwise, copy directly.
; 	The CGA variation can only be done fast enough in assembly.
;*****************************************************************************
		public	_Outch
_Outch		proc	far
		push	bp			; bp = sp
		mov	bp, sp
		push	di
		mov	al, [bp+6]		; Move attr, ochar into ax
		mov	ah, byte ptr _Attr
		mov	cx, [bp+8]		; repeat cnt for Tcleol, tabs

		mov	es, T_SEG		; es:di = Lptr[ Prow ][ Pcol ]
		mov	bx, word ptr es:_Prow
		shl	bx, 1
		shl	bx, 1
		mov	dx, word ptr es:_Pcol
		shl	dx, 1
		mov	es, PC_SEG
		les	di, dword ptr es:_Lptr[bx]
		add	di, dx

		cld				; DF = 0 (direction flag)
		cmp	byte ptr _Vtype, CGA
		ja	notcga

		mov	bx, ax			; save ax in bx
; For CGA we must wait for retrace.
owait0:		mov	dx, 03dah		; setup for horizontal checks
		sti
		nop
		cli
owait1:		in	al, dx			; wait till horizontal active
		shr	al, 1
		jc	owait1
		cli
owait2:		in	al, dx			; wait till retrace
		shr	al, 1
		jnc	owait2

		mov	ax, bx			; Load character and
		stosw				;   move to video memory
		sti
		loop	owait0			; repeat
		jmp	short getout		; Done for CGA

notcga:		rep	stosw			; Non-CGA - no waiting

getout:		pop	di
		pop	bp
		ret
_Outch		endp



;*****************************************************************************
;void Chattr( WORD **line, int attr )
;	Change the bold bit for the entire screen line 'line' to 'attr'.
;*****************************************************************************
		public	_Chattr
_Chattr		proc	far
		push	bp			; bp = sp
		mov	bp, sp
		push	di
		les 	di, [bp+6]		; set di to video line
		mov 	cx, 80			; do to end of line
		mov	bh, [bp+10]		; save attr in bh
		cld				; forward

		cmp	byte ptr _Vtype, CGA
		ja	nowait

; For CGA we must wait for retrace.
iwait0:		mov	dx, 03dah		; setup for horizontal checks
		sti
		nop
		cli
iwait1:		in	al, dx			; wait till horizontal active
		shr	al, 1
		jc	iwait1
		cli
iwait2:		in	al, dx			; wait till retrace
		shr	al, 1
		jnc	iwait2

; NOTE: the loop must be as tight as possible for CGA
;		we don't have much time!!!
		mov	ax, es:[di]		; get current character
		and 	ah, 0f7h		; clear the bold bit
		or	ah, bh			; add if needed
		stosw				; put it back and inc
		sti
		loop	iwait0			; loop till done
		jmp	short done

nowait:		mov	ax, es:[di]		; get current character
		and	ah, 0f7h		; clear the bold bit
		or	ah, bh			; add if needed
		stosw				; put it back and inc
		loop	nowait			; loop till done

done:		pop	di
		pop	bp
		ret	
_Chattr		endp


;*****************************************************************************
;void Tforce( void )
;	move the cursor to Prow, Pcol
;*****************************************************************************
		public	_Tforce
_Tforce		proc	far
		mov	es, T_SEG
		mov	dh, byte ptr es:_Prow
		mov	dl, byte ptr es:_Pcol
		mov	bh, 0
		mov	ah, 2
		int	10h
		ret
_Tforce		endp


;*****************************************************************************
;void Tclrwind( void )
;	Clear the screen to the correct attributes and home cursor
;	If border set, redisplay the border
;*****************************************************************************
		public	_Tclrwind
_Tclrwind	proc	far
		mov	bh, byte ptr _Attr
		mov	ax, 600h
		mov	cx, 0			;  0,  0
		mov	dx, 174fh		; 23, 79
		int	10h
		mov	ax, 600h		; clear the "PAW"
		mov	bh, 7			; always white on black
		mov	cx, 1800h		; 24,  0
		mov	dx, 184fh		; 24, 79
		int	10h

		mov	es, T_SEG		; home cursor
		mov	ax, es:_Tstart
		mov	es:_Prow, ax
		mov	es:_Pcol, ax
		call	_Tforce

		cmp	es:_Tstart, 0		; if border set
		je	retclear
		call	_Border			;    Border()
retclear:	ret
_Tclrwind	endp


endif

;*****************************************************************************
;void Tkbrdy( void )
;	This routine returns TRUE if a character is waiting, or Mstate is
;	INMACRO.
;	If a character is waiting, it is stored in _Kbrdy
;	Handles extended commands if necessary.
;	This is done in assembler because function 6 sets the zero flag and
;	none of the Microsoft MSDOS interface commands allow us to check the
;	zero flag!
;*****************************************************************************
		public	_Tkbrdy
_Tkbrdy		proc	far
		mov	es, Mstate_seg		; if Mstate == INMACRO
		cmp	es:_Mstate, INMACRO	;	return TRUE
		je	gotone
		mov	es, Kbrdy_seg		; if Kbrdy != EOF
		cmp	es:_Kbrdy, EOF		;	return TRUE
		jne	gotone
		mov	ah, 6			; non/blocking char input
		mov	dl, 255
		int	21h
		jz	none			; no chars waiting
		mov	ah, 0			; just in case regular
		cmp	al, 0			; extended ?
		jne	regular
		mov	ah, 7			; read extended char
		int	21h
		mov	ah, 0			; turn to META by
		and	al, 07fh		;	stripping hi bit
		add	ax, 128			;	and adding 128
regular:	mov	es:_Kbrdy, ax		; save the char in Kbrdy
gotone:		mov	ax, 1			; return char waiting
			ret
none:		mov	ax, 0			; return no char waiting
		ret
_Tkbrdy		endp


;*****************************************************************************
;void Wait()
;	This routine waits for aprox. 2 clock ticks (18.2 clock ticks per
; 	minute). It dosen't handle wrapping at midnight...
;*****************************************************************************
		public	_Wait
_Wait		proc	far
		mov	ah, 0			; get bios time (cx:dx)
		int	1ah
		mov	bx, dx
		add	bx, 2
wloop:		mov	ah, 0
		int	1ah
		cmp	dx, bx
		jb	wloop
		ret	
_Wait		endp


;*****************************************************************************
;void interrupt far C_error( void )
;	This is the critical error handler (int 24).
;	All it does is set al to 3 to tell dos to ignore the error.
;	The error code will be returned in errno.
;	Dos restores the "correct" error handler on program termination.
;*****************************************************************************
		public	_C_error
_C_error	proc	far
		mov	al, 3
		iret
_C_error	endp


;*****************************************************************************
;unsigned Getvtype( void )
;*****************************************************************************
		public	_Getvtype
_Getvtype	proc	far
		mov	ah, 15			; get the video type
		int	10h
		mov	byte ptr _Vtype, al
		ret
_Getvtype	endp


;*****************************************************************************
;unsigned Getcursor( void )
;*****************************************************************************
		public	_Getcursor
_Getcursor	proc	far
		cmp	byte ptr _Vtype, 7
		je	getcursor
		mov	word ptr _Ctype, 0607h	; non-mono Ctype[0]
getcursor:	mov	ah, 3			; get the current cursor
		mov	bh, 0
		int	10h
		push	cx			; save cursor shape
ifdef PC
		mov	cl, 8			; move dh to dl
		shr	dx, cl
		push	dx			; type for screen call
		call	_Screen
		add	sp, 2
endif
		pop	ax			; and restore
		ret
_Getcursor	endp



;*****************************************************************************
;void Setcursor( unsigned type )
;*****************************************************************************
		public	_Setcursor
_Setcursor	proc	far
		push	bp
		mov	bp, sp
		mov	ah, 1
		mov	cx, word ptr [bp+6]
		int	10h
		pop	bp
		ret
_Setcursor	endp


;*****************************************************************************
;char *IBMalloc( unsigned size )
;*****************************************************************************
		public	_IBMalloc
_IBMalloc	proc	far
		push	bp
		mov	bp, sp
		; convert size from bytes to paragraphs
		mov	bx, word ptr [bp+6]	;size
		add	bx, 15			;round up
		mov	cl, 4
		shr	bx, cl
		mov	ah, 48h
		int	21h
		jnc	retok
		sub	ax, ax
		cwd
		jmp	short aret
retok:		mov	dx, ax
		sub	ax, ax
aret:		pop	bp
		ret
_IBMalloc	endp


;*****************************************************************************
;void IBMfree( char *ptr )
;*****************************************************************************
		public	_IBMfree
_IBMfree	proc	far
		push	bp
		mov	bp, sp
		mov	ax, word ptr [bp+8]
		mov	es, ax
		mov	ah, 49h
		int	21h
		pop	bp
		ret
_IBMfree	endp
	

;*****************************************************************************
;BOOLEAN Mouse_init( void )
;	Initialize the mouse:
;		- mouse position reset to center of screen
;		- mouse cursor off
;		- default mouse cursor used
;		- light pen emulation on
;		- default movement ratios used
;		- bounded to screen size
;Returns TRUE if mouse installed.
;*****************************************************************************
		public	_Init_mouse
_Init_mouse	proc	far
		mov	ax, 0		;initialize the mouse
		int	33h
		mov	_Mouse, ax	;save result
		cmp	ax, 0		;is mouse installed
		je	retmouse	;no

		mov	ax, 10		;set the attribute cursor
		mov	bx, 0
ifdef WARREN
		mov	cx, 7700h	;AND mask
		mov	dx, 77e8h	;XOR mask
else
		mov	cx, 0ffffh	;AND mask
		mov	dx, 07000h	;XOR mask
endif
		int	33h
		call	far ptr _Mouse_bounds	;set the mouse bounds

retmouse:	mov	ax, _Mouse
		ret	
_Init_mouse	endp


;*****************************************************************************
;void Mouse_bounds( void )
;	set the mouse bounds for Zedit
;	compensates for border
;*****************************************************************************
		public	_Mouse_bounds
_Mouse_bounds	proc	far
		cmp	word ptr _Mouse, 0	;mouse set?
		je	mbret			;no, just return

		mov	bx, 0			;offset is 0
		mov	es, T_SEG		;set es
ifdef PC
		cmp	word ptr es:_Tstart, 0	;if border set
		je	setbounds
		mov	bx, 8			;offset is 8
endif
setbounds:	mov	dx, es:_Colmax		;maxcol =
		dec	dx			;  (Colmax - 1) << 3 - offset
		mov	cl, 3
		shl	dx, cl
		sub	dx, bx
		mov	cx, bx			;mincol = offset
		mov	ax, 7
		int	33h
		mov	dx, es:_Rowmax		;maxrow = (Rowmax - 2) << 3;
		sub	dx, 2
		mov	cl, 3
		shl	dx, cl
		mov	cx, bx			;minrow = offset
		mov	ax, 8
		int	33h
mbret:		ret
_Mouse_bounds	endp


;*****************************************************************************
;void Mouse_cursor( BOOLEAN new )
;	turn the mouse cursor on or off if necessary.
;	only turn mouse on or off if state has changed and Showmouse set
;*****************************************************************************
		public	_Mouse_cursor
_Mouse_cursor	proc	far
		push	bp
		mov	bp, sp
		cmp	word ptr ShowMouse, 0
		je	mcret
		mov	ax, word ptr [bp+6]
		cmp	word ptr state, ax	;has state changed?
		je	mcret
		mov	state, ax		;set state to new
		cmp	ax, 1			;ON  ax=1
		je	setstate
		mov	ax, 2			;OFF ax=2
setstate:	int	33h

mcret:		pop	bp
		ret	
_Mouse_cursor	endp


;*****************************************************************************
;int Get_mouse( int *row, int *col )
;	sets row and col to the current row and col and returns buttons
;	pressed
;*****************************************************************************
		public	_Get_mouse
_Get_mouse	proc	far
		push	bp
		mov	bp, sp
		mov	ax, 3			;get mouse state
		int	33h
		push	bx			;save button status

		mov	ax, cx			;*col = cx >> 3 - Tstart
		mov	cl, 3
		shr	ax, cl
		mov	es, T_SEG
		sub	ax, es:_Tstart
		les	bx, dword ptr [bp+10]
		mov	word ptr es:[bx], ax

		shr	dx, cl			;*row = dx >> 3
		les	bx, dword ptr [bp+6]
		mov	word ptr es:[bx], dx

		;if ShowMouse is FALSE and the mouse has moved
		;	set ShowMouse to TRUE and display mouse cursor
		cmp	word ptr ShowMouse, 0
		jne	gmret
		cmp	dx, 12			;dx=row
		jne	setem
		mov	cx, 40
		mov	es, T_SEG
		sub	cx, es:_Tstart
		cmp	ax, cx
		je	gmret
setem:		mov	word ptr ShowMouse, 1
		mov	ax,1
		push	ax
		call	_Mouse_cursor
		add	sp, 2

gmret:		pop	ax			;restore button status
		pop	bp
		ret	
_Get_mouse	endp


;*****************************************************************************
;void Reset_mouse( void )
;	reset the mouse on exit
;*****************************************************************************
		public	_Reset_mouse
_Reset_mouse	proc	far
		mov	ax, 0
		int	33h
		ret
_Reset_mouse	endp

ibmasm_text	ends
		end
