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

#include <stdio.h>
#include <stdlib.h>
#include <octave/oct.h>
#include "config.h"

#ifdef HAVE_LIBV4L2_H
#include "cl_v4l2_handler.h"
#endif

#ifdef HAVE_MFAPI_H
#include "cl_mf_handler.h"
#endif

using namespace std;

// PKG_ADD: autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_enum_devices__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{l} =} __imaq_enum_devices__ ()\n\
List image capture devices.\n\
@end deftypefn")
{
#ifdef HAVE_LIBV4L2_H
  v4l2_handler imgh;
#endif
#ifdef HAVE_MFAPI_H
  mf_handler imgh;
#endif
  return octave_value (imgh.enum_devices ());
}

// PKG_ADD: autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_open__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{h} =} __imaq_handler_open__ (@var{type}, @var{device})\n\
Creates an instance of imaq_handler for a v4l2 or MF device and opens it.\n\
@seealso{getsnapshot}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage();
      return retval;
    }

  string type   = args(0).string_value ();
  string device = args(1).string_value ();

  imaq_handler *h = 0;

  if (type == "v4l2")
#ifdef HAVE_LIBV4L2_H
    h = new v4l2_handler ();
#else
    error ("octave-image-acquisition was built without v4l2 support");
#endif
  else if (type == "mf")
#ifdef HAVE_MFAPI_H
    h = new mf_handler ();
#else
    error ("octave-image-acquisition was built without media foundation support");
#endif
  else
    error ("unknown interface '%s'", type.c_str());

  //octave_stdout << "h = " << h << std::endl;
  h->open (device.c_str (), false);
  retval.append (octave_value (h));

  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_querycap__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_querycap__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_querycap__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{c} = } __imaq_handler_querycap__ (@var{h})\n\
Query device capabilities, driver name, card type etc. from imaq_handler @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->querycap ();
    }
  return retval;
}


// INPUTS

// PKG_ADD: autoload ("__imaq_handler_enum_inputs__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_enum_inputs__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_enum_inputs__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{inputs} = } __imaq_handler_enum_inputs__ (@var{h})\n\
Enumerate video inputs from @var{h}.\n\
Returns a struct with information for all available inputs.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->enum_inputs ();
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_get_input__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_get_input__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_get_input__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{N} =} __imaq_handler_get_input__ (@var{h})\n\
Query the current video input from @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();
  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = octave_value(imgh->get_input ());
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_set_input__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_set_input__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_set_input__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __imaq_handler_set_input__ (@var{h}, @var{n})\n\
Select video input @var{n} for @var{h}.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      if (! args(1).isnumeric())
        error("N has to be a integer selecting the desired video input, starting from  0.");
      else
        {
          int num = args(1).int_value ();
          imgh->set_input (num);
        }
    }
  return retval;
}

// FORMAT

// PKG_ADD: autoload ("__imaq_handler_enum_formats__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_enum_formats__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_enum_formats__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{formats} = } __imaq_handler_enum_formats__ (@var{h})\n\
Enumerate image formats from @var{h}.\n\
Returns a struct with information for all available formats.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->enum_formats ();
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_enum_framesizes__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_enum_framesizes__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_enum_framesizes__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{sizes} = } __imaq_handler_enum_framesizes__ (@var{h}, @var{format})\n\
Enumerate available frame sizes from imaq_handler @var{h} for given pixelformat (FOURCC).\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      string pixel_format = args(1).string_value ();
      retval = octave_value(imgh->enum_framesizes (pixel_format));
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_enum_frameintervals__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_enum_frameintervals__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_enum_frameintervals__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __imaq_handler_enum_frameintervals__ (@var{h}, @var{size}, @var{format})\n\
Enumerate available frame intervals from imaq_handler @var{h}.\n\
Return a Nx2 matrix with numerator, denominator.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      if (!args (1).is_matrix_type())
        print_usage();
      else
        {
          Matrix s = args(1).matrix_value ();
          unsigned int width = s(0);
          unsigned int height = s(1);
          string pixel_format = args(2).string_value ();
          retval = octave_value(imgh->enum_frameintervals (pixel_format, width, height));
        }
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_get_frameinterval__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_get_frameinterval__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_get_frameinterval__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __imaq_handler_get_frameinterval__ (@var{h})\n\
Return current frame interval as numerator, denominator.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = octave_value(imgh->get_frameinterval ());
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_set_frameinterval__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_set_frameinterval__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_set_frameinterval__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{T} = } __imaq_handler_set_frameinterval_ (@var{h}, @var{s})\n\
Set frame interval numerator and denominator.\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      imgh->set_frameinterval(args(1).matrix_value ());
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_g_fmt__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_g_fmt__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_g_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} @var{fmt} = __imaq_handler_g_fmt__ (@var{h})\n\
Get format pixelformat, size[width height].\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = octave_value(imgh->g_fmt ());
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_s_fmt__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_s_fmt__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_s_fmt__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __imaq_handler_s_fmt__ (@var{h}, @var{fmt}, @var{size})\n\
Set format @var{fmt}, @var{size} (V4L2_FIELD_INTERLACED).\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }
  if (!args (1).is_string() || !args (2).is_matrix_type())
    {
      print_usage();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      string fmt = args(1).string_value ();
      Matrix s = args(2).matrix_value ();
      unsigned int xres = s(0);
      unsigned int yres = s(1);

      imgh->s_fmt (fmt, xres, yres);
    }
  return retval;
}
// CONTROLS

