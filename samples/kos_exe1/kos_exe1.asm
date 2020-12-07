format PE CONSOLE
include 'WIN32AX.INC'

section '.data' data readable writeable

text    db  "It looks like you have just compiled    "
        db  "your first program for KolibriOS.       "
        db  "                                        "
        db  "Congratulations!                        ", 0
 
title   db  "Example application", 0
 

section '.code' code readable executable

entry start

start:
        call    draw_window             ; draw the window
 
; After the window is drawn, it's practical to have the main loop.
; Events are distributed from here.
 
event_wait:
        mov     eax, 10                 ; function 10 : wait until event
        int 0x40                           ; event type is returned in eax
 
        cmp     eax, 1                  ; Event redraw request ?
        je      red                     ; Expl.: there has been activity on screen and
                                        ; parts of the applications has to be redrawn.
 
        cmp     eax, 2                  ; Event key in buffer ?
        je      key                     ; Expl.: User has pressed a key while the
                                        ; app is at the top of the window stack.
 
        cmp     eax, 3                  ; Event button in buffer ?
        je      button                  ; Expl.: User has pressed one of the
                                        ; applications buttons.
 
        jmp     event_wait
 
;  The next section reads the event and processes data.
 
red:                                    ; Redraw event handler
        call    draw_window             ; We call the window_draw function and
        jmp     event_wait              ; jump back to event_wait
 
key:                                    ; Keypress event handler
        mov     eax, 2                  ; The key is returned in ah. The key must be
        int 0x40                           ; read and cleared from the system queue.
        jmp     event_wait              ; Just read the key, ignore it and jump to event_wait.
 
button:                                 ; Buttonpress event handler
        mov     eax,17                  ; The button number defined in window_draw
        int 0x40                           ; is returned to ah.
 
        cmp     ah,1                    ; button id=1 ?
        jne     noclose
        mov     eax,-1                  ; Function -1 : close this program
        int 0x40
 
noclose:
        jmp     event_wait              ; This is for ignored events, useful at development
 
;  *********************************************
;  ******  WINDOW DEFINITIONS AND DRAW  ********
;  *********************************************
;
;  The static window parts are drawn in this function. The window canvas can
;  be accessed later from any parts of this code (thread) for displaying
;  processes or recorded data, for example.
;
;  The static parts *must* be placed within the fn 12 , ebx = 1 and ebx = 2.
 
draw_window:
        mov     eax, 12                 ; function 12: tell os about windowdraw
        mov     ebx, 1                  ; 1, start of draw
        int 0x40
 
        mov     eax, 0                  ; function 0 : define and draw window
        mov     ebx, 100 * 65536 + 300  ; [x start] *65536 + [x size]
        mov     ecx, 100 * 65536 + 120  ; [y start] *65536 + [y size]
        mov     edx, 0x14ffffff         ; color of work area RRGGBB
                                        ; 0x02000000 = window type 4 (fixed size, skinned window)
        mov     esi, 0x808899ff         ; color of grab bar  RRGGBB
                                        ; 0x80000000 = color glide
        mov     edi, title
        int 0x40
 
        mov     ebx, 25 * 65536 + 35    ; draw info text with function 4
        mov     ecx, 0x224466
        mov     edx, text
        mov     esi, 40
        mov     eax, 4
 
  .newline:                             ; text from the DATA AREA
        int 0x40
        add     ebx, 10
        add     edx, 40
        cmp     byte[edx], 0
        jne     .newline
 
        mov     eax, 12                 ; function 12:tell os about windowdraw
        mov     ebx, 2                  ; 2, end of draw
        int 0x40
 
        ret
        push 0
        call [ExitProcess]

section '.idata' import data readable writeable

library kernel,'kernel32.dll';,\
;msvcrt,'msvcrt.dll'

import kernel,\
ExitProcess,'ExitProcess'

;import msvcrt,\
;printf,'printf',\
;_getch,'_getch',\
;strlen, 'strlen'