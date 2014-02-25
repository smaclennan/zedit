; Dummy Int 23h/24h handlers prevent
; DOS's default handlers from activating.

	.MODEL large

	.CODE
i23h proc far           ; Ctrl-C handler
	iret            ; Ignore ctrl-c keystroke
i23h endp

i24h proc far           ; Critical-Error handler
	mov     al,03h  ; "Fail": failing I/O function
	iret            ;         returns carry flag set
i24h endp

	PUBLIC _install_ints
_install_ints proc far
	assume  ds:dgroup
	push    ds
	lea     dx,[i23h]       ; Install Ctrl-C handler
	mov     ax,2523h
	int     21h
	lea     dx,[i24h]       ; Install Critical-Error handler
	mov     ax,2524h
	int     21h
	pop     ds
	ret
_install_ints endp

end
