obj-m += handle_divided_by_zero.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

test: test.c
	gcc test.c -o test -O0 -g3

testsignlinux: testsignlinux.cpp
	g++ testsignlinux.cpp -o testsignlinux -O0 -g3 -fnon-call-exceptions
