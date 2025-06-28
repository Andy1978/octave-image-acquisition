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

#include <cassert>
#include "cl_imaq_handler.h"

#include <jpeglib.h> // for JPG_to_RGB

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(imaq_handler, "imaq_handler", "imaq_handler");

bool imaq_handler::type_loaded = false;

imaq_handler::imaq_handler ()
  : octave_base_dld_value(), preview_window(0), ITU_standard (601)
{
  //octave_stdout << "imaq_handler C'Tor, type_loaded = " << type_loaded << endl;
  if (!type_loaded)
    {
      type_loaded = true;
      imaq_handler::register_type();
      //octave_stdout << "imaq_handler C'Tor, type_id() = " << type_id() << std::endl;
    }
}

//~ v4l2_handler::v4l2_handler (const v4l2_handler& m)
//~ : octave_base_dld_value()
//~ {
//~ octave_stdout << "v4l2_handler: the copy constructor shouldn't be called" << std::endl;
//~ }

imaq_handler::~imaq_handler ()
{
  //octave_stdout << "imaq_handler D'Tor " << std::endl;
  // delete preview_window if active
  if (preview_window)
    {
      delete preview_window;
      preview_window = 0;
    }
}

void
imaq_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class imaq_handler" << std::endl;
  //os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << std::endl;
}


octave_scalar_map
imaq_handler::open (std::string d, bool quiet)
{
  octave_scalar_map ret;
  octave_stdout << "imaq_handler::open (d = " << d << " called" << std::endl;
  return ret;
}

void
imaq_handler::close ()
{

}

// RGB3 aka RGB24
// return [height x width x 3] uint8 matrix
octave_value imaq_handler::get_RGB24 (void *start, size_t length, uint32_t width, uint32_t height)
{
  dim_vector dv (3, width, height);
  uint8NDArray img (dv);
  assert(img.numel() == int(length));

  unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
  memcpy(p, start, length);

  Array<octave_idx_type> perm (dim_vector (3, 1));
  perm(0) = 2;
  perm(1) = 1;
  perm(2) = 0;
  return octave_value(img.permute (perm));
}

// RAW Bayer, 1 byte per pixel
// return [height * width] uint8 matrix
octave_value imaq_handler::get_raw_bayer1 (void *start, size_t length, uint32_t width, uint32_t height)
{
  dim_vector dv (width, height);
  uint8NDArray img (dv);
  assert(img.numel() == int(length));
  unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
  memcpy(p, start, length);
  return octave_value(img.transpose ());
}

// RAW Bayer, 2 bytes per pixel
// return [height * width] uint16 matrix
octave_value imaq_handler::get_raw_bayer2 (void *start, size_t length, uint32_t width, uint32_t height)
{
  dim_vector dv (width, height);
  uint16NDArray img (dv);
  assert(img.numel()*2 == int(length));
  unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
  memcpy(p, start, length);
  return octave_value(img.transpose ());
}

// YUYV aka YUV 4:2:2
// https://www.kernel.org/doc/html/v4.8/media/uapi/v4l/pixfmt-yuyv.html
// V4L2_PIX_FMT_YUYV is known in the Windows environment as YUY2
// return struct with fields Y, Cb, Cr
octave_value imaq_handler::get_YUYV (void *start, size_t length, uint32_t width, uint32_t height)
{
  dim_vector dvy (width, height);
  dim_vector dvc (width/2, height);
  uint8NDArray y (dvy);
  uint8NDArray cb (dvc);
  uint8NDArray cr (dvc);
  assert ((y.numel() + cb.numel() + cr.numel()) == int(length));
  unsigned int i;
  unsigned char *s = reinterpret_cast<unsigned char*>(start);
  for (i=0; i < (width * height); ++i)
    y(i) = s[2 * i];
  for (i=0; i < (width * height / 2); ++i)
    {
      cb(i) = s[4 * i + 1];
      cr(i) = s[4 * i + 3];
    }

  octave_scalar_map img;
  img.assign ("Y", y.transpose ());
  img.assign ("Cb", cb.transpose ());
  img.assign ("Cr", cr.transpose ());
  return octave_value(img);
}

// YVU420 aka YV12
// see also is_YUV
// https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/pixfmt-yuv420.html
octave_value imaq_handler::get_YVU420 (void *start, size_t length, uint32_t width, uint32_t height, bool is_YUV)
{
  dim_vector dvy (width, height);
  dim_vector dvc (width/2, height/2);
  uint8NDArray y (dvy);
  uint8NDArray c1 (dvc);
  uint8NDArray c2 (dvc);
  assert ((y.numel() + c1.numel() + c2.numel()) == int(length));

  // Y
  unsigned char *p = reinterpret_cast<unsigned char*>(y.fortran_vec());
  memcpy(p, start, y.numel ());

  // C1
  p = reinterpret_cast<unsigned char*>(c1.fortran_vec());
  memcpy(p, (unsigned char*)start + y.numel (), c1.numel ());

  // C2
  p = reinterpret_cast<unsigned char*>(c2.fortran_vec());
  memcpy(p, (unsigned char*)start + y.numel () + c1.numel (), c2.numel ());

  octave_scalar_map img;
  img.assign ("Y", y.transpose ());
  if (is_YUV)
    // V4L2_PIX_FMT_YUV420
    {
      img.assign ("Cb", c1.transpose ());
      img.assign ("Cr", c2.transpose ());
    }
  else
    // V4L2_PIX_FMT_YVU420
    {
      img.assign ("Cb", c2.transpose ());
      img.assign ("Cr", c1.transpose ());
    }

  return octave_value(img);
}

