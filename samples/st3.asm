format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable

string1 db 'Privet123',0
string2 db '123456789123',0
fmt1     db 'length1 = %d',13,10,0
fmt2     db 'length2 = %d',13,10,0
string3  db 'press any key...',0
len dd 0

section '.code' code readable executable

entry start

start:
    ;; NOTE cdecl: Registers EAX, ECX, and EDX are caller-saved (here i dont save them), and the rest are callee-saved

    push string1
    call [strlen] ; cdecl
    add esp, 4 ; clean arg
    mov dword [len], eax
    push [len]
    push fmt1
    call [printf] ; cdecl
    add esp, 8 ; clean arg

    push string2
    call [strlen] ; cdecl
    add esp, 4 ; clean arg
    mov dword [len], eax
    push [len]
    push fmt2
    call [printf] ; cdecl
    add esp, 8 ; clean arg

    push string3
    call [printf]
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
printf,'printf',\
_getch,'_getch',\
strlen, 'strlen'