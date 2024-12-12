// Copyright (C) 2014-2024 Andreas Weber <andy.weber.aw@gmail.com>
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

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(imaq_handler, "imaq_handler", "imaq_handler");

bool imaq_handler::type_loaded = false;

imaq_handler::imaq_handler ()
  : octave_base_value(), preview_window(0)
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
  //~ : octave_base_value()
//~ {
  //~ octave_stdout << "v4l2_handler: the copy constructor shouldn't be called" << std::endl;
//~ }

imaq_handler::~imaq_handler ()
{
  //octave_stdout << "imaq_handler D'Tor " << endl;

  // delete preview_window if active
  if (preview_window)
    {
      delete preview_window;
      preview_window = 0;
    }

  // stop streaming, unmap & free buffers, close v4l2 device
  close();
}

void
imaq_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class imaq_handler" << endl;
  //os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}


octave_scalar_map
imaq_handler::open (string d, bool quiet)
{
  octave_scalar_map ret;
  octave_stdout << "imaq_handler::open (d = " << d << " called" << std::endl;
  return ret;
}

void
imaq_handler::close ()
{
  //streamoff();
  //if (fd >= 0)
  //  v4l2_close(fd);
  //fd = -1;
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
// http://www.linuxtv.org/downloads/v4l-dvb-apis/re23.html
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

// Just return the bytes as vector
octave_value imaq_handler::get_raw_bytes (void *start, size_t length)
{
	dim_vector dv (length, 1);
	uint8NDArray img (dv);
	unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
	memcpy(p, start, length);
	return octave_value(img);
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
