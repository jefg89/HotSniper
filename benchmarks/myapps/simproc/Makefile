include ../../tools/hooks/buildconf.makefile
TARGET = simproc
HOOKS_CFLAGS += -std=c++11 -I../openssl/include -I../../../include

all: obj
	g++ -o $(TARGET) $(TARGET).o  
obj:
	g++ -c $(TARGET).cc $(HOOKS_CFLAGS)
clean :
	rm -f *.o $(TARGET)



