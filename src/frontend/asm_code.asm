;[1;34m[20]: [0mADDRESS = [0x7f931a532e30], name = 'carti',  lngth = 5, keywrd = 0, added_stts = 1 id_type = 0, host_fnc = 00, cntr_prms = 1, cntr_lcls = 2, offset = 0
;[1;34m[21]: [0mADDRESS = [0x7f931a5356e0], name = 'void',   lngth = 4, keywrd = 0, added_stts = 1 id_type = 5, host_fnc = 20, cntr_prms = 0, cntr_lcls = 0, offset = 0
;[1;34m[22]: [0mADDRESS = [0x7f931a537f90], name = 'bro',    lngth = 3, keywrd = 0, added_stts = 1 id_type = 6, host_fnc = 20, cntr_prms = 0, cntr_lcls = 0, offset = 0
;[1;34m[23]: [0mADDRESS = [0x7f931a53a840], name = 'dude',   lngth = 4, keywrd = 0, added_stts = 1 id_type = 6, host_fnc = 20, cntr_prms = 0, cntr_lcls = 0, offset = 1
;[1;34m[24]: [0mADDRESS = [0x7f931a53d0f0], name = 'eval',   lngth = 4, keywrd = 0, added_stts = 1 id_type = 0, host_fnc = 00, cntr_prms = 1, cntr_lcls = 0, offset = 0
;[1;34m[25]: [0mADDRESS = [0x7f931a53f9a0], name = 'man',    lngth = 3, keywrd = 0, added_stts = 1 id_type = 5, host_fnc = 24, cntr_prms = 0, cntr_lcls = 0, offset = 0
push 5
pop bx
; bx = 5

; tree.c:53:
; ( type = 4, value = 64
; tree.c:53:
; ( type = 4, value = 68
; tree.c:89:
20:
; tree.c:53:
; ( type = 2, value = 59
; tree.c:53:
; ( type = 2, value = 61
; tree.c:53:
; ( type = 1, value = 25
; tree.c:110:
push 25
; tree.c:197:
; ) type = 1, value = 25
; tree.c:137:
; name = 'bro'
pop  [bx + 0]
; tree.c:197:
; ) type = 2, value = 61
; tree.c:53:
; ( type = 2, value = 59
; tree.c:53:
; ( type = 2, value = 61
; tree.c:53:
; ( type = 1, value = 10
; tree.c:110:
push 10
; tree.c:197:
; ) type = 1, value = 10
; tree.c:137:
; name = 'dude'
pop  [bx + 1]
; tree.c:197:
; ) type = 2, value = 61
; tree.c:53:
; ( type = 2, value = 59
; tree.c:53:
; ( type = 4, value = 67
; tree.c:71:
push bx
; tree.c:72:
push bx
; tree.c:73:
push 2
; tree.c:74:
add
; tree.c:53:
; ( type = 4, value = 44
; tree.c:53:
; ( type = 3, value = 23
; tree.c:114:
; name = 'dude'
push [bx + 1]
; tree.c:197:
; ) type = 3, value = 23
; tree.c:100:
; name = 'dude'
pop [bx + 2]
; tree.c:197:
; ) type = 4, value = 44
; tree.c:78:
pop bx

; tree.c:80:
call 24:

; tree.c:82:
pop bx

; tree.c:197:
; ) type = 4, value = 67
; tree.c:192:
; NOP
; tree.c:197:
; ) type = 2, value = 59
; tree.c:197:
; ) type = 2, value = 59
; tree.c:197:
; ) type = 2, value = 59
; tree.c:93:
ret
; tree.c:197:
; ) type = 4, value = 68
; tree.c:53:
; ( type = 4, value = 64
; tree.c:53:
; ( type = 4, value = 68
; tree.c:89:
24:
; tree.c:53:
; ( type = 2, value = 59
; tree.c:53:
; ( type = 2, value = 61
; tree.c:53:
; ( type = 1, value = 9
; tree.c:110:
push 9
; tree.c:197:
; ) type = 1, value = 9
; tree.c:137:
; name = 'man'
pop  [bx + 0]
; tree.c:197:
; ) type = 2, value = 61
; tree.c:192:
; NOP
; tree.c:197:
; ) type = 2, value = 59
; tree.c:93:
ret
; tree.c:197:
; ) type = 4, value = 68
; tree.c:195:
; 64
; tree.c:197:
; ) type = 4, value = 64
; tree.c:197:
; ) type = 4, value = 64
