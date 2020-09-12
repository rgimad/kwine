format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable

        string1: times 256 db 0
        string2: times 256 db 0

        ;len1 dd 0
        ;len2 dd 0

section '.code' code readable executable

entry start

start:
        cinvoke puts, <"Enter string1: ">
        cinvoke gets, string1

        cinvoke puts, <"Enter string2: ">
        cinvoke gets, string2

        ;cinvoke strlen, string1
        ;mov dword [len1], eax

        ;cinvoke strlen, string2
        ;mov dword [len2], eax

        cinvoke strcat, string1, string2

        cinvoke puts, "string1 + string2 = "
        cinvoke puts, string1

        cinvoke _getch
        invoke ExitProcess,0

section '.idata' import data readable writeable

library kernel,'kernel32.dll',\
msvcrt,'msvcrt.dll'

import kernel,\
ExitProcess,'ExitProcess'

import msvcrt,\
puts,'puts',\
gets, 'gets',\
_getch, '_getch',\
strcat, 'strcat'