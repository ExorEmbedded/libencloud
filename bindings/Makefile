CC ?= gcc
LD ?= ld
CFLAGS += -g
LDFLAGS += -L../src -lece

# use distutils by default
WITH_DISTUTILS = 1

# debugging
#WRAP = valgrind --leak-check=full --trace-children=yes

ifdef WITH_DISTUTILS
all:
	python setup.py build
	sudo python setup.py install
else
all: ece.py _ece.so
endif

clean:
	rm -rf build ece_wrap.c ece.py *.pyc *.o *.so

ece.py: ece.i
	swig -python $<

_ece.so:
	$(CC) $(CFLAGS) -c ece_wrap.c -I/usr/include/python2.7 -I../include
	$(CC) -shared -o _ece.so ece_wrap.o $(LDFLAGS)

check: all
	$(WRAP) python test.py
