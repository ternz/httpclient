INCLUDE=..

EXE=util_test header_test client_test

FLAG=-g

all:$(EXE)

util_test: util_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a

header_test: header_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a

client_test: client_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a -lcurl

clean:
	rm -f $(EXE)