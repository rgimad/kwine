format PE CONSOLE
include 'WIN32AX.INC'

section '.data' data readable writeable

hed db 'Hello World',0     ; название программы

section '.code' code readable executable

entry start

start:
        mov  eax,40      ; вызов системной функции 40: установить маску для ожидаемых событий (полный список функций см. в sysfuncr.txt описание событий в конце документа)
        mov  ebx,0x7     ; программа будет реагировать только на события, которые мы разрешим этой функцией.
        int 0x40         ; По умолчанию ( если эту функцию не вызывать) программе будут приходить сообщения о нажатии кнопки в программе,
    red_win:                  ; рисуем окно программы
        call draw_window  ; вызов процедуры draw_window (см. ниже)

still:                    ; нарисовали - переходим в основной цикл программы 
        mov eax,10        ; функция 10: ждем события, которые мы выше определили функцией 40
        int 0x40
                          ; событие произошло, теперь проверяем, какое именно
        cmp al,0x1        ; если система говорит, что нужно перерисовать окно (изменились его размер или положение)
        jz  red_win       ; перепрыгиваем на процедуру red_win (перерисовываем окно) и ждем события дальше
        cmp al,0x3        ; если была нажата кнопка в программе,
        jz  button        ; то прыгаем на процедуру button

key:                      ; если это и не кнопка и не перерисовка, значит остается одно - клавиша на клавиатуре
        mov  eax,2        ; функция 2: считываем код нажатой клавиши и забываем, потому как мы не используем клавиатуру в этой программе
        int  0x40
        jmp still         ; сделали свое дело и опять возвращаемся в основной цикл программы и ждем событий

button:                   ; процедура обрабоки кнопок в программе
        mov eax,17        ; функция 17: получить номер нажатой кнопки
        int  0x40
        shr  eax,8        ; сдвигаем регистр eax на 8 бит вправо, чтобы получить номер нажатой кнопки

        cmp eax,1
        jne  no_exit      ; если это не кнопка 1 (зарезервирована системой как кнопка закрытия программы), пропускаем 2 следующие строчки кода

        ;or   eax,-1       ;(равносильно команде mov eax,-1, но более оптимально - функция -1: завершение программы)
        ;int  0x40

 no_exit:                  ; и переходим сюда, где проверяем, какая же все таки кнопка была нажата (в этой программе других кнопок нет, 
        jmp  still        ; поэтому возвращаемся в основной цикл программы и опять ждем событий)

draw_window:              ; процедура отрисовки окна приложения
        mov eax,12        ; функция 12,
        mov ebx,1         ; подфункция 1: сообщить системе, что мы хотим нарисовать окно
        int 0x40

       xor  eax,eax             ; обнуляет регистр eax (по сути - то же, что и mov eax,0, но более оптимально) - функция 0: нарисовать окно
       mov  ebx,50*65536+180    ; [координата окна по оси x]*65536 + [размер по оси x]
       mov  ecx,30*65536+100    ; [координата по оси y]*65536 + [размер по оси y]
       mov  edx,0x33AABBCC      ; цвет рабочей области окна RRGGBB, первые 2 цифры (33) определяют тип и параметры окна,
                                ; тут - 3 тип (окно со скином изменяемого размера) и координнаты окна отсчитываются он клиентской области
       mov  edi,hed             ; заголовок программы
       int 0x40
       
; здесь рисуются все основные элементы программы, если они есть, в нашем случае это просто пустое окно

       mov eax,12          ; функция 12,
       mov ebx,2           ; подфункция 2 - сообщить системе, что мы закончили отрисовку окна.
       int 0x40

    ret

section '.idata' import data readable writeable

;library kernel,'kernel32.dll',\
;msvcrt,'msvcrt.dll'

;import kernel,\
;ExitProcess,'ExitProcess'

;import msvcrt,\
;puts,'puts',\
;gets,'gets',\
;_getch,'_getch'