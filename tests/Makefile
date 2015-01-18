CFLAGS = -g -Wall  -I. -DCONFIG_MXC_EPDC
CXXFLAGS = -g -Wall  -I. -DCONFIG_MXC_EPDC


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

linux/list.h:
	mkdir -p linux
	echo > $@

linux/err.h:
	mkdir -p linux
	echo > $@

linux/types.h:
	mkdir -p linux
	echo > $@

pplans_dbgled.h: ../include/pplans_dbgled.h 
	cp $^ $@

pplans_pmic.h: ../include/pplans_pmic.h 
	cp $^ $@

pplans_dbgled.c: ../drivers/misc/pplans_dbgled.c
	cp $^ $@

clean:
	-rm -f coverage.xml
	-rm -f test
	-rm -f *.gcda
	-rm -f *.gcno
	-rm -f *.o
	-rm -f cmd_ledset.c
	-rm -f pplans_pmic.h
	-rm -f pplans_pmic.c
	-rm -f pplans_dbgled.c
	-rm -f pplans_dbgled.h
	-rm -f mxc_epdc_fb.c
	-rm -f mxc_epdc_fb.h
	-rm -f lcd.h command.h



cmd_ledset.o: cmd_ledset.c common.h command.h asm/arch-mx6/gpio.h asm/io.h asm/arch/mx6.h asm/arch/imx-regs.h asm/arch/mx6dl_pins.h asm/arch/gpio.h asm/gpio.h asm/arch/iomux.h asm/arch/gpio.h pplans_dbgled.h

ledset_test.o: ledset_test.cpp mockio.h

mxc_epdc_fb_test.o: mxc_epdc_fb_test.cpp mockio.h mxc_epdc_fb.h

mockio.h:  common.h lcd.h

pplans_pmic.o: pplans_pmic.c pplans_pmic.h

mxc_epdc_fb.o: mxc_epdc_fb.h lcd.h linux/list.h linux/err.h linux/types.h mxc_epdc_fb.c
	$(CC) $(CFLAGS) -c -Dstatic= -Dinline= mxc_epdc_fb.c


cmd_ledset.c: ../common/cmd_ledset.c
	cp $^ $@
    
pplans_pmic.c: ../drivers/misc/pplans_pmic.c
	cp $^ $@

mxc_epdc_fb.c: ../drivers/video/mxc_epdc_fb.c
	cp $^ $@

mxc_epdc_fb.h: ../drivers/video/mxc_epdc_fb.h
	cp $^ $@

lcd.h: ../include/lcd.h
	cp $^ $@


test: pplans_pmic.o mxc_epdc_fb_test.o ledset_test.o pmic_setup_test.o cmd_ledset.o mxc_epdc_fb.o  \
      pplans_dbgled.o mockio.o lib.o
	$(CXX)  -o $@ $^  $(LDLIBS)  -lCppUTest -lCppUTestExt --coverage

