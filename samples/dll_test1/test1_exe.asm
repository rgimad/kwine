format PE Console
entry start

include 'win32ax.inc'

section '.data' data readable writeable
res   dd   ?

section '.text' code readable executable
CONST_NUM equ 9

    start:
        invoke multiply_by_3, CONST_NUM
        mov dword [res], eax
        cinvoke printf, <"%d * 3 = %d",13,10>, CONST_NUM, [res]
        cinvoke getchar
        invoke ExitProcess, 0


section '.idata' import data readable writeable
    library kernel, 'kernel32.dll',\
            msvcrt, 'msvcrt.dll',\
            test1_dll, 'test1_dll.dll'

    import kernel,\
           ExitProcess, 'ExitProcess'

    import msvcrt,\
           getchar, 'getchar',\
           printf, 'printf'

    import test1_dll,\
           multiply_by_3, 'multiply_by_3'
