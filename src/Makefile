## depends on libv4l-dev libfltk1.3-dev

.PHONY:clean
.PHONY:all

TARGETS = __v4l2_handler__.oct cl_v4l2_handler.o cl_img_win.o

LIBS = -lv4l2 $(shell fltk-config  --ldflags)
MKOCTFLAGS = -g -Wall -v

all: $(TARGETS)

%.oct: %.cc cl_v4l2_handler.o cl_img_win.o
	mkoctfile $(MKOCTFLAGS) $^ $(LIBS)

%.o: %.cc %.h
	mkoctfile $(MKOCTFLAGS) -c $<

clean:
	rm -f $(TARGETS)
	rm -f *.ppm *.o *.oct octave-core octave-workspace
	rm -rf doxy
