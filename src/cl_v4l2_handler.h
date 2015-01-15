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

#ifndef _V4L2_HANDLER_
#define _V4L2_HANDLER_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <libv4l2.h>

#include <octave/oct.h>
#include <octave/ov-struct.h>

#include <octave/dMatrix.h>
#include <iostream>
#include "cl_img_win.h"

using namespace std;

#define CLEAR(x) memset(&(x), 0, sizeof(x))
#define xioctl(n, r, p) xioctl_name(n, r, p, #r, __FILE__, __LINE__)

//! buffers for mmap
struct buffer
{
  void   *start;  //!< start of buffer
  size_t length;  //!< length in bytes, e.g. 3*640*480 (nColors*width*height)
};

/*!
 * v4l2 wrapper for octave-image-acquisition
 *
 * A big help was the Video Grabber example using libv4l by Mauro Carvalho Cheha
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/v4l2grab-example.html
 */
class v4l2_handler: public octave_base_value
{
public:

  v4l2_handler ();

  octave_base_value *clone (void) const // TODO: check if this is okay
  {
    octave_stdout << "v4l2_handler clone" << endl;
    return new v4l2_handler (*this);
  }

  octave_base_value *empty_clone (void) const // TODO: check if this is okay
  {
    octave_stdout << "v4l2_handler empty_clone" << endl;
    return new v4l2_handler ();
  }

  ~v4l2_handler (void);

  void open (string d);       //!< open a v4l2 device e.g. /dev/video0
  void print (std::ostream& os, bool pr_as_read_syntax) const;  //!< print itself on ostream
  octave_value querycap ();  //!< Query device capabilities

  octave_value enuminput (); //!< Enumerate video inputs
  int g_input ();            //!< Query the current video input
  void s_input (int index);  //!< Select video input

  octave_value enum_fmt (enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE); //!< Enumerate image formats
  Matrix enum_framesizes (string pixelformat);     //!< Enumerate frame sizes
  octave_scalar_map g_fmt ();                      //!< Get current format
  void s_fmt (string fmtstr, __u32 xres, __u32 yres); //!< Set format

  Matrix enum_frameintervals (string pixelformat, __u32 width, __u32 height);     //!< Enumerate frame intervals
  Matrix g_parm ();                    //!< Get streaming parameters (like frame interval)
  void s_parm (Matrix timeperframe);   //!< Set streaming parameters (like frame interval)

  octave_value queryctrl ();                  //!< Query controls
  int g_ctrl (int id);                        //!< Get control
  void s_ctrl (int id, int value);            //!< Set control

  octave_value_list capture (int nargout,
                             int preview=0);  //!< Retrieve captured image from buffer
  void capture_to_ppm (const char *fn);       //!< Retrieve captured image from buffer and save it as ppm

  void streamon (unsigned int n);             //!< start streaming with n buffers
  void streamoff ();                          //!< stop streaming

  void close ();                              //!< close v4l2 device

  bool preview_window_is_shown()
  {
    Fl::wait(0);
    return (preview_window)? preview_window->shown() : false;
  }

private:
  v4l2_handler (const v4l2_handler& m);

  int fd;
  string dev;
  unsigned int n_buffer;
  struct buffer *buffers;
  bool streaming;
  img_win *preview_window;

  // Properties
  bool is_constant (void) const
  {
    return true;
  }
  bool is_defined (void) const
  {
    return true;
  }

  void xioctl_name (int fh, unsigned long int request, void *arg, const char* name, const char* file, const int line);
  octave_scalar_map get_osm (struct v4l2_queryctrl queryctrl);
  void reqbufs (unsigned int n);  //!< Initiate Memory Mapping or User Pointer I/O
  void mmap ();
  void qbuf ();
  void munmap ();

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

v4l2_handler* get_v4l2_handler_from_ov (octave_value ov);

#endif