// NV12
// https://www.kernel.org/doc/html/v4.9/media/uapi/v4l/pixfmt-nv12.html
octave_value imaq_handler::get_NV12 (void *start, size_t length, uint32_t width, uint32_t height)
{
  dim_vector dvy (width, height);
  dim_vector dvc (width/2, height/2);
  uint8NDArray y (dvy);
  uint8NDArray cb (dvc);
  uint8NDArray cr (dvc);
  assert ((y.numel() + cb.numel() + cr.numel()) == int(length));

  // Y
  unsigned char *p = reinterpret_cast<unsigned char*>(y.fortran_vec());
  memcpy(p, start, y.numel ());

  unsigned char *s = reinterpret_cast<unsigned char*>(start) + y.numel ();
  for (unsigned int i = 0; i < (width * height / 4); ++i)
    {
      cb(i) = s[2 * i];
      cr(i) = s[2 * i + 1];
    }

  octave_scalar_map img;
  img.assign ("Y", y.transpose ());
  img.assign ("Cb", cb.transpose ());
  img.assign ("Cr", cr.transpose ());
  return octave_value(img);
}

// Just return the bytes as vector
octave_value imaq_handler::get_raw_bytes (void *start, size_t length)
{
  dim_vector dv (length, 1);
  uint8NDArray img (dv);
  unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
  memcpy(p, start, length);
  return octave_value(img);
}

// YCbCr_to_RGB to RGB24
// returns [HxWx3] uint8 matrix with RGB values in the range 0..255
// See also ycbcrfunc.m from the image package and ./tests/cap_yuv.m
uint8NDArray imaq_handler::YCbCr_to_RGB (const octave_value& in, int ITU_standard)
{
  //LARGE_INTEGER PerformanceFrequency;
  //LARGE_INTEGER StartingCounts;
  //LARGE_INTEGER EndCounts;
  //QueryPerformanceFrequency(&PerformanceFrequency);
  //QueryPerformanceCounter(&StartingCounts);

  // prüfen, ob der input ein octave_scalar_map ist
  if (! in.isstruct ())
    error ("imaq_handler::YCbCr_to_RGB: IN must be a struct");

  const octave_scalar_map tmp = in.scalar_map_value ();

  // prüfen, ob die Felder Y, Cb, Cr vorhanden sind
  if (! tmp.contents ("Y").is_defined ())
    error ("imaq_handler::YCbCr_to_RGB: IN has no field 'Y'");

  if (! tmp.contents ("Cb").is_defined ())
    error ("imaq_handler::YCbCr_to_RGB: IN has no field 'Cb'");

  if (! tmp.contents ("Cr").is_defined ())
    error ("imaq_handler::YCbCr_to_RGB: IN has no field 'Cr'");

  uint8NDArray Y  = tmp.contents ("Y").uint8_array_value();
  uint32_t width  = Y.dims ()(1);
  uint32_t height = Y.dims ()(0);

  uint8NDArray Cb = tmp.contents ("Cb").uint8_array_value();
  uint8NDArray Cr = tmp.contents ("Cr").uint8_array_value();

  if ((Cb.dims ()(0) != Cr.dims ()(0)) || (Cb.dims ()(1) != Cr.dims ()(1)))
    error ("imaq_handler::YCbCr_to_RGB: this code expects, that Cb and Cr have the same size");

  int v_subs =  height / Cb.dims ()(0);
  int h_subs =  width / Cb.dims ()(1);

  //printf ("DEBUG: v_subs = %i\n", v_subs);
  //printf ("DEBUG: h_subs = %i\n", h_subs);

  if (v_subs * Cb.dims ()(0) != height)
    error ("imaq_handler::YCbCr_to_RGB: vertical subsampling is not an integer");

  if (h_subs * Cb.dims ()(1) != width)
    error ("imaq_handler::YCbCr_to_RGB: horizontal subsampling is not an integer");

  unsigned char *pY = reinterpret_cast<unsigned char*>(Y.fortran_vec());
  unsigned char *pCb = reinterpret_cast<unsigned char*>(Cb.fortran_vec());
  unsigned char *pCr = reinterpret_cast<unsigned char*>(Cr.fortran_vec());

  dim_vector dv (height, width, 3);
  uint8NDArray img (dv);
  unsigned char *pImg = reinterpret_cast<unsigned char*>(img.fortran_vec());

  double Kb = 0;
  double Kr = 0;
  switch (ITU_standard)
    {
    // ITU-R BT.601 (formerly CCIR 601)
    case 601:
      Kb = 0.114;
      Kr = 0.299;
      break;

    // ITU-R BT.709 standard
    case 709:
      Kb = 0.0722;
      Kr = 0.2116;
      break;
    // ITU-R BT.2020 standard
    case 2020:
      Kb = 0.0593;
      Kr = 0.2627;
      break;
    default:
      error ("imaq_handler:YCbCr_to_RGB: unknown ITU_standard %i", ITU_standard);
    }

  double Kg = 1 - Kb - Kr;

  // Directly based on
  // https://en.wikipedia.org/wiki/YCbCr

  double f02 = 2 - 2*Kr;
  double f11 = - Kb/Kg*(2-2*Kb);
  double f12 = - Kr/Kg*(2-2*Kr);
  double f21 = 2 - 2*Kb;

  for (octave_idx_type r = 0; r < height; r++)
    for (octave_idx_type c = 0; c < width; c++)
      {
        // step 1: undo footroom/headroom offset and scaling
        // y = nominal 0..1 but 8% overshoot and 6% undershoot due to filtering is possible
        // cb+cr = nominal -0.5..0.5 but 6% over- and undershoot possible

        double y  = (pY[r + c * height] - 16) / 219.0;
        int chroma_idx = r/v_subs + (c/h_subs) * (height/v_subs);
        double cb = (pCb[chroma_idx] - 16) / 224.0 - 0.5;
        double cr = (pCr[chroma_idx] - 16) / 224.0 - 0.5;

        double rv = 255 * (y + f02 * cr);
        double gv = 255 * (y + f11*cb + f12*cr);
        double bv = 255 * (y + f21*cb);

        rv = (rv > 255)? 255 : ((rv < 0)? 0 : rv);
        gv = (gv > 255)? 255 : ((gv < 0)? 0 : gv);
        bv = (bv > 255)? 255 : ((bv < 0)? 0 : bv);

        pImg [r + c * height + 0 * width * height] = rv;
        pImg [r + c * height + 1 * width * height] = gv;
        pImg [r + c * height + 2 * width * height] = bv;
      }

  //QueryPerformanceCounter(&EndCounts);
  //double d = (EndCounts.QuadPart - StartingCounts.QuadPart) / double (PerformanceFrequency.QuadPart);
  //printf ("d = %.3f ms\n", d * 1e3);

  return img;
}

