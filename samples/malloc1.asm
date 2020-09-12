format PE CONSOLE
include 'C:\Program Files (x86)\fasmw17322\INCLUDE\WIN32AX.INC'

section '.data' data readable writeable
          ; db - reserve byte, dw - reserve word, dd - reserve dword, dq - reserve qword

  arr     dd ? ; pointer to array
  n       dd 10 ; array length
  x       dd ?

; arr - address of pointer to array
; [arr] - pointer to array


section '.code' code readable executable

entry start

start:
        cinvoke malloc, n*4
        mov [arr], eax

        xor ecx, ecx
        while1:
                cmp ecx, [n]
                je while1_end

                mov eax, [arr]
                mov dword [eax + 4*ecx], ecx
                add dword [eax + 4*ecx], 2
                inc ecx
                jmp while1
        while1_end:

        call show_array
        cinvoke memset, [arr], 0, n*4
        call show_array
        jmp endprog

show_array:
        cinvoke printf, <13,10,"Test array = ">
        xor ecx, ecx
        while2:
                cmp ecx, [n]
                je while2_end

                mov eax, [arr]
                mov eax, dword [eax + 4*ecx]
                mov [x], eax

                push ecx
                cinvoke printf, " %d ", [x]
                pop ecx

                inc ecx
                jmp while2

        while2_end:
                ret

endprog:
        cinvoke free, [arr]

        cinvoke _getch
        invoke ExitProcess,0

section '.idata' import data readable writeable

library kernel,'kernel32.dll',\
msvcrt,'msvcrt.dll'

import kernel,\
ExitProcess,'ExitProcess'

import msvcrt,\
printf,'printf',\
memset,'memset',\
malloc,'malloc',\
free,'free',\
_getch, '_getch'