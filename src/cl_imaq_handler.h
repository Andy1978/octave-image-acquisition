// Copyright (C) 2014-2025 Andreas Weber <andy.weber.aw@gmail.com>
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

#ifndef _IMAQ_HANDLER_
#define _IMAQ_HANDLER_

//~ #include <stdio.h>
//~ #include <stdlib.h>
//~ #include <string.h>
//~ #include <fcntl.h>
//~ #include <errno.h>

#include <iostream>

//#ifdef WIN32
//typedef unsigned int uint32;
//#endif

#include <octave/oct.h>
#include <octave/ov-struct.h>

//#include <octave/dMatrix.h>
#include <cstdint>

#include "cl_img_win.h"

// See https://savannah.gnu.org/bugs/?61994#comment30
// jwe: ... any .oct file that introduces a new type should derive from octave_base_dld_value instead of just octave_base_value ...
class imaq_handler: public octave_base_dld_value
{
public:

  imaq_handler ();

  //~ octave_base_value *clone (void) const // TODO: check if this is okay
  //~ {
  //~ octave_stdout << "v4l2_handler clone" << endl;
  //~ return new v4l2_handler (*this);
  //~ }

  //~ octave_base_value *empty_clone (void) const // TODO: check if this is okay
  //~ {
  //~ octave_stdout << "v4l2_handler empty_clone" << endl;
  //~ return new v4l2_handler ();
  //~ }

  ~imaq_handler (void);

  virtual octave_map enum_devices ()
  {
    return octave_map ();
  }

  virtual octave_value enum_inputs ()
  {
    return octave_value ();  //!< Enumerate video inputs
  }
  virtual int get_input ()
  {
    return 0;  //!< Query the current video input
  }
  virtual void set_input (int index)   {}                          //!< Select video input

  virtual octave_value enum_formats ()
  {
    return octave_value ();  //!< Enumerate image formats
  }

  virtual void print (std::ostream& os, bool pr_as_read_syntax);  //!< print itself on ostream
  virtual octave_scalar_map open (std::string d, bool quiet);

  virtual octave_value querycap ()
  {
    return octave_value ();  //!< Query device capabilities
  }

  virtual Matrix enum_framesizes (std::string pixelformat)
  {
    return Matrix ();  //!< Enumerate frame sizes
  }
  virtual octave_scalar_map g_fmt ()
  {
    return octave_scalar_map ();  //!< Get current format
  }
  virtual void s_fmt (std::string fmtstr, uint32_t xres, uint32_t yres) {}             //!< Set format

  virtual Matrix enum_frameintervals (std::string pixelformat, uint32_t width, uint32_t height)
  {
    return Matrix ();  //!< Enumerate frame intervals
  }
  virtual Matrix get_frameinterval ()
  {
    return Matrix ();
  }
  virtual void set_frameinterval (Matrix timeperframe) {}

  virtual octave_value queryctrl ()
  {
    return octave_value ();  //!< Query controls
  }
  virtual int g_ctrl (int id)
  {
    return 0; //!< Get control
  }
  virtual void s_ctrl (int id, octave_value val) {}     //!< Set control

  virtual void streamon (unsigned int n) {}             //!< start streaming with n buffers
  virtual void streamoff () {}                          //!< stop streaming

  virtual octave_value_list capture (int nargout,
                                     bool preview,
                                     bool raw_output)   //!< Retrieve captured image from buffer
  {
    return ovl();
  }

  //virtual void capture_to_ppm (const char *fn);         //!< Retrieve captured image from buffer and save it as ppm

  bool preview_window_is_shown()
  {
    Fl::wait(0);
    return (preview_window)? preview_window->shown() : false;
  }

  void set_ITU_standard (int standard){ ITU_standard = standard; };

  static uint8NDArray YCbCr_to_RGB (const octave_value& in, int ITU_standard = 601);
  static uint8NDArray JPG_to_RGB (const octave_value& in);

  virtual void close ();                              //!< close device

  // Properties

  bool is_defined (void) const { return true; }

  // Attention/FIXME: some examples show overloading rows + columns, some dims
  // It looks like Octave 9.3 only uses "dims"
  //int rows () const { return 1; }
  //int columns () const { return 1; }
  dim_vector dims (void) const { static dim_vector dv(1, 1); return dv; }

  bool is_constant (void) const { return true; }
  bool isobject (void) const { return true;}
  //bool print_as_scalar () const { return true; }

protected:

  img_win *preview_window;
  int ITU_standard;

  static octave_value get_RGB24      (void *start, size_t length, uint32_t width, uint32_t height);
  static octave_value get_raw_bayer1 (void *start, size_t length, uint32_t width, uint32_t height);
  static octave_value get_raw_bayer2 (void *start, size_t length, uint32_t width, uint32_t height);
  static octave_value get_YUYV       (void *start, size_t length, uint32_t width, uint32_t height);
  static octave_value get_YVU420     (void *start, size_t length, uint32_t width, uint32_t height, bool is_YUV = false);
  static octave_value get_NV12       (void *start, size_t length, uint32_t width, uint32_t height);
  static octave_value get_raw_bytes  (void *start, size_t length);

private:
  imaq_handler (const imaq_handler& m);
  static bool type_loaded;

  DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA
};

imaq_handler* get_imaq_handler_from_ov (octave_value ov);

#endif
