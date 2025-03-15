#include <stdio.h>
#include <stdlib.h>
#include <jpeglib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main ()
{
  int r;
  
  const char fn[] = "C270_MJPG_frame.jpg";
  struct stat file_info;
	unsigned long jpg_size;
  
  r = stat(fn, &file_info);
  if (r)
  {
    fprintf (stderr, "stat file '%s' failed\n", fn);
    return -1;
  }

  jpg_size = file_info.st_size;
  printf ("jpg_size = %ld\n", jpg_size);
	unsigned char *jpg_buffer = (unsigned char*) malloc(jpg_size + 100);    
  
  FILE* fd = fopen(fn, "rb");
	r = fread(jpg_buffer, 1, jpg_size, fd);
  printf ("r = %i\n", r);
  printf ("ferror = %i\n", ferror(fd));
  printf ("feof = %i\n", feof(fd));
	fclose(fd);
  
  
 	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
  
  cinfo.err = jpeg_std_error(&jerr);	
	jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, jpg_buffer, jpg_size);
  
  r = jpeg_read_header(&cinfo, TRUE);
  if (r != JPEG_HEADER_OK)
    fprintf (stderr, "header error r = %i\n", r);
  //fprintf (stderr, "JPEG_HEADER_TABLES_ONLY = %i\n", JPEG_HEADER_TABLES_ONLY);
  
  jpeg_start_decompress(&cinfo);
  
  int width = cinfo.output_width;
	int height = cinfo.output_height;
	int pixel_size = cinfo.output_components;
  
  printf ("width = %d, height = %d, pixel_size = %d\n", width, height, pixel_size);
  
  
  unsigned char *buffer = (unsigned char*) malloc(width*height*pixel_size);
  
  
  while (cinfo.output_scanline < cinfo.output_height)
  {
		unsigned char *buffer_array[1];
		buffer_array[0] = buffer + (cinfo.output_scanline) * width * pixel_size;
		jpeg_read_scanlines(&cinfo, buffer_array, 1);

	}
	printf ("Proc: Done reading scanlines\n");
  
  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);
  free(jpg_buffer);
  
  int fdx = open("output.ppm", O_CREAT | O_WRONLY, 0666);
	char buf[1024];
	r = sprintf(buf, "P6 %d %d 255\n", width, height);
	write(fdx, buf, r); // Write the PPM image header before data
	write(fdx, buffer, width*height*pixel_size); // Write out all RGB pixel data

	close(fdx);
	free(buffer);
  
  return 0;
}
