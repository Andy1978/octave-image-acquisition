## depends on libv4l-dev libfltk1.3-dev

.PHONY:clean
.PHONY:all

TARGETS = test_imaq_handler test_img_win

FL_LDFLAGS = $(shell fltk-config  --ldflags)
LIBS = -lv4l2
CXXFLAGS = -Wall -Wextra -I../src
CC = g++

all: $(TARGETS)

imaq_handler.o: ../src/__imaq_handler__.cc ../src/__imaq_handler__.h
	mkoctfile $(CXXFLAGS) -v -c $< -o $@

test_imaq_handler: test_imaq_handler.cc imaq_handler.o img_win.o
	mkoctfile $(CXXFLAGS) -v --link-stand-alone -Wl,-rpath=/usr/local/lib/octave/3.8.0/ $^ -o $@ $(LIBS) $(FL_LDFLAGS)

img_win.o: ../src/__img_win__.cc ../src/__img_win__.h
	$(CC) $(CXXFLAGS) -c $< -o $@ $(FL_LDFLAGS)

test_img_win: test_img_win.cc img_win.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(FL_LDFLAGS)

clean:
	rm -f $(TARGETS)
	rm -f *.ppm *.o *.oct octave-core octave-workspace
