CFLAGS = -g -Wall  -I.
CXXFLAGS = -g -Wall  -I.


ifeq ($(WITH_COVERAGE),Y)
CFLAGS += -g -pg -fprofile-arcs -ftest-coverage -Wno-write-strings -DTESTING=1
endif

all: test

command.h: 
	echo > $@


asm/io.h:
	mkdir -p asm
	echo > $@

asm/arch-mx6/gpio.h:
	mkdir -p asm/arch-mx6
	echo > $@

asm/arch/mx6.h:
	mkdir -p asm/arch
	echo > $@

asm/arch/imx-regs.h:
	mkdir -p asm/arch
	echo > $@

asm/arch/mx6dl_pins.h:
	mkdir -p asm/arch
	echo > $@

asm/arch/gpio.h:
	mkdir -p asm/arch
	echo > $@

asm/gpio.h:
	mkdir -p asm
	echo > $@

asm/arch/iomux.h:
	mkdir -p asm/arch
	echo > $@

pplans_dbgled.h: ../include/pplans_dbgled.h 
	cp $^ $@


clean:
	-rm -f coverage.xml
	-rm -f test
	-rm -f *.gcda
	-rm -f *.gcno
	-rm -f *.o
	-rm -f cmd_ledset.c


cmd_ledset.o: cmd_ledset.c common.h command.h asm/arch-mx6/gpio.h asm/io.h asm/arch/mx6.h asm/arch/imx-regs.h asm/arch/mx6dl_pins.h asm/arch/gpio.h asm/gpio.h asm/arch/iomux.h asm/arch/gpio.h

pplans_dbgled.o: pplans_dbgled.c pplans_dbgled.h

cmd_ledset.c: ../common/cmd_ledset.c
	cp $^ $@
    
pplans_dbgled.c: ../drivers/misc/pplans_dbgled.c
	cp $^ $@


test: ledset_test.o cmd_ledset.o  pplans_dbgled.o lib.o
	$(CXX) -o $@ $^  $(LDLIBS)  -lCppUTest --coverage

