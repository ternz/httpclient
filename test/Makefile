INCLUDE=..

EXE=util_test header_test

FLAG=-g

all:$(EXE)

util_test: util_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../util.o

header_test: header_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a

clean:
	rm -f $(EXE)