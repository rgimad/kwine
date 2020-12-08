format PE Console DLL
entry DllEntryPoint

include 'win32ax.inc'

section '.text' code readable executable

proc DllEntryPoint hinstDLL,fdwReason,lpvReserved
	mov	eax,TRUE
	ret
endp

proc multiply_by_3 x
	mov eax, [x]
	push ebx
	mov ebx, 3
	mul ebx
	pop ebx
	ret
endp

section '.idata' import data readable writeable

  library kernel,'KERNEL32.DLL',\
	  user,'USER32.DLL'

  import kernel,\
	 GetLastError,'GetLastError',\
	 SetLastError,'SetLastError',\
	 FormatMessage,'FormatMessageA',\
	 LocalFree,'LocalFree'

  import user,\
	 MessageBox,'MessageBoxA'

section '.edata' export data readable

  export 'test1_dll.dll',\
	 multiply_by_3, 'multiply_by_3'

section '.reloc' fixups data readable discardable
; !!! important: without these lines below, dll will be invalid and won't be loaded 
  if $=$$
    dd 0,8		; if there are no fixups, generate dummy entry
  end if