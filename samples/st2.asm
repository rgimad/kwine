format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable

string1 db 'Privet123',0
string2 db '123456789123',0
len dd 0

section '.code' code readable executable

entry start

start:

 cinvoke strlen, string1
 mov dword [len], eax
 cinvoke printf, <"length1 = %d", 13,10>, [len]

 cinvoke strlen, string2
 mov dword [len], eax
 cinvoke printf, <"length2 = %d", 13,10>, [len]

 cinvoke printf, <"Press any key...",13,10,0>
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
strlen, 'strlen'