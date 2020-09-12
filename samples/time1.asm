format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable
    timestamp dd ?

section '.code' code readable executable

entry start

start:
    ;; NOTE cdecl: Registers EAX, ECX, and EDX are caller-saved (here i dont save them), and the rest are callee-saved

    cinvoke time, 0
    mov dword [timestamp], eax

    ; cinvoke printf, <"time = %d",13,10,"time old = %d">, [timestamp], 1588794527
    cinvoke printf, <"time(0) = %d",13,10>, [timestamp]

    cinvoke _getch
    invoke ExitProcess,0

section '.idata' import data readable writeable

library kernel,'kernel32.dll',\
msvcrt,'msvcrt.dll'

import kernel,\
ExitProcess,'ExitProcess'

import msvcrt,\
printf,'printf',\
_getch,'_getch',\
time, 'time'