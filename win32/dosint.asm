; Dummy Int 1Bh/23h/24h handlers prevent
; DOS's default handlers from activating.
; (Int 1Bh is issued by Int 09h handler
; and chained by DOS)

	.MODEL large

	.DATA
old_i1bh dd ?           ; Saved Int 1bh vector

	.CODE
; SAM label i1bh far          ; Disable ".display
i23h proc far
proc i23h far           ; Ctrl-C handler
	iret            ; Ignore ctrl-c keystroke
endp

proc i24h far           ; Critical-Error handler
	mov     al,03h  ; "Fail": failing I/O function
iret            ;         returns carry flag set
endp

proc install_i1Bh_i23h_i24h
	assume  ds:dgroup
	push    ds
	mov     ax,351bh        ; Save Int 1bh vector
	int     21h
	push    es bx
	pop     [old_i1bh]
	mov     ax,cs
	mov     ds,ax
	assume  ds:@code
	lea     dx,[i1bh]       ; Install Int 1bh vector
	mov     ax,251bh
	int     21h
	lea     dx,[i23h]       ; Install Ctrl-C handler
	mov     ax,2523h
	int     21h
	lea     dx,[i24h]       ; Install Critical-Error handler
	mov     ax,2524h
	int     21h
	pop     ds
	ret
endp

proc restore_i1bh
	assume  ds:dgroup       ; (DOS auto-restores Int 23h and
	push    ds              ;  24h vectors on termination)
	lds     dx,[old_i1bh]
	assume  ds:nothing
	mov     ax,251bh
	int     21h
	pop     ds
	assume  ds:dgroup
	ret
endp
