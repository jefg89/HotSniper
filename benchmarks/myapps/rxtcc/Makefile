
TARGET = tcc
HOOKS_CFLAGS += -std=c++11 -I../openssl/include -I../../../include 

tx: 
	g++ $(TARGET).cc $(HOOKS_CFLAGS) -o $(TARGET)
rx:
	g++ -o rx_dft rx_dft.cc -I /usr/local/include  -L/usr/local/lib -lfftw3 -lpthread

l:
	g++ -o label label.cc

clean :
	rm -f *.o $(TARGET)




