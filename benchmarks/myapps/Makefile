
.PHONY: all

include ../tools/hooks/buildconf.makefile

SUBDIRS = $(shell ls -d */)


all:
	make -C tools
	make -C pi
	cd openssl && ./config && make -j8
	make -C aes
	make -C noaes
	make -C aggro
	make -C attestation
	make -C tcc
#make -C rxtcc


# all:
# 	for dir in $(SUBDIRS) ; do \
# 		make -C  $$dir ; \
# 	done

clean:
	for dir in $(SUBDIRS) ; do \
		make -C  $$dir clean ; \
	done