// PKG_ADD: autoload ("__imaq_handler_queryctrl__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_queryctrl__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_queryctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{controls} = } __imaq_handler_queryctrl__ (@var{h})\n\
Query controls like brightness, contrast, saturation etc. from imaq_handler @var{h}.\n\
Use the field id for calls to __imaq_handler_s_ctrl__.\n\
@seealso{__imaq_handler_s_ctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      retval = imgh->queryctrl ();
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_g_ctrl__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_g_ctrl__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_g_ctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{value} =} __imaq_handler_g_ctrl__ (@var{h}, @var{id})\n\
Get value for control @var{id} from imaq_handler @var{h}.\n\
Use the field id from __imaq_handler_queryctrl__.\n\
@seealso{__imaq_handler_queryctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }
  if (!args (1).isnumeric())
    {
      error("ID has to be an integer value");
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      unsigned int id = args(1).int_value ();
      retval = octave_value(imgh->g_ctrl (id));
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_s_ctrl__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_s_ctrl__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_s_ctrl__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {} __imaq_handler_s_ctrl__ (@var{h}, @var{id}, @var{value})\n\
Set control @var{id} like brightness, contrast, saturation etc. in imaq_handler @var{h}.\n\
Use the field id from __imaq_handler_queryctrl__.\n\
@seealso{__imaq_handler_queryctrl__}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }
  if (!args (1).isnumeric() || !args (2).isnumeric())
    {
      error("ID and VALUE has to be integer values");
      return retval;
    }
  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      unsigned int id = args(1).int_value ();
      unsigned int value = args(2).int_value ();
      imgh->s_ctrl (id, value);
    }
  return retval;
}

// STREAMING

// PKG_ADD: autoload ("__imaq_handler_streamoff__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_streamoff__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_streamoff__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __imaq_handler_streamoff__ (@var{h})\n\
Stop streaming.\n\
@seealso{streamoff}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 1)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    imgh->streamoff ();
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_streamon__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_streamon__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_streamon__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} __imaq_handler_streamon__ (@var{h}, @var{n})\n\
Start streaming with @var{n} buffers. It is recommended to use at least 2 buffers.\n\
@seealso{streamoff, getsnapshot}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  if (nargin != 2)
    {
      print_usage ();
      return retval;
    }
  if (!args (1).isnumeric())
    {
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      unsigned int n_buffers = args(1).int_value ();
      imgh->streamon (n_buffers);
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_handler_capture__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_capture__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_capture__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{f} =} __imaq_handler_capture__ (@var{h}, @var{preview}, @var{rgb})\n\
Get a snapshot from @var{h}\n\
@end deftypefn")
{
  octave_value_list retval;
  int nargin = args.length ();

  printf ("nargin = %i\n", nargin);

  if (nargin != 3)
    {
      print_usage ();
      return retval;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      printf ("call\n");
      retval = imgh->capture (nargout, args(1).bool_value (), args(2).bool_value ());
    }
  return retval;
}

// PKG_ADD: autoload ("__imaq_preview_window_is_shown__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_preview_window_is_shown__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_preview_window_is_shown__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{l} =} __imaq_preview_window_is_shown__ (@var{h})\n\
Return preview_window->shown().\n\
@end deftypefn")
{
  octave_value ret;
  if (args.length () != 1)
    {
      print_usage ();
      return ret;
    }

  imaq_handler* imgh = get_imaq_handler_from_ov (args(0));
  if (imgh)
    {
      ret = imgh->preview_window_is_shown ();
    }
  return ret;
}

