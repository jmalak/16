		PUBLIC	MXGETVERSION
MX_TEXT		SEGMENT	PARA PUBLIC USE16 'CODE'
		ASSUME CS:MX_TEXT, DS:DGROUP, SS:DGROUP
MXGETVERSION:
	mov		ax,128H
	retf
MX_TEXT		ENDS
		END