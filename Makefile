MKLINUX=make -f Makefile.linux
MKDOS=make -f Makefile.dos
EXELINUX=licmd
EXEDOS=licmd.exe
TESTCMD="uname"
SYSTEM=$(shell $(TESTCMD))

.PHONE: all clean

all:
ifeq ($(SYSTEM), Linux)
	cd src && $(MKLINUX) 
else 
	@echo Never mind the above errors. It means you are under DOS.
	cd src 
	$(MKDOS) 
endif

clean:
ifeq ($(SYSTEM), Linux)
	rm -f licmd
else
	@echo Never mind the above errors. It means you are under DOS.
	rm -f licmd.exe
endif
