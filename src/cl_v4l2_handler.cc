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

#include "cl_v4l2_handler.h"

#define ARRAY_SIZE(a)	(sizeof(a)/sizeof((a)[0]))

static std::string
num2s (unsigned num) //taken from v4l2-ctl.cpp
{
  char buf[10];
  sprintf(buf, "%08x", num);
  return buf;
}

static std::string
buftype2s (int type) //taken from v4l2-ctl.cpp
{
  switch (type)
    {
    case 0:
      return "Invalid";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE:
      return "Video Capture";
    case V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE:
      return "Video Capture Multiplanar";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT:
      return "Video Output";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_MPLANE:
      return "Video Output Multiplanar";
    case V4L2_BUF_TYPE_VIDEO_OVERLAY:
      return "Video Overlay";
    case V4L2_BUF_TYPE_VBI_CAPTURE:
      return "VBI Capture";
    case V4L2_BUF_TYPE_VBI_OUTPUT:
      return "VBI Output";
    case V4L2_BUF_TYPE_SLICED_VBI_CAPTURE:
      return "Sliced VBI Capture";
    case V4L2_BUF_TYPE_SLICED_VBI_OUTPUT:
      return "Sliced VBI Output";
    case V4L2_BUF_TYPE_VIDEO_OUTPUT_OVERLAY:
      return "Video Output Overlay";
    default:
      return "Unknown (" + num2s(type) + ")";
    }
}

static struct { //taken from yavta Copyright (C) 2005-2010 Laurent Pinchart
  const char *name;
  unsigned int fourcc;
} pixel_formats[] = {
  { "RGB332", V4L2_PIX_FMT_RGB332 },
  { "RGB555", V4L2_PIX_FMT_RGB555 },
  { "RGB565", V4L2_PIX_FMT_RGB565 },
  { "RGB555X", V4L2_PIX_FMT_RGB555X },
  { "RGB565X", V4L2_PIX_FMT_RGB565X },
  { "BGR24", V4L2_PIX_FMT_BGR24 },
  { "RGB24", V4L2_PIX_FMT_RGB24 },
  { "BGR32", V4L2_PIX_FMT_BGR32 },
  { "RGB32", V4L2_PIX_FMT_RGB32 },
  { "Y8", V4L2_PIX_FMT_GREY },
  { "Y10", V4L2_PIX_FMT_Y10 },
  { "Y12", V4L2_PIX_FMT_Y12 },
  { "Y16", V4L2_PIX_FMT_Y16 },
  { "UYVY", V4L2_PIX_FMT_UYVY },
  { "VYUY", V4L2_PIX_FMT_VYUY },
  { "YUYV", V4L2_PIX_FMT_YUYV },
  { "YVYU", V4L2_PIX_FMT_YVYU },
  { "NV12", V4L2_PIX_FMT_NV12 },
  { "NV21", V4L2_PIX_FMT_NV21 },
  { "NV16", V4L2_PIX_FMT_NV16 },
  { "NV61", V4L2_PIX_FMT_NV61 },
//  { "NV24", V4L2_PIX_FMT_NV24 },
//  { "NV42", V4L2_PIX_FMT_NV42 },
  { "SBGGR8", V4L2_PIX_FMT_SBGGR8 },
  { "SGBRG8", V4L2_PIX_FMT_SGBRG8 },
  { "SGRBG8", V4L2_PIX_FMT_SGRBG8 },
  { "SRGGB8", V4L2_PIX_FMT_SRGGB8 },
//  { "SBGGR10_DPCM8", V4L2_PIX_FMT_SBGGR10DPCM8 },
//  { "SGBRG10_DPCM8", V4L2_PIX_FMT_SGBRG10DPCM8 },
  { "SGRBG10_DPCM8", V4L2_PIX_FMT_SGRBG10DPCM8 },
//  { "SRGGB10_DPCM8", V4L2_PIX_FMT_SRGGB10DPCM8 },
  { "SBGGR10", V4L2_PIX_FMT_SBGGR10 },
  { "SGBRG10", V4L2_PIX_FMT_SGBRG10 },
  { "SGRBG10", V4L2_PIX_FMT_SGRBG10 },
  { "SRGGB10", V4L2_PIX_FMT_SRGGB10 },
  { "SBGGR12", V4L2_PIX_FMT_SBGGR12 },
  { "SGBRG12", V4L2_PIX_FMT_SGBRG12 },
  { "SGRBG12", V4L2_PIX_FMT_SGRBG12 },
  { "SRGGB12", V4L2_PIX_FMT_SRGGB12 },
  { "DV", V4L2_PIX_FMT_DV },
  { "MJPEG", V4L2_PIX_FMT_MJPEG },
  { "MPEG", V4L2_PIX_FMT_MPEG },
};

static std::string v4l2_fourcc_name(unsigned int fourcc)
{
  static char name[5];
  for (int i = 0; i < 4; ++i) {
    name[i] = fourcc & 0xff;
    fourcc >>= 8;
  }
  name[4] = '\0';
  return string(name);
}

static std::string v4l2_format_name(unsigned int fourcc)
{
  for (unsigned int i = 0; i < ARRAY_SIZE(pixel_formats); ++i) {
    if (pixel_formats[i].fourcc == fourcc)
      return string(pixel_formats[i].name);
  }
  return v4l2_fourcc_name(fourcc);
}

static unsigned int v4l2_format_code(const char *name)
{
  unsigned int i;

  for (i = 0; i < ARRAY_SIZE(pixel_formats); ++i) {
    if (strcasecmp(pixel_formats[i].name, name) == 0)
      return pixel_formats[i].fourcc;
  }

  //try fourcc format
  unsigned int fourcc = 0;
  for (int i = 3; i >=0; i--) {
    fourcc <<= 8;
    fourcc += name[i];
  }
  return fourcc;
}

DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(v4l2_handler, "v4l2_handler", "v4l2_handler");

v4l2_handler::v4l2_handler ()
  : octave_base_value(),
    fd(-1), n_buffer(0), buffers(0), streaming(0),
    preview_window(0)
{
  //octave_stdout << "v4l2_handler C'Tor " << endl;
}

v4l2_handler::v4l2_handler (const v4l2_handler& m)
  : octave_base_value()
{
  octave_stdout << "v4l2_handler: the copy constructor shouldn't be called" << std::endl;
}

v4l2_handler::~v4l2_handler ()
{
  //octave_stdout << "v4l2_handler D'Tor " << endl;

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
v4l2_handler::print (std::ostream& os, bool pr_as_read_syntax = false) const
{
  os << "This is class v4l2_handler" << endl;
  os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}

// calls to xioctl should never fail.
// If it fails something unexpected happened
void
v4l2_handler::xioctl_name (int fh, unsigned long int request, void *arg, const char* name, const char* file, const int line)
{
  int r;
  do
    {
      r = v4l2_ioctl(fh, request, arg);
    }
  while (r == -1 && ((errno == EINTR) || (errno == EAGAIN)));

  if (r == -1)
    {
      error("%s:%i xioctl %s error %d, %s\n", file, line, name, errno, strerror(errno));
    }
}

void
v4l2_handler::open (string d)
{
  fd = v4l2_open(d.c_str(), O_RDWR | O_NONBLOCK, 0);
  if (fd < 0)
    {
      error("Cannot open device '%s'. Error %d, '%s'\n", d.c_str(), errno, strerror(errno));
    }
  else
    {
      dev = d;
    }
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-querycap.html
 * v4l2-ctl -D
 * \return octave_scalar_map with device capabilities
 */
octave_value
v4l2_handler::querycap ()
{
  struct v4l2_capability cap;
  CLEAR(cap);
  xioctl (fd, VIDIOC_QUERYCAP, &cap);

  octave_scalar_map st;
  if (!error_state)
    {
      st.assign ("driver",    std::string((const char*)cap.driver));
      st.assign ("card",      std::string((const char*)cap.card));
      st.assign ("bus_info",  std::string((const char*)cap.bus_info));

      char tmp[15];
      snprintf (tmp, 15, "%u.%u.%u", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
      st.assign ("version",   std::string(tmp));
      st.assign ("capabilities", (unsigned int)(cap.capabilities));
    }
  return octave_value (st);
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-input.html
 */
int
v4l2_handler::g_input ()
{
  int index;
  xioctl (fd, VIDIOC_G_INPUT, &index);
  return index;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-input.html
 */
void
v4l2_handler::s_input (int index)
{
  xioctl (fd, VIDIOC_S_INPUT, &index);
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enuminput.html
 * see also output from "v4l2-ctl -n"
 * \return octave_map with the enumeration of all inputs
 */
octave_value
v4l2_handler::enuminput ()
{
  octave_map ret;
  struct v4l2_input inp;
  CLEAR (inp);
  inp.index = 0;
  while (v4l2_ioctl (fd, VIDIOC_ENUMINPUT, &inp) >= 0)
    {
      octave_scalar_map st;
      st.assign ("name", std::string((const char*)inp.name));
      switch (inp.type)
        {
        case V4L2_INPUT_TYPE_TUNER:
          st.assign ("type", "V4L2_INPUT_TYPE_TUNER");
          break;
        case V4L2_INPUT_TYPE_CAMERA:
          st.assign ("type", "V4L2_INPUT_TYPE_CAMERA");
          break;
        }

      st.assign ("audioset",     (unsigned int)inp.audioset);
      st.assign ("tuner",        (unsigned int)inp.tuner);
      st.assign ("std",          (unsigned int)inp.std);
      st.assign ("status",       (unsigned int)inp.status);
      st.assign ("capabilities", (unsigned int)inp.capabilities);

      ret.assign(octave_idx_type(inp.index), st);
      inp.index++;
    }

  return octave_value(ret);
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-fmt.html
 * see also "v4l2-ctl -w --list-formats"
 * \return octave_map with available video formats
 */
octave_value
v4l2_handler::enum_fmt (enum v4l2_buf_type type)
{
  octave_map ret;
  struct v4l2_fmtdesc fmt;
  CLEAR(fmt);
  fmt.index = 0;
  fmt.type = type;
  while (v4l2_ioctl (fd, VIDIOC_ENUM_FMT, &fmt) >= 0)
    {
      octave_scalar_map sm;
      sm.assign ("type", buftype2s(fmt.type));
      sm.assign ("description", std::string((const char*)fmt.description));
      sm.assign ("pixelformat", std::string(v4l2_format_name(fmt.pixelformat)));
      sm.assign ("fourcc", std::string(v4l2_fourcc_name(fmt.pixelformat)));
      sm.assign ("flags_compressed", fmt.flags == V4L2_FMT_FLAG_COMPRESSED);
      sm.assign ("flags_emulated", fmt.flags == V4L2_FMT_FLAG_EMULATED);

      ret.assign(octave_idx_type(fmt.index), sm);
      fmt.index++;
    }
  return octave_value(ret);
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-framesizes.html
 * see also v4l2-ctl --list-formats-ext
 * \param pixel_format e.g. 'RGB24'
 * \return Nx2 Matrix with width, height
 * \sa enum_frameintervals
 */
Matrix
v4l2_handler::enum_framesizes (string pixelformat)
{
  Matrix ret;
  __u32 pfcode = v4l2_format_code(pixelformat.c_str());
  struct v4l2_frmsizeenum frmsize;
  CLEAR(frmsize);
  frmsize.pixel_format = pfcode;
  frmsize.index = 0;
  while (v4l2_ioctl(fd, VIDIOC_ENUM_FRAMESIZES, &frmsize) >= 0)
    {
      if (frmsize.type == V4L2_FRMSIZE_TYPE_DISCRETE)
        {
          ret.resize(frmsize.index+1, 2);
          ret(frmsize.index, 0) = frmsize.discrete.width;
          ret(frmsize.index, 1) = frmsize.discrete.height;
        }
      else
        error("frmsize.type not implemented");
      frmsize.index++;
    }
  return ret;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enum-frameintervals.html
 * see also v4l2-ctl --list-formats-ext
 * \param pixel_format e.g. 'RGB24'
 * \param width in px
 * \param height in px
 * \return Nx2 matrix with frame interval numerator, denominator
 * \sa enum_framesizes
 */
Matrix
v4l2_handler::enum_frameintervals (string pixelformat, __u32 width, __u32 height)
{
  Matrix ret;
  __u32 pfcode = v4l2_format_code(pixelformat.c_str());
  struct v4l2_frmivalenum frmival;
  CLEAR(frmival);
  frmival.pixel_format = pfcode;
  frmival.width = width;
  frmival.height = height;
  frmival.index = 0;
  while (v4l2_ioctl(fd, VIDIOC_ENUM_FRAMEINTERVALS, &frmival) >= 0)
    {
      if (frmival.type == V4L2_FRMIVAL_TYPE_DISCRETE)
        {
          ret.resize(frmival.index+1, 2);
          ret(frmival.index, 0) = frmival.discrete.numerator;
          ret(frmival.index, 1) = frmival.discrete.denominator;
        }
      else if (frmival.type == V4L2_FRMIVAL_TYPE_STEPWISE)
        {
          error("Sorry, this isn't implemented yet"); //TODO
        }
      frmival.index++;
    }
  return ret;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-parm.html
 */
Matrix
v4l2_handler::g_parm ()
{
  Matrix ret(1,2);
  struct v4l2_streamparm sparam;
  CLEAR(sparam);
  sparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_G_PARM, &sparam);
  if(!error_state)
    {
      if(sparam.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
        {
          const struct v4l2_fract &tf = sparam.parm.capture.timeperframe;
          ret(0) = tf.numerator;
          ret(1) = tf.denominator;
        }
      else
        {
          warning("v4l2_handler::g_parm: V4L2_CAP_TIMEPERFRAME is not supported");
          return Matrix(0,0);
        }
    }
  return ret;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-parm.html
 */
void
v4l2_handler::s_parm (Matrix timeperframe)
{
  struct v4l2_streamparm sparam;
  CLEAR(sparam);
  sparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_G_PARM, &sparam);
  if (sparam.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)
    {
      sparam.parm.capture.timeperframe.numerator = timeperframe(0);
      sparam.parm.capture.timeperframe.denominator = timeperframe(1);
      xioctl(fd, VIDIOC_S_PARM, &sparam);
      struct v4l2_fract *tf = &sparam.parm.capture.timeperframe;
      if (!tf->denominator || !tf->numerator)
        error("v4l2_handler::s_parm: Invalid framerate");

      if (tf->numerator != __u32(timeperframe(0)) || tf->denominator != __u32(timeperframe(1)))
        warning("v4l2_handler::s_parm: driver is using %d/%d as timeperframe but %d/%d was requested",
                tf->numerator, tf->denominator, __u32(timeperframe(0)), __u32(timeperframe(1)));
    }
  else
    {
       warning("v4l2_handler::s_parm: V4L2_CAP_TIMEPERFRAME is not supported");
    }
}

// get octave_scalar_map from v4l2_queryctrl
octave_scalar_map
v4l2_handler::get_osm (struct v4l2_queryctrl queryctrl)
{
  octave_scalar_map ctrl;
  ctrl.assign ("id", int(queryctrl.id));
  //ctrl.assign ("value", g_ctrl(queryctrl.id));
  ctrl.assign ("min", int(queryctrl.minimum));
  ctrl.assign ("max", int(queryctrl.maximum));
  if (queryctrl.type == V4L2_CTRL_TYPE_INTEGER)
    ctrl.assign ("step", int(queryctrl.step));

  if ( queryctrl.type == V4L2_CTRL_TYPE_INTEGER
       ||queryctrl.type == V4L2_CTRL_TYPE_BOOLEAN
       ||queryctrl.type == V4L2_CTRL_TYPE_MENU)
    ctrl.assign ("default", int(queryctrl.default_value));

  if (queryctrl.type == V4L2_CTRL_TYPE_MENU)
    {
      struct v4l2_querymenu querymenu;
      CLEAR(querymenu);
      stringstream menu_str;

      querymenu.id = queryctrl.id;

      for (querymenu.index = queryctrl.minimum;
           int(querymenu.index) <= queryctrl.maximum;
           querymenu.index++)
        {
          if (0 == ioctl (fd, VIDIOC_QUERYMENU, &querymenu))
            {
              menu_str << querymenu.index << ":" << querymenu.name << ";";
            }
        }
      ctrl.assign ("menu", menu_str.str());
    }
  return ctrl;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-queryctrl.html
 * or better http://www.linuxtv.org/downloads/v4l-dvb-apis/extended-controls.html
 * because most "Exposure" ctrls are "extended controls"
 *
 * see also v4l2-ctl -L
 *
 * Use id for calls to s_ctrl
 * \sa s_ctrl
 */
octave_value
v4l2_handler::queryctrl ()
{
  struct v4l2_queryctrl queryctrl;
  CLEAR(queryctrl);

  octave_scalar_map ctrls;
  queryctrl.id = V4L2_CTRL_FLAG_NEXT_CTRL;
  while (0 == ioctl (fd, VIDIOC_QUERYCTRL, &queryctrl))
    {
      if (queryctrl.flags & V4L2_CTRL_FLAG_DISABLED)
        continue;
      // convert name to lower, replace spaces with _ and remove others
      std::string field;
      const char* n= (const char*)queryctrl.name;
      int len = strlen(n);
      for (int i=0; i<len; ++i)
        {
          char c = tolower(n[i]);
          if(!islower(c))
            {
              if (c !=' ')
                continue;
              else
                c = '_';
            }
          field.append(1, c);
        }
      ctrls.assign (field, get_osm(queryctrl));
      queryctrl.id |= V4L2_CTRL_FLAG_NEXT_CTRL;
    }
  return ctrls;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/control.html
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-ctrl.html
 */
int
v4l2_handler::g_ctrl (int id)
{
  struct v4l2_control control;
  CLEAR(control);
  control.id = id;
  xioctl(fd, VIDIOC_G_CTRL, &control);
  return control.value;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/control.html
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-ctrl.html
 */
void
v4l2_handler::s_ctrl (int id, int value)
{
  struct v4l2_control control;
  CLEAR(control);
  control.id = id;
  control.value = value;
  xioctl(fd, VIDIOC_S_CTRL, &control);
}

/*!
 * \param xres the width of the image
 * \param yres the height of the image
 * \param fmt pixelformat
 *
 * The used libv4l2 pixelformat is set to fmt, V4L2_FIELD_INTERLACED
 */
void
v4l2_handler::s_fmt (string fmtstr, __u32 xres, __u32 yres)
{
  if (streaming)
    {
      error("v4l2_handler::s_fmt: you have to stop streaming first");
    }
  else
    {
      struct v4l2_format fmt;
      CLEAR(fmt);
      fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(fd, VIDIOC_G_FMT, &fmt);

      if(xres && yres)
        {
          fmt.fmt.pix.width       = xres;
          fmt.fmt.pix.height      = yres;
        }
      unsigned int fmt_code = 0;
      if (!fmtstr.empty())
        {
          fmt_code = v4l2_format_code(fmtstr.c_str());
          fmt.fmt.pix.pixelformat = fmt_code;
        }
      fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
      xioctl(fd, VIDIOC_S_FMT, &fmt);
      if (fmt_code && fmt.fmt.pix.pixelformat != fmt_code)
        {
          warning("v4l2_handler::s_fmt: Libv4l changed the pixelformat from\n         '%s'(0x%x) to '%s'(0x%x)",
                  v4l2_format_name(fmt_code).c_str(), fmt_code,
                  v4l2_format_name(fmt.fmt.pix.pixelformat).c_str(), fmt.fmt.pix.pixelformat);
        }
      if (xres && yres && ((fmt.fmt.pix.width != xres) || (fmt.fmt.pix.height != yres)))
        warning("v4l2_handler::s_fmt: Driver is sending image at %dx%d although %dx%d was requested",
                fmt.fmt.pix.width, fmt.fmt.pix.height, xres, yres);
    }
}

octave_scalar_map
v4l2_handler::g_fmt ()
{
  struct v4l2_format fmt;
  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_G_FMT, &fmt);
  Matrix s(1,2);
  s(0) = fmt.fmt.pix.width;
  s(1) = fmt.fmt.pix.height;

  octave_scalar_map ret;
  ret.assign ("size", s);
  ret.assign ("pixelformat", std::string(v4l2_format_name(fmt.fmt.pix.pixelformat)));
  return ret;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html
 * \param n number of buffers to initiate. A count value of zero frees all buffers.
 */
void
v4l2_handler::reqbufs (unsigned int n)
{
  if (fd>=0)
    {
      struct v4l2_requestbuffers req;
      CLEAR(req);
      req.count = n;
      req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      req.memory = V4L2_MEMORY_MMAP;
      xioctl(fd, VIDIOC_REQBUFS, &req);
      if (req.count<n)
        error("v4l2_handler::reqbufs: VIDIOC_REQBUFS: running out of free memory\n");
      n_buffer = req.count;
    }
}

void
v4l2_handler::mmap ()
{
  struct v4l2_buffer buf;
  buffers = (buffer*)calloc(n_buffer, sizeof(*buffers));
  for (unsigned int i = 0; i < n_buffer; ++i)
    {
      CLEAR(buf);
      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = i;

      xioctl(fd, VIDIOC_QUERYBUF, &buf);

      buffers[i].length = buf.length;
      buffers[i].start  = v4l2_mmap(NULL, buf.length,
                                    PROT_READ | PROT_WRITE, MAP_SHARED,
                                    fd, buf.m.offset);

      if (buffers[i].start == MAP_FAILED)
        {
          error("v4l2_handler::mmap: MAP_FAILED %s", strerror(errno));
        }
    }
}

void
v4l2_handler::qbuf ()
{
  struct v4l2_buffer buf;
  for (unsigned int i = 0; i < n_buffer; ++i)
    {
      CLEAR(buf);
      buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = V4L2_MEMORY_MMAP;
      buf.index = i;
      // enqueu buffer
      xioctl(fd, VIDIOC_QBUF, &buf);
    }
}

/*!
 * \param nargout Number of output Parameter [image, sequence, timestamp, timecode]
 * \param preview 0=no preview, 1=show preview win if closed, 2=leave it closed
 * \return image, sequence, timestamp, [timecode]
 */
octave_value_list
v4l2_handler::capture (int nargout, int preview)
{
  octave_value_list ret;

  if(!streaming)
    {
      error("v4l2_handler::capture: Streaming wasn't enabled. Please use 'start(obj)'");
      return octave_value();
    }
  struct v4l2_format fmt;
  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_G_FMT, &fmt);

  struct v4l2_buffer buf;

  fd_set fds;
  struct timeval tv;
  int r = -1;

  // wait for image
  do
    {
      FD_ZERO(&fds);
      FD_SET(fd, &fds);

      // 2s Timeout TODO: make this configurable
      tv.tv_sec = 2;
      tv.tv_usec = 0;
      r = select(fd + 1, &fds, NULL, NULL, &tv);
    }
  while ((r == -1 && (errno == EINTR)));
  if (r == -1)
    {
      error("v4l2_handler::capture: Select failed.");
      return octave_value();
    }

  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  // dequeue buffers
  xioctl(fd, VIDIOC_DQBUF, &buf);

  // calculate real fps
  static double last_timestamp = 0;
  double timestamp = buf.timestamp.tv_sec+buf.timestamp.tv_usec/1.0e6;
  double dt = (last_timestamp)? timestamp - last_timestamp: -1;
  last_timestamp = timestamp;

  //debug output
  //octave_stdout << "INFO: pixelformat = " << v4l2_format_name(fmt.fmt.pix.pixelformat) << endl;
  //octave_stdout << "INFO: width = " << fmt.fmt.pix.width << ", height = " << fmt.fmt.pix.height << endl;
  //octave_stdout << "INFO: Bytes captured = " << buf.bytesused << endl;

  if ((fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24) && preview)
    {
      error("v4l2_handler::capture: Preview is only available if VideoFormat is 'RGB3' aka 'RGB24' (V4L2_PIX_FMT_RGB24)");
      preview = false;
    }

  if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24)
    // RGB3 aka RGB24
    // return [height x width x 3] uint8 matrix
    {
      dim_vector dv (3, fmt.fmt.pix.width, fmt.fmt.pix.height);
      uint8NDArray img (dv);
      assert(img.numel() == int(buf.bytesused));

      unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
      memcpy(p, buffers[buf.index].start, buf.bytesused);

      Array<octave_idx_type> perm (dim_vector (3, 1));
      perm(0) = 2;
      perm(1) = 1;
      perm(2) = 0;

      ret(0) = octave_value(img.permute (perm));
    }
  else if (  fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SBGGR10
          || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG10
          || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SRGGB10
          || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SBGGR12
          || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SGBRG12
          || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG12
          || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SRGGB12 )
    // RAW Bayer, 2 bytes per pixel
    // return [height * width] uint16 matrix
    {
      dim_vector dv (fmt.fmt.pix.width, fmt.fmt.pix.height);
      uint16NDArray img (dv);
      assert(img.numel()*2 == int(buf.bytesused));
      unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
      memcpy(p, buffers[buf.index].start, buf.bytesused);
      ret(0) = octave_value(img. transpose ());
    }
  else if ( fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SBGGR8
         || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SGBRG8
         || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SGRBG8
         || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_SRGGB8 )
    // RAW Bayer, 1 byte per pixel
    // return [height * width] uint8 matrix
    {
      dim_vector dv (fmt.fmt.pix.width, fmt.fmt.pix.height);
      uint8NDArray img (dv);
      assert(img.numel() == int(buf.bytesused));
      unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
      memcpy(p, buffers[buf.index].start, buf.bytesused);
      ret(0) = octave_value(img.transpose ());
    }
  else if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUYV)
    // YUYV aka YUV 4:2:2
    // http://www.linuxtv.org/downloads/v4l-dvb-apis/V4L2-PIX-FMT-YUYV.html
    // return struct with fields Y, Cb, Cr
    {
      dim_vector dvy  (fmt.fmt.pix.width, fmt.fmt.pix.height);
      dim_vector dvc (fmt.fmt.pix.width/2, fmt.fmt.pix.height);
      uint8NDArray y (dvy);
      uint8NDArray cb (dvc);
      uint8NDArray cr (dvc);
      assert ((y.numel() + cb.numel() + cr.numel()) == int(buf.bytesused));
      unsigned int i;
      unsigned char *s = reinterpret_cast<unsigned char*>(buffers[buf.index].start);
      for (i=0; i < (fmt.fmt.pix.width * fmt.fmt.pix.height); ++i)
        y(i) = s[2 * i];
      for (i=0; i < (fmt.fmt.pix.width * fmt.fmt.pix.height / 2); ++i)
        {
          cb(i) = s[4 * i + 1];
          cr(i) = s[4 * i + 3];
        }

      octave_scalar_map img;
      img.assign ("Y", y.transpose ());
      img.assign ("Cb", cb.transpose ());
      img.assign ("Cr", cr.transpose ());
      ret(0) = octave_value(img);
    }
  else if (   fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YVU420
           || fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420)
    // YVU420 aka YV12
    // http://www.linuxtv.org/downloads/v4l-dvb-apis/re23.html
    {
      dim_vector dvy  (fmt.fmt.pix.width, fmt.fmt.pix.height);
      dim_vector dvc (fmt.fmt.pix.width/2, fmt.fmt.pix.height/2);
      uint8NDArray y (dvy);
      uint8NDArray c1 (dvc);
      uint8NDArray c2 (dvc);
      assert ((y.numel() + c1.numel() + c2.numel()) == int(buf.bytesused));

      // Y
      unsigned char *p = reinterpret_cast<unsigned char*>(y.fortran_vec());
      memcpy(p, buffers[buf.index].start, y.numel ());

      // C1
      p = reinterpret_cast<unsigned char*>(c1.fortran_vec());
      memcpy(p, (unsigned char*)buffers[buf.index].start + y.numel (), c1.numel ());

      // C2
      p = reinterpret_cast<unsigned char*>(c2.fortran_vec());
      memcpy(p, (unsigned char*)buffers[buf.index].start + y.numel () + c1.numel (), c2.numel ());

      octave_scalar_map img;
      img.assign ("Y", y.transpose ());
      if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_YUV420)
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

      ret(0) = octave_value(img);
    }
  else
    // No conversion for this format
    // http://www.linuxtv.org/downloads/v4l-dvb-apis/ch02s10.html
    // Just return the bytes as vector in this case.
    {
      //octave_stdout << "INFO: No conversion for "
      //              << v4l2_format_name(fmt.fmt.pix.pixelformat)
      //              << " implemented, returning raw stream..." << endl;

      dim_vector dv (buf.bytesused, 1);
      uint8NDArray img (dv);
      unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
      memcpy(p, buffers[buf.index].start, buf.bytesused);
      ret(0) = octave_value(img);
    }

  if (nargout > 1)
    ret(1) = octave_value(buf.sequence);

  if (nargout > 2)
    {
      // add timestamp to frame
      octave_scalar_map timestamp;
      timestamp.assign ("tv_sec", (long int)(buf.timestamp.tv_sec));
      timestamp.assign ("tv_usec", (long int)(buf.timestamp.tv_usec));
      ret(2) = octave_value(timestamp);
    }

  if (nargout > 3)
    {
      if (buf.flags & V4L2_BUF_FLAG_TIMECODE)
        {
          octave_scalar_map timecode;
          timecode.assign ("type", int(buf.timecode.type));
          timecode.assign ("flags", int(buf.timecode.flags));
          timecode.assign ("frames", int(buf.timecode.frames));
          timecode.assign ("seconds", int(buf.timecode.seconds));
          timecode.assign ("minutes", int(buf.timecode.minutes));
          timecode.assign ("hours", int(buf.timecode.hours));
          ret(3) = octave_value(timecode);
        }
      else
        {
          warning("v4l2_handler::capture: Timecode not available");
          ret(3) = octave_value();
        }
    }

  // use preview window?
  if (preview)
    {
      if (!preview_window)
        {
          preview_window = new img_win(10, 10, fmt.fmt.pix.width, fmt.fmt.pix.height);
          preview_window->show();
        }
      if (preview_window)
        {
          if(preview == 1 && !preview_window->shown())
            preview_window->show();

          // We can only use preview for RGB24 aka RGB3
          if (fmt.fmt.pix.pixelformat == V4L2_PIX_FMT_RGB24)
            {
              // octave_stdout << "Bytes captured = " << buf.bytesused << endl;
              // sanity checks
              if (buf.bytesused != (3 *  fmt.fmt.pix.width * fmt.fmt.pix.height))
                error ("v4l2_handler::capture: Returned size of buffer doesn't match 3 * width * height");
              else
                {
                  preview_window->copy_img(reinterpret_cast<unsigned char*>(buffers[buf.index].start),
                                           fmt.fmt.pix.width, fmt.fmt.pix.height, 1);
                  preview_window->custom_label(dev.c_str(), buf.sequence, 1.0/dt);
                }
            }
        }
    }
  else if (preview_window)
    {
      delete preview_window;
      preview_window = 0;
    }

  xioctl(fd, VIDIOC_QBUF, &buf);
  return ret;
}

/*!
 * Main purpose is for debugging this class
 */
void
v4l2_handler::capture_to_ppm (const char *fn)
{
  uint8NDArray img = capture (1, 0)(0).uint8_array_value();
  Matrix per(3,1);
  per(0) = 2;
  per(1) = 1;
  per(2) = 0;
  img = img.permute(per);

  unsigned char* p=reinterpret_cast<unsigned char*>(img.fortran_vec());
  FILE *fout = fopen (fn, "w");
  if (!fout)
    {
      error("v4l2_handler::capture_to_ppm: Cannot open file '%s'", fn);
    }
  fprintf (fout, "P6\n%d %d 255\n",
           img.dim2(), img.dim3());
  fwrite (p, img.numel(), 1, fout);
  fclose (fout);
}

/*!
 * - Set field to V4L2_FIELD_INTERLACED
 * - Requests buffers
 * - mmap the buffers
 * - enque the buffers
 * - start streaming
 */
void
v4l2_handler::streamon (unsigned int n)
{
  if(streaming)
    {
      warning("v4l2_handler::streamon: Streaming already enabled. Buffer size unchanged.");
    }
  else
    {
      // set needed pixelformat and field
      s_fmt("", 0, 0);
      // request buffers
      reqbufs(n);
      // mmap the buffers
      mmap();
      // enque the buffers
      qbuf();

      enum   v4l2_buf_type type;
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(fd, VIDIOC_STREAMON, &type);
      if (!error_state)
        streaming = 1;
    }
}

/*!
 * - Stop streaming
 * - unmap the buffers
 * - free buffers
 */
void
v4l2_handler::streamoff ()
{
  if(streaming)
    {
      if (preview_window)
        preview_window->hide();
      Fl::wait(0);
      enum   v4l2_buf_type type;
      type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      xioctl(fd, VIDIOC_STREAMOFF, &type);

      streaming = 0;
    }
  // unmap the buffers
  munmap();
  // free the buffers
  reqbufs(0);
}

void
v4l2_handler::munmap ()
{
  if(buffers)
    {
      for (unsigned int i = 0; i < n_buffer; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);
      free(buffers);
      buffers = 0;
    }
}

void
v4l2_handler::close ()
{
  streamoff();
  if (fd >= 0)
    v4l2_close(fd);
  fd = -1;
}

v4l2_handler*
get_v4l2_handler_from_ov (octave_value ov)
{
  static bool type_loaded = false;
  if (!type_loaded)
    {
      v4l2_handler::register_type();
      type_loaded = true;
    }

  if (ov.type_id() != v4l2_handler::static_type_id())
    {
      error("get_v4l2_handler_from_ov: Not a valid v4l2_handler");
      return 0;
    }

  v4l2_handler* imgh = 0;
  const octave_base_value& rep = ov.get_rep();
  imgh = &((v4l2_handler &)rep);
  return imgh;
}
