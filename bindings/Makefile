CC ?= gcc
LD ?= ld
CFLAGS += -g
LDFLAGS += -L../src -lece

# debugging
#WRAP = valgrind --leak-check=full --trace-children=yes

all: ece.py _ece.so

clean:
	rm -f ece_wrap.c ece.py *.pyc *.o *.so

ece.py: ece.i
	swig -python $<

_ece.so:
	$(CC) $(CFLAGS) -c ece_wrap.c -I/usr/include/python2.7 -I../include
	$(CC) -shared -o _ece.so ece_wrap.o $(LDFLAGS)

check:
	$(WRAP) python test.py
