## depends on libv4l-dev libfltk1.3-dev

.PHONY:clean
.PHONY:all

TARGETS = _imaq_handler_.oct

LIBS = -lv4l2 $(shell fltk-config  --ldflags)
MKOCTFLAGS = -g -Wall -v

all: $(TARGETS)

%.oct: %.cc __imaq_handler__.o __img_win__.o
	mkoctfile $(MKOCTFLAGS) $^ $(LIBS)

%.o: %.cc %.h
	mkoctfile $(MKOCTFLAGS) -c $<

clean:
	rm -f $(TARGETS)
	rm -f *.ppm *.o *.oct octave-core
	rm -f octave-workspace
	rm -rf doxy
