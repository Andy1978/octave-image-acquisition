// Copyright (C) 2024-2025 Andreas Weber <andy.weber.aw@gmail.com>
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
#include <mferror.h>     // for MF_E_NO_MORE_TYPES
#include <comdef.h>      // for IID_IAMCameraControl, IID_IAMVideoProcAmp

_COM_SMARTPTR_TYPEDEF(IAMCameraControl, IID_IAMCameraControl);
_COM_SMARTPTR_TYPEDEF(IAMVideoProcAmp, IID_IAMVideoProcAmp);

class mf_handler: public imaq_handler
{
public:

  mf_handler ();

  octave_base_value *clone (void) const // TODO: check if this is okay
  {
    octave_stdout << "mf_handler clone" << std::endl;
    return new mf_handler (*this);
  }

  octave_base_value *empty_clone (void) const // TODO: check if this is okay
  {
    octave_stdout << "mf_handler empty_clone" << std::endl;
    return new mf_handler ();
  }

  ~mf_handler (void);

  octave_map enum_devices ();

  octave_scalar_map open (std::string d, bool quiet);
  void print (std::ostream& os, bool pr_as_read_syntax);  //!< print itself on ostream

  // TODO: querycap not yet implemented for mf
  octave_value querycap (); //!< Query device capabilities

  // TODO: enum_inputs, get_input, set_input not yet implemented for media foundation
  octave_value enum_inputs ();       //!< Enumerate video inputs
  int get_input ();                  //!< Query the current video input
  void set_input (int index);        //!< Select video input

  // loops reader->GetNativeMediaType
  // returns all possible combinations of pixelformat, frame size and frame interval
  octave_map loop_native_media_types ();

  octave_value enum_formats ();      //!< Enumerate image formats

  void s_fmt (std::string fmtstr, uint32_t xres, uint32_t yres); //!< Set format
  octave_scalar_map g_fmt (IMFMediaType *pType);            //!< Get current format
  octave_scalar_map g_fmt ();

  Matrix enum_framesizes (std::string pixelformat);                                       //!< Enumerate frame sizes

  Matrix enum_frameintervals (std::string pixelformat, uint32_t width, uint32_t height);  //!< Enumerate frame intervals
  Matrix get_frameinterval ();
  void set_frameinterval (Matrix timeperframe);

  octave_value queryctrl ();                  //!< Query controls
  int g_ctrl (int id);                        //!< Get control
  void s_ctrl (int id, octave_value val);     //!< Set control

  octave_value_list capture (int nargout, bool preview, bool raw_output);  //!< Retrieve captured image from buffer

  /*
    void capture_to_ppm (const char *fn);       //!< Retrieve captured image from buffer and save it as ppm
    void streamon (unsigned int n);             //!< start streaming with n buffers
    void streamoff ();                          //!< stop streaming
  */
  void close ();                              //!< close device

  //int rows () const { return 5; }
  //int columns () const { return 5; }
  //bool is_constant (void) const { return true; }
  //bool is_defined (void) const { return true; }
  //bool is_object (void) const { return true;}
  // 4.4+
  //bool isobject (void) const { return true;}
  //bool print_as_scalar () const { return true; }

private:
  mf_handler (const mf_handler& m);

  IMFMediaSource* device;
  IMFSourceReader* reader;

  //  bool streaming;

};

#endif
