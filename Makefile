FLAG=-g
INCLUDE=.
LIB_DIR=

OBJ=util.o header.o request.o

all: libhttpclient.a

libhttpclient.a: $(OBJ)
	ar r $@ $(OBJ)

#util.o: util.cpp util.h
#	g++ $(FLAG) -c $< -o $@ 

%.o: %.cpp
	g++ $(FLAG) -c $< -o $@ 

clean:
	rm -f $(OBJ) libhttpclient.a

