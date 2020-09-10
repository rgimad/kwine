NEWLIB_INCLUDES=D:\KOSSDK\newlib\libc\include
APP_DYNAMIC_LDS=D:\KOSSDK\newlib/app-dynamic.lds
LIBDIR=D:\KOSSDK\kos32-msys-5.4.0\win32\lib
KWINE_MAIN_TARGET=kwine

CC=kos32-gcc
LD=kos32-ld
OBJCOPY=kos32-objcopy

CCFLAGS=-c -fomit-frame-pointer -I $(NEWLIB_INCLUDES) -I include/ -Wall -Wextra
LDFLAGS=-call_shared -nostdlib --subsystem console -T $(APP_DYNAMIC_LDS) --image-base 0 -L $(LIBDIR)  -lgcc -lapp -lc.dll

BASH=ubuntu1804 run

all: kwine_main
	ubuntu1804 run "mcopy -D o -i kolibri.img kwine ::kwine/kwine"
	qemu-system-x86_64 -fda kolibri.img -m 256 -usb -usbdevice tablet

#libs:
    #

kwine_main:
	$(CC) $(CCFLAGS) kwine.c -o kwine.o
	$(LD) kwine.o -o $(KWINE_MAIN_TARGET) $(LDFLAGS)
	$(OBJCOPY) $(KWINE_MAIN_TARGET) -O binary

clean:
	del *.o
	del $(KWINE_MAIN_TARGET)