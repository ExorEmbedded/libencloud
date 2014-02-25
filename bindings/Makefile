CC ?= gcc
LD ?= ld
CFLAGS += -g
LDFLAGS += -L../src -lencloud

# use distutils by default
WITH_DISTUTILS = 1

# debugging
#WRAP = valgrind --leak-check=full --trace-children=yes

ifdef WITH_DISTUTILS
all:
	python setup.py build
else
all: encloud.py _encloud.so
endif

install:
	sudo python setup.py install

clean:
	rm -rf build encloud_wrap.c encloud.py *.pyc *.o *.so

encloud.py: encloud.i
	swig -python $<

_encloud.so:
	$(CC) $(CFLAGS) -c encloud_wrap.c -I/usr/include/python2.7 -I../include
	$(CC) -shared -o _encloud.so encloud_wrap.o $(LDFLAGS)

check: install
	$(WRAP) python test.py
