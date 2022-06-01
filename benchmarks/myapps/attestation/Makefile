include ../../tools/hooks/buildconf.makefile
TARGET = attestation
HOOKS_CFLAGS += -std=c++11 -I../openssl/include -I../../../include -I../BLAKE2/sse
NO_OPENMP?=1
NO_OPENMP_0=-fopenmp
NO_OPENMP_1=
CC?=gcc
#CFLAGS?=-O3 -march=native
#CFLAGS+=-std=c89 -Wall -Wextra -pedantic -Wno-long-long 
HOOKS_CFLAGS+=$(NO_OPENMP_$(NO_OPENMP))
LIBS=
#FILES=b2sum.c ../ref/blake2b-ref.c ../ref/blake2s-ref.c ../ref/blake2bp-ref.c ../ref/blake2sp-ref.c
FILES=../BLAKE2/sse/blake2b.c ../BLAKE2/sse/blake2s.c ../BLAKE2/sse/blake2bp.c ../BLAKE2/sse/blake2sp.c

all: 
	g++ $(TARGET).cc $(FILES) $(HOOKS_CFLAGS) -o $(TARGET)  ../openssl/libcrypto.a

clean :
	rm -f *.o $(TARGET)