// returns uint8 RGB image
uint8NDArray imaq_handler::JPG_to_RGB (const octave_value& in)
{
  // ToDo: Wie prüfen, ob es ein uint8 vector ist? Das müsste bei (M)JPEG der fall sein
  uint8NDArray img = in.uint8_array_value();
  int len = img.numel ();
  //printf ("len = %i\n", len);
  unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());

  struct jpeg_decompress_struct cinfo;
  struct jpeg_error_mgr jerr;

  cinfo.err = jpeg_std_error(&jerr);
  jpeg_create_decompress(&cinfo);
  jpeg_mem_src(&cinfo, p, len);

  int r = jpeg_read_header(&cinfo, TRUE);
  if (r != JPEG_HEADER_OK)
    fprintf (stderr, "header error r = %i\n", r);
  //fprintf (stderr, "JPEG_HEADER_TABLES_ONLY = %i\n", JPEG_HEADER_TABLES_ONLY);

  jpeg_start_decompress(&cinfo);

  int width = cinfo.output_width;
  int height = cinfo.output_height;
  int pixel_size = cinfo.output_components;

  assert (pixel_size == 3); // FIXME: kann es auch andere geben? Graustufen?

  //printf ("width = %d, height = %d, pixel_size = %d\n", width, height, pixel_size);

  // create uint8 RGB image
  dim_vector dv (3, width, height);
  uint8NDArray out_img (dv);
  unsigned char *p_out = reinterpret_cast<unsigned char*>(out_img.fortran_vec());

  while (cinfo.output_scanline < cinfo.output_height)
    {
      unsigned char *buffer_array[1];
      buffer_array[0] = p_out + (cinfo.output_scanline) * width * pixel_size;
      jpeg_read_scanlines(&cinfo, buffer_array, 1);
      //printf ("read line %i\n", cinfo.output_scanline);

    }
  //printf ("Proc: Done reading scanlines\n");

  jpeg_finish_decompress(&cinfo);
  jpeg_destroy_decompress(&cinfo);

  Array<octave_idx_type> perm (dim_vector (3, 1));
  perm(0) = 2;
  perm(1) = 1;
  perm(2) = 0;
  return out_img.permute (perm);
}

imaq_handler*
get_imaq_handler_from_ov (octave_value ov)
{
  //octave_stdout << "get_imaq_handler_from_ov ov.type_id() = " << ov.type_id() << std::endl;
  if (ov.type_id() != imaq_handler::static_type_id())
    {
      error("get_imaq_handler_from_ov: Not a valid imaq_handler");
      return 0;
    }

  imaq_handler* imgh = 0;
  const octave_base_value& rep = ov.get_rep();
  imgh = &((imaq_handler &)rep);
  return imgh;
}
