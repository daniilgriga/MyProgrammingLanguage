; ( type = 4, value = 64
; ( type = 4, value = 68
; ( type = 4, value = 67
call 20:
; ) type = 4, value = 67
20:
 ; ( type = 2, value = 59
; ( type = 2, value = 61
; ( type = 1, value = 25
push 25
; ) type = 1, value = 25
pop [22]
; name = 'bro'
; ) type = 2, value = 61
; ( type = 2, value = 59
; ( type = 2, value = 61
; ( type = 1, value = 10
push 10
; ) type = 1, value = 10
pop [23]
; name = 'dude'
; ) type = 2, value = 61
; ( type = 2, value = 59
; ( type = 2, value = 61
; ( type = 4, value = 67
call 24:
; ) type = 4, value = 67
pop [22]
; name = 'bro'
; ) type = 2, value = 61
; ( type = 2, value = 59
; ( type = 2, value = 61
; ( type = 3, value = 22
push [22]
; name = 'bro'
; ) type = 3, value = 22
pop [22]
; name = 'bro'
; ) type = 2, value = 61
; NOP
; ) type = 2, value = 59
; ) type = 2, value = 59
; ) type = 2, value = 59
; ) type = 2, value = 59
ret
; ) type = 4, value = 68
; ( type = 4, value = 64
; ( type = 4, value = 68
; ( type = 4, value = 67
call 24:
; ) type = 4, value = 67
24:
 ; ( type = 2, value = 59
; ( type = 2, value = 61
; ( type = 1, value = 9
push 9
; ) type = 1, value = 9
pop [25]
; name = 'man'
; ) type = 2, value = 61
; NOP
; ) type = 2, value = 59
ret
; ) type = 4, value = 68
; 64
; ) type = 4, value = 64
; ) type = 4, value = 64
