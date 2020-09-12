format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable

string1: times 256 db 0
pos1 dd 0
pos2 dd 0

section '.code' code readable executable

entry start

start:
    ;; NOTE cdecl: Registers EAX, ECX, and EDX are caller-saved (here i dont save them), and the rest are callee-saved

    cinvoke printf, <"Enter a string: ",0>
    cinvoke gets, string1
    ;cinvoke puts, string1

    cinvoke strchr, string1, 49
    sub eax, string1
    mov dword [pos1], eax

    cinvoke strrchr, string1, 49
    sub eax, string1
    mov dword [pos2], eax

    cinvoke printf, <"first '1' pos = %d",13,10,"last '1' pos = %d">, [pos1], [pos2]

    cinvoke _getch
    invoke ExitProcess,0

section '.idata' import data readable writeable

library kernel,'kernel32.dll',\
msvcrt,'msvcrt.dll'

import kernel,\
ExitProcess,'ExitProcess'

import msvcrt,\
printf,'printf',\
gets,'gets',\
puts,'puts',\
_getch,'_getch',\
strchr, 'strchr',\
strrchr, 'strrchr'