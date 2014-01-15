#include "cl_img_win.h"
#include <FL/Fl.H>
#include <unistd.h>

/*
 * Just a little test caller for img_win
 */
 
int main()
{
  int w=300;
  int h=250;
  int i;
  img_win *tmp = new img_win(20,20,w,h);
  tmp->show();
  for (i=0; i<50; i++)
    {
      Fl::wait(0.01);
      usleep(10000);
    }

  w *=2;
  h *=2;
  uchar img[w*h*3];
  for (i=0; i<h; i+=3)
    {
      for(int y=0; y<h; ++y)
        for(int x=0; x<w; ++x)
          {
            img[(x+y*w)*3]=(x/10+y/10)%2 * 255;
            img[(x+y*w)*3+1]=(y>i)*255;
            img[(x+y*w)*3+2]=(y>2*i)*255;
          }

      tmp->copy_img(img, w, h, 1);
    }

  delete(tmp);
  return 0;
}
