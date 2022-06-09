include ../../tools/hooks/buildconf.makefile
TARGET = tcc
HOOKS_CFLAGS += -std=c++11 -I../openssl/include -I../../../include 

all: 
	g++ $(TARGET).cc $(HOOKS_CFLAGS) -o $(TARGET)

clean :
	rm -f *.o $(TARGET)




