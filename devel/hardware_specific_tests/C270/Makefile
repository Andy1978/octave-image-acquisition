.PHONY:all
.PHONY:clean

all:
	octave -q check.m
	octave -q try_different_pixelformats.m

clean:
	rm -f *.png
	rm -rf out
	rm -f octave-core octave-workspace
