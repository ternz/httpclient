INCLUDE=..

EXE=util_test header_test client_test curl_multi_test client_test2

FLAG=-g

all:$(EXE)

util_test: util_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a -lcurl

header_test: header_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a -lcurl

client_test: client_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a -lcurl

client_test2: client_test2.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) ../libhttpclient.a -lcurl

curl_multi_test: curl_multi_test.cpp
	g++ $(FLAG) -o $@ $< -I$(INCLUDE) -lcurl

clean:
	rm -f $(EXE)