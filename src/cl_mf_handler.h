// Copyright (C) 2024 Andreas Weber <andy.weber.aw@gmail.com>
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

#ifndef _MF_HANDLER_
#define _MF_HANDLER_

#include <octave/oct.h>
#include <octave/ov-struct.h>
#include <octave/dMatrix.h>
#include <iostream>

#include "cl_imaq_handler.h"
#include "cl_img_win.h"

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h> // für MF_E_NO_MORE_TYPES

using namespace std;

class mf_handler: public imaq_handler
{
public:

  mf_handler ();

  octave_base_value *clone (void) const // TODO: check if this is okay
  {
    octave_stdout << "mf_handler clone" << endl;
    return new mf_handler (*this);
  }

  octave_base_value *empty_clone (void) const // TODO: check if this is okay
  {
    octave_stdout << "mf_handler empty_clone" << endl;
    return new mf_handler ();
  }

  ~mf_handler (void);

  octave_map enum_devices ();

  octave_scalar_map open (string d, bool quiet);
  void print (std::ostream& os, bool pr_as_read_syntax);  //!< print itself on ostream

  // TODO: enum_inputs, get_input, set_input not yet implemented for media foundation

  octave_value enum_formats ();      //!< Enumerate image formats
	
	void s_fmt (string fmtstr, __u32 xres, __u32 yres);
	octave_scalar_map g_fmt ();

  octave_value_list capture (int nargout, int preview);  //!< Retrieve captured image from buffer


/*
  octave_value querycap ();        //!< Query device capabilities

  octave_value enuminput ();       //!< Enumerate video inputs
  int g_input ();                  //!< Query the current video input
  void s_input (int index);        //!< Select video input

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
*/
  void close ();                              //!< close device

/*
  bool preview_window_is_shown()
  {
    Fl::wait(0);
    return (preview_window)? preview_window->shown() : false;
  }

  bool is_video_capture () {return _is_video_capture;}
  bool is_meta_capture () {return _is_meta_capture;}
*/

private:
  mf_handler (const mf_handler& m);
	
	IMFSourceReader* reader;
	
  //static bool type_loaded;
/*
  int fd;
  string dev;
  unsigned int n_buffer;
  struct buffer *buffers;
  bool streaming;
  img_win *preview_window;
  bool _is_video_capture;
  bool _is_meta_capture;
*/

  // Properties
  bool is_constant (void) const
  {
    return true;
  }
  bool is_defined (void) const
  {
    return true;
  }
/*
  void xioctl_name (int fh, unsigned long int request, void *arg, const char* name, const char* file, const int line);
  octave_scalar_map get_osm (struct v4l2_queryctrl queryctrl);
  void reqbufs (unsigned int n);  //!< Initiate Memory Mapping or User Pointer I/O
  void mmap ();
  void qbuf ();
  void munmap ();
  octave_scalar_map expand_cap (unsigned int cap);
*/
  //DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

//v4l2_handler* get_v4l2_handler_from_ov (octave_value ov);

#endif