// PKG_ADD: autoload ("__imaq_handler_YCbCr_to_RGB__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_YCbCr_to_RGB__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_YCbCr_to_RGB__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{rgb} =} __imaq_handler_YCbCr_to_RGB__ (@var{yuv}, @var{ITU_standard})\n\
Convert YCbCr image (scalar struct with fields Y, Cb, Cr) into uint8 RGB image.\n\
Interpolation of chroma subsampling is done internally.\n\
ITU_standard might be 601, 709 or 2020.\n\
See also ycbcr2rgb from the image package.\n\
@end deftypefn")
{
  return ovl(imaq_handler::YCbCr_to_RGB (args(0), args(1).int_value()));
}

// PKG_ADD: autoload ("__imaq_handler_JPG_to_RGB__", which ("__imaq_handler__.oct"));
// PKG_DEL: autoload ("__imaq_handler_JPG_to_RGB__", which ("__imaq_handler__.oct"), "remove");
DEFUN_DLD(__imaq_handler_JPG_to_RGB__, args, nargout,
          "-*- texinfo -*-\n\
@deftypefn {Loadable Function} {@var{rgb} =} __imaq_handler_JPG_to_RGB__ (@var{jpg})\n\
Convert raw mjpg stream (as returned from the camera) into uint8 RGB image.\n\
@end deftypefn")
{
  return ovl(imaq_handler::JPG_to_RGB (args(0)));
}

/*
%!demo
%! disp("open /dev/video0 and show live images with 2 different formats")
%! vi = __imaq_handler_open__("v4l2", "/dev/video0");
%! s = __imaq_handler_enum_framesizes__(vi, "RGB24"); # get available frame sizes
%! __imaq_handler_s_fmt__(vi, "RGB24", s(1,:));       # use the default framesize
%! __imaq_handler_streamon__(vi, 2);                  # enable streaming with 2 buffers
%! l = 200;
%! for i=1:l
%!   __imaq_handler_capture__(vi, 1);                 # capture 200 frames and show preview
%! endfor
%! __imaq_handler_streamoff__(vi);                    # diable streaming
%! __imaq_handler_s_fmt__(vi, "RGB24", s(2,:));       # use smales available format
%! disp("The image size is now")
%! disp(__imaq_handler_g_fmt__(vi))
%! __imaq_handler_streamon__(vi, 2);                  # enable streaming with 2 buffers
%! for i=1:l
%!   __imaq_handler_capture__(vi, 1);
%! endfor
%! __imaq_handler_streamoff__(vi);
*/

/*
%!demo
%! x = __imaq_handler_open__(__test__device__{:});
%! disp("get controls")
%! ctrls = __imaq_handler_queryctrl__(x)
%! fieldnames(__imaq_handler_queryctrl__(x))
*/

/*
%!test
%! x = __imaq_handler_open__(__test__device__{:});
%! s = __imaq_handler_enum_framesizes__(x, "RGB24");
%! default_size = s(1,:);
%! __imaq_handler_s_fmt__(x, "RGB24", default_size);
%! t = __imaq_handler_enum_frameintervals__(x, default_size, "RGB24");
%! #__imaq_handler_enum_fmt__(x).description
%! __imaq_handler_streamon__(x, 2);
%! [img, seq, timestamp] = __imaq_handler_capture__(x);
%! assert(size(img), [default_size(2), default_size(1), 3]);
*/

/*  change controls
%!test
%! x = __imaq_handler_open__(__test__device__{:});
%! s = __imaq_handler_enum_framesizes__(x, "RGB24");
%! __imaq_handler_s_fmt__(x, "RGB24", s(end,:));
%! ctrls = __imaq_handler_queryctrl__(x);
%!   if (isfield(ctrls, "brightness"))
%!   min_brightness = ctrls.brightness.min;
%!   max_brightness = ctrls.brightness.max;
%!   __imaq_handler_s_ctrl__(x, ctrls.brightness.id, min_brightness);
%!   assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), min_brightness)
%!   __imaq_handler_s_ctrl__(x, ctrls.brightness.id, max_brightness);
%!   assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), max_brightness)
%!   v = round(max_brightness/2);
%!   __imaq_handler_s_ctrl__(x, ctrls.brightness.id, v);
%!   assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), v);
%! endif
*/

/*  check get timeperframe (1/fps).
 *  This may fail for example with some sn9c20x cameras
%!test
%! x = __imaq_handler_open__(__test__device__{:});
%! r = __imaq_handler_get_frameinterval__(x);
*/
