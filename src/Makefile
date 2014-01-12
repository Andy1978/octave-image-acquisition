## depends on libv4l-dev libfltk1.3-dev

.PHONY:clean
.PHONY:all

TARGETS = __imaq_handler_open__.oct __imaq_handler_streamon__.oct __imaq_handler_streamoff__.oct\
          __imaq_handler_capture__.oct __imaq_handler_s_fmt__.oct __imaq_handler_g_fmt__.oct __img_win__.o __imaq_handler__.o

LIBS = -lv4l2 $(shell fltk-config  --ldflags)
MKOCTFLAGS = -Wall -v

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
