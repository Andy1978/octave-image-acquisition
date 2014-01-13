## depends on libv4l-dev libfltk1.3-dev

.PHONY:clean
.PHONY:all

TARGETS = __imaq_handler_open__.oct __imaq_handler_querycap__.oct __imaq_handler_streamon__.oct __imaq_handler_streamoff__.oct\
          __imaq_handler_capture__.oct __imaq_handler_s_fmt__.oct __imaq_handler_g_fmt__.oct __img_win__.o __imaq_handler__.o\
          __imaq_handler_enuminput__.oct __imaq_handler_s_input__.oct __imaq_handler_g_input__.oct\
          __imaq_handler_enum_fmt__.oct __imaq_handler_enum_framesizes__.oct __imaq_handler_enum_frameintervals__.oct\
          __imaq_handler_queryctrl__.oct __imaq_handler_g_ctrl__.oct __imaq_handler_s_ctrl__.oct

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
