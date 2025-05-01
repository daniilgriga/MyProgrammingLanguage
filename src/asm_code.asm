;[1;34m[20]: [0mADDRESS = [0x7ffcf0e2a9e0], name = 'carti',  lngth = 5, keywrd = 0, added_stts = 1 id_type = 0, host_fnc = 00, cntr_prms = 2, cntr_lcls = 2, offset = 0
;[1;34m[21]: [0mADDRESS = [0x7ffcf0e2d290], name = 'void',   lngth = 4, keywrd = 0, added_stts = 1 id_type = 5, host_fnc = 20, cntr_prms = 0, cntr_lcls = 0, offset = 0
;[1;34m[22]: [0mADDRESS = [0x7ffcf0e2fb40], name = 'loh',    lngth = 3, keywrd = 0, added_stts = 1 id_type = 5, host_fnc = 20, cntr_prms = 0, cntr_lcls = 0, offset = 0
;[1;34m[23]: [0mADDRESS = [0x7ffcf0e323f0], name = 'bro',    lngth = 3, keywrd = 0, added_stts = 1 id_type = 6, host_fnc = 20, cntr_prms = 0, cntr_lcls = 0, offset = 0
;[1;34m[24]: [0mADDRESS = [0x7ffcf0e34ca0], name = 'dude',   lngth = 4, keywrd = 0, added_stts = 1 id_type = 5, host_fnc = 25, cntr_prms = 0, cntr_lcls = 0, offset = 1
;[1;34m[25]: [0mADDRESS = [0x7ffcf0e37550], name = 'eval',   lngth = 4, keywrd = 0, added_stts = 1 id_type = 0, host_fnc = 00, cntr_prms = 1, cntr_lcls = 0, offset = 0
push 5
pop bx
; bx = 5

; src/tree.c:53:
; ( type = 4, value = 64
; src/tree.c:53:
; ( type = 4, value = 68
; src/tree.c:91:
call 20:
hlt
; src/tree.c:94:
20:
; src/tree.c:53:
; ( type = 2, value = 59
; src/tree.c:53:
; ( type = 2, value = 61
; src/tree.c:53:
; ( type = 1, value = 25
; src/tree.c:135:
push 25
; src/tree.c:202:
; ) type = 1, value = 25
; src/tree.c:142:
; name = 'bro'
pop  [bx + 0]
; src/tree.c:202:
; ) type = 2, value = 61
; src/tree.c:53:
; ( type = 2, value = 59
; src/tree.c:53:
; ( type = 2, value = 61
; src/tree.c:53:
; ( type = 1, value = 10
; src/tree.c:135:
push 10
; src/tree.c:202:
; ) type = 1, value = 10
; src/tree.c:142:
; name = 'dude'
pop  [bx + 1]
; src/tree.c:202:
; ) type = 2, value = 61
; src/tree.c:53:
; ( type = 2, value = 59
; src/tree.c:53:
; ( type = 2, value = 61
; src/tree.c:53:
; ( type = 4, value = 67
; src/tree.c:71:
push bx
; src/tree.c:72:
push bx
; src/tree.c:73:
push 2
; src/tree.c:74:
add
; src/tree.c:53:
; ( type = 4, value = 44
; src/tree.c:53:
; ( type = 3, value = 24
; src/tree.c:116:
; name = 'dude'
push [bx + 1]
; src/tree.c:202:
; ) type = 3, value = 24
; src/tree.c:105:
; name = 'dude'
pop [bx + 2]
; src/tree.c:202:
; ) type = 4, value = 44
; src/tree.c:78:
pop bx

; src/tree.c:80:
call 25:

; src/tree.c:82:
pop bx

; src/tree.c:202:
; ) type = 4, value = 67
; src/tree.c:142:
; name = 'bro'
pop  [bx + 0]
; src/tree.c:202:
; ) type = 2, value = 61
; src/tree.c:53:
; ( type = 2, value = 59
; src/tree.c:53:
; ( type = 2, value = 61
; src/tree.c:53:
; ( type = 3, value = 23
; src/tree.c:116:
; name = 'bro'
push [bx + 0]
; src/tree.c:202:
; ) type = 3, value = 23
; src/tree.c:142:
; name = 'bro'
pop  [bx + 0]
; src/tree.c:202:
; ) type = 2, value = 61
; src/tree.c:197:
; NOP
; src/tree.c:202:
; ) type = 2, value = 59
; src/tree.c:197:
; NOP
; src/tree.c:202:
; ) type = 2, value = 59
; src/tree.c:197:
; NOP
; src/tree.c:202:
; ) type = 2, value = 59
; src/tree.c:197:
; NOP
; src/tree.c:202:
; ) type = 2, value = 59
; src/tree.c:98:
ret
; src/tree.c:202:
; ) type = 4, value = 68
; src/tree.c:53:
; ( type = 4, value = 64
; src/tree.c:53:
; ( type = 4, value = 68
; src/tree.c:94:
25:
; src/tree.c:53:
; ( type = 2, value = 59
; src/tree.c:53:
; ( type = 2, value = 61
; src/tree.c:53:
; ( type = 1, value = 9
; src/tree.c:135:
push 9
; src/tree.c:202:
; ) type = 1, value = 9
; src/tree.c:142:
; name = 'dude'
pop  [bx + 1]
; src/tree.c:202:
; ) type = 2, value = 61
; src/tree.c:197:
; NOP
; src/tree.c:202:
; ) type = 2, value = 59
; src/tree.c:98:
ret
; src/tree.c:202:
; ) type = 4, value = 68
; src/tree.c:200:
; 64
; src/tree.c:202:
; ) type = 4, value = 64
; src/tree.c:200:
; 64
; src/tree.c:202:
; ) type = 4, value = 64
