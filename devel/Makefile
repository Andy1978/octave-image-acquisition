## depends on libv4l-dev libfltk1.3-dev

.PHONY:clean
.PHONY:all

TARGETS = test_v4l2_handler test_img_win

FL_LDFLAGS = $(shell fltk-config  --ldflags)
LIBS = -lv4l2
CXXFLAGS = -Wall -Wextra -I../src
CC = g++

all: $(TARGETS)

cl_v4l2_handler.o: ../src/cl_v4l2_handler.cc ../src/cl_v4l2_handler.h
	mkoctfile $(CXXFLAGS) -v -c $< -o $@

test_v4l2_handler: test_v4l2_handler.cc cl_v4l2_handler.o cl_img_win.o
	mkoctfile $(CXXFLAGS) -v --link-stand-alone -Wl,-rpath=/usr/local/lib/octave/3.8.0/ $^ -o $@ $(LIBS) $(FL_LDFLAGS)

cl_img_win.o: ../src/cl_img_win.cc ../src/cl_img_win.h
	$(CC) $(CXXFLAGS) -c $< -o $@ $(FL_LDFLAGS)

test_img_win: test_img_win.cc cl_img_win.o
	$(CC) $(CXXFLAGS) $^ -o $@ $(FL_LDFLAGS)

.PHONY:clean

../src/__v4l2_handler__.oct:
	$(MAKE) -C ../src

check: ../src/__v4l2_handler__.oct
	octave -q run_tests.m

cpp_check: test_img_win test_v4l2_handler 
	./test_img_win
	./test_v4l2_handler

clean:
	rm -f $(TARGETS) fntest.log
	rm -f *.ppm *.o *.oct octave-core octave-workspace
