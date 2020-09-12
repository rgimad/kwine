format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable

buffer: times 256 db 0
string1  db 'enter a string: ',0
string3  db 'press any key...',0
len dd 0

section '.code' code readable executable

entry start

start:
    ;; NOTE cdecl: Registers EAX, ECX, and EDX are caller-saved (here i dont save them), and the rest are callee-saved

    push string1
    call [puts] ; cdecl
    add esp, 4 ; clean arg

    push buffer
    call [gets] ; cdecl
    add esp, 4 ; clean arg

    push buffer
    call [puts]
    add esp, 4

    push buffer
    call [puts]
    add esp, 4

    push buffer
    call [puts]
    add esp, 4

    call [_getch]

    push 0
    call [ExitProcess]
    ;invoke ExitProcess,0

section '.idata' import data readable writeable

library kernel,'kernel32.dll',\
msvcrt,'msvcrt.dll'

import kernel,\
ExitProcess,'ExitProcess'

import msvcrt,\
puts,'puts',\
gets,'gets',\
_getch,'_getch'