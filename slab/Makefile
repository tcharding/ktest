obj-m += tmodule.o

#DIR=/usr/lib/modules/$(shell uname -r)/build
DIR=/home/tobin/build/kdev/linux

all:
	make -C ${DIR} M=$(PWD) modules

clean:
	make -C ${DIR} M=$(PWD) clean
