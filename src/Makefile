MKOCTFILE ?= mkoctfile -Wall

all: 

%.oct: %.cc
	$(MKOCTFILE) $<

clean:
	rm -f *.o octave-core core *.oct *~
