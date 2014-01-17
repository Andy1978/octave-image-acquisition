// Copyright (C) 2014 Andreas Weber <andy.weber.aw@gmail.com>
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 3 of the License, or (at your option) any later
// version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
// details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, see <http://www.gnu.org/licenses/>.

/*!
  10.01.2014 Andreas Weber
  /file img_win.cpp
*/
#include "cl_img_win.h"
#include <stdlib.h>
#include <FL/Fl.H>

img_win::img_win (int x, int y, int w, int h)
  :Fl_Double_Window (x, y, w, h, "img_win"), pixel(NULL), RGB(0)
{
  Fl::visual(FL_RGB);
  //cout << "img_win::img_win C'Tor" << endl;
  // create test image
  uchar tmp[w*h];
  for(int x=0; x<w; ++x)
    for(int y=0; y<h; ++y)
      tmp[x+y*w]=(x/10+y/10)%2 * 255;
  copy_img(tmp, w, h, RGB);
}

img_win::~img_win ()
{
  //cout << "img_win::~img_win D'Tor" << endl;
  hide();
  Fl::wait(0);
  free(pixel);
}

void
img_win::draw ()
{
  Fl_Double_Window::draw ();
  if (pixel)
    fl_draw_image (pixel, BORDER_PX, BORDER_PX, img_w(), img_h(), (RGB)? 3: 1);
}

void
img_win::copy_img (const unsigned char* p, unsigned int w, unsigned int h, bool rgb)
{
  int len = w * h * ((rgb)? 3:1);
  if (len != pixel_len())
    {
      // resize buffers
      //cout << "img_win::copy_img resize buffers" << endl;
      pixel = (uchar*)realloc (pixel, len);
      if (!pixel)
        {
          cerr << "ERROR: could not allocate memory for internal pixel structure" << endl;
          exit (EXIT_FAILURE);
        }
      Fl_Double_Window::size (w + 2 * BORDER_PX, h + 2 * BORDER_PX);
      RGB = rgb;
      custom_label ("dummy", 0, 0);
    }

  memcpy (pixel, p, len);
  redraw ();
  if (shown ())
    flush ();
}

void
img_win::custom_label (const char *device, unsigned int seq, double fps)
{
#define BUF_LEN 80
  static char buf[BUF_LEN];
  snprintf (buf, BUF_LEN, "%d x %d %s seq=%06d fps=%5.2f %s", img_w(), img_h(), (RGB)? "RGB":"gray", seq, fps, device);
  label (buf);
}
