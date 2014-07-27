CFLAGS = -g -Wall  -I.
CXXFLAGS = -g -Wall  -I.


ifeq ($(WITH_COVERAGE),Y)
CFLAGS += -g -pg -fprofile-arcs -ftest-coverage -Wno-write-strings -DTESTING=1
endif

clean:
	-rm coverage.xml
	-rm test
	-rm *.gcda
	-rm *.gcno
	-rm *.o


test: ledset_test.o ../common/cmd_ledset.o lib.o
	$(CXX) -o $@ $^  $(LDLIBS)  -lCppUTest --coverage

