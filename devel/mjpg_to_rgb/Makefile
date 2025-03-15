.PHONY: clean

output.ppm: jpg_to_ppm C270_MJPG_frame.jpg
	./jpg_to_ppm

jpg_to_ppm: main.c
	gcc -Wall -Wextra main.c -ljpeg -o jpg_to_ppm

clean:
	rm -f jpg_to_ppm output.ppm
