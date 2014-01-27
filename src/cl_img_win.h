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
  fast and simple image viewer based on FLTK.
  Currently RGB24 (8bit red, green, blue) and Grayvalues with 8bit are supported.
*/

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/fl_draw.H>
#include <iostream>
#include <stdio.h>
#include <string.h>   //for memcpy

using namespace std;

#ifndef _CIMAGEWINDOW_H_
#define _CIMAGEWINDOW_H_

//! width and height of border around actual image
#define BORDER_PX 1

class img_win : public Fl_Double_Window
{
private:
  uchar* pixel;         //!< internal memory for image
  bool RGB;

  int img_w ()
  {
    return w () - 2 * BORDER_PX;
  }
  int img_h ()
  {
    return h () - 2 * BORDER_PX;
  }
  int pixel_len()
  {
    return img_w () * img_h () * ((RGB)? 3: 1);
  }

public:
  img_win (int x, int y, int w, int h);
  ~img_win ();
  void draw ();
  void copy_img (const unsigned char* p, unsigned int w, unsigned int h, bool rgb);
  void custom_label (const char *device, unsigned int seq, double fps);
};

#endif
