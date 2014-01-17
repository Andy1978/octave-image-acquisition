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

static std::string num2s(unsigned num) //taken from v4l2-ctl.cpp
{
  char buf[10];
  sprintf(buf, "%08x", num);
  return buf;
}

static std::string buftype2s(int type) //taken from v4l2-ctl.cpp
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

static std::string fcc2s(unsigned int val) //taken from v4l2-ctl.cpp
{
  std::string s;

  s += val & 0xff;
  s += (val >> 8) & 0xff;
  s += (val >> 16) & 0xff;
  s += (val >> 24) & 0xff;
  return s;
}

DEFINE_OCTAVE_ALLOCATOR(v4l2_handler);
DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(v4l2_handler, "v4l2_handler", "v4l2_handler");

v4l2_handler::v4l2_handler()
  : octave_base_value(),
    fd(-1), n_buffer(0), buffers(0), streaming(0),
    preview_window(0)
{
  //octave_stdout << "v4l2_handler C'Tor " << endl;
}

v4l2_handler::v4l2_handler(const v4l2_handler& m)
  : octave_base_value()
{
  octave_stdout << "v4l2_handler: the copy constructor shouldn't be called" << std::endl;
}

v4l2_handler::~v4l2_handler()
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

void v4l2_handler::print(std::ostream& os, bool pr_as_read_syntax = false) const
{
  os << "This is class v4l2_handler" << endl;
  os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}

// calls to xioctl should never fail.
// If it fails something unexpected happened
void v4l2_handler::xioctl_name(int fh, unsigned long int request, void *arg, const char* name, const char* file, const int line)
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

void v4l2_handler::open(string d)
{
  fd = v4l2_open(d.c_str(), O_RDWR | O_NONBLOCK, 0);
  if (fd < 0)
    {
      error("Cannot open device %s. Error %d, %s\n", d.c_str(), errno, strerror(errno));
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
octave_value v4l2_handler::querycap()
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
      snprintf(tmp, 15, "%u.%u.%u", (cap.version >> 16) & 0xFF, (cap.version >> 8) & 0xFF, cap.version & 0xFF);
      st.assign ("version",   std::string(tmp));
      st.assign ("capabilities", (unsigned int)(cap.capabilities));
    }
  return octave_value (st);
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-input.html
 */
int v4l2_handler::g_input()
{
  int index;
  xioctl (fd, VIDIOC_G_INPUT, &index);
  return index;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-input.html
 */
void v4l2_handler::s_input(int index)
{
  xioctl (fd, VIDIOC_S_INPUT, &index);
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-enuminput.html
 * see also output from "v4l2-ctl -n"
 * \return octave_map with the enumeration of all inputs
 */
octave_value v4l2_handler::enuminput()
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
octave_value v4l2_handler::enum_fmt(enum v4l2_buf_type type)
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
      sm.assign ("pixelformat", fcc2s(fmt.pixelformat));
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
 * \param pixel_format constant e.g. V4L2_PIX_FMT_RGB24
 * \return Nx2 Matrix with width, height
 * \sa enum_frameintervals
 */
Matrix v4l2_handler::enum_framesizes(__u32 pixel_format)
{
  Matrix ret;
  struct v4l2_frmsizeenum frmsize;
  CLEAR(frmsize);
  frmsize.pixel_format = pixel_format;
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
 * \param pixel_format constant e.g. V4L2_PIX_FMT_RGB24
 * \param width in px
 * \param height in px
 * \return Nx2 matrix with frame interval numerator, denominator
 * \sa enum_framesizes
 */
Matrix v4l2_handler::enum_frameintervals(__u32 pixel_format, __u32 width, __u32 height)
{
  Matrix ret;
  struct v4l2_frmivalenum frmival;
  CLEAR(frmival);
  frmival.pixel_format = pixel_format;
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
Matrix v4l2_handler::g_parm()
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
          error("timeperframe is not supported");
        }
    }
  return ret;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-g-parm.html
 */
void v4l2_handler::s_parm(Matrix timeperframe)
{
  struct v4l2_streamparm sparam;
  CLEAR(sparam);
  sparam.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  sparam.parm.capture.timeperframe.numerator = timeperframe(0);
  sparam.parm.capture.timeperframe.denominator = timeperframe(1);
  xioctl(fd, VIDIOC_S_PARM, &sparam);
  struct v4l2_fract *tf = &sparam.parm.capture.timeperframe;
  if (!tf->denominator || !tf->numerator)
    error("Invalid framerate");
  if (tf->numerator != __u32(timeperframe(0)) || tf->denominator != __u32(timeperframe(1)))
    warning("driver is using %d/%d as timeperframe but %d/%d was requested",
      tf->numerator, tf->denominator, __u32(timeperframe(0)), __u32(timeperframe(1)));
}

// get octave_scalar_map from v4l2_queryctrl
octave_scalar_map v4l2_handler::get_osm (struct v4l2_queryctrl queryctrl)
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
octave_value v4l2_handler::queryctrl()
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
int v4l2_handler::g_ctrl(int id)
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
void v4l2_handler::s_ctrl(int id, int value)
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
 *
 * The used libv4l2 pixelformat is set to V4L2_PIX_FMT_RGB24, V4L2_FIELD_INTERLACED
 */
void v4l2_handler::s_fmt(__u32 xres, __u32 yres)
{
  if(streaming)
    {
      error("v4l2_handler::s_fmt you have to stop streaming first");
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
      fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB24;
      fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
      xioctl(fd, VIDIOC_S_FMT, &fmt);
      if (fmt.fmt.pix.pixelformat != V4L2_PIX_FMT_RGB24)
        {
          error("Libv4l didn't accept RGB24 format. Can't proceed.\n");
        }
      if (xres && yres && ((fmt.fmt.pix.width != xres) || (fmt.fmt.pix.height != yres)))
        warning("driver is sending image at %dx%d but %dx%d was requested",
                fmt.fmt.pix.width, fmt.fmt.pix.height, xres, yres);
    }
}

Matrix v4l2_handler::g_fmt()
{
  struct v4l2_format fmt;
  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_G_FMT, &fmt);
  Matrix ret(1,2);
  ret(0) = fmt.fmt.pix.width;
  ret(1) = fmt.fmt.pix.height;
  return ret;
}

/*!
 * http://www.linuxtv.org/downloads/v4l-dvb-apis/vidioc-reqbufs.html
 * \param n number of buffers to initiate. A count value of zero frees all buffers.
 */
void v4l2_handler::reqbufs (unsigned int n)
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
        error("init_buffers VIDIOC_REQBUFS: running out of free memory\n");
      n_buffer = req.count;
    }
}

void v4l2_handler::mmap ()
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
          error("init_buffers mmap failed %s", strerror(errno));
        }
    }
}

void v4l2_handler::qbuf ()
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
 *
 * \return image, sequence, timestamp, [timecode]
 */
octave_value_list v4l2_handler::capture (int nargout, bool preview)
{
  octave_value_list ret;

  if(!streaming)
    {
      error("v4l2_handler::capture streaming wasn't enabled");
      return octave_value();
    }
  struct v4l2_format fmt;
  CLEAR(fmt);
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  xioctl(fd, VIDIOC_G_FMT, &fmt);
    
  dim_vector dv (3, fmt.fmt.pix.width, fmt.fmt.pix.height);
  uint8NDArray img (dv);

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
      error("v4l2_handler::capture select failed.");
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

  // check buffer sizes
  if(img.numel() != int(buf.bytesused))
    error("v4l2_handler::capture size mismatch, please file a bug report");

  unsigned char* p=reinterpret_cast<unsigned char*>(img.fortran_vec());
  memcpy(p, buffers[buf.index].start, buf.bytesused);
  if (nargout > 0)
    {
      // permute RGB24 byte order to octave RGB image format
      Matrix per(3,1);
      per(0) = 2;
      per(1) = 1;
      per(2) = 0;
      ret(0) = octave_value(img.permute(per));
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
          octave_stdout << "this buffer has a frame timecode" << endl;
          octave_scalar_map timecode;
          timecode.assign ("type", int(buf.timecode.type));
          timecode.assign ("flags", int(buf.timecode.flags));
          timecode.assign ("frames", int(buf.timecode.frames));
          timecode.assign ("seconds", int(buf.timecode.seconds));
          timecode.assign ("minutes", int(buf.timecode.minutes));
          timecode.assign ("hours", int(buf.timecode.hours));
          ret(3) = octave_value(timecode);
          return ret;
        }
      else
        {
          error("v4l2_handler::capture timecode not available");
        }
    }

  xioctl(fd, VIDIOC_QBUF, &buf);

  // show preview window?
  if (preview)
    {
      if (!preview_window)
        {
          preview_window = new img_win(10, 10, fmt.fmt.pix.width, fmt.fmt.pix.height);
          preview_window->show();
        }
      if (preview_window)
        {
          if(!preview_window->shown())
            preview_window->show();
          preview_window->copy_img(p, fmt.fmt.pix.width, fmt.fmt.pix.height, 1); //until now only RGB is supported
          preview_window->custom_label(dev.c_str(), buf.sequence, 1.0/dt);
        }
    }
  else if (preview_window)
    {
      delete preview_window;
      preview_window = 0;
    }
  return ret;
}

/*!
 * Main purpose is for debugging this class
 */
void v4l2_handler::capture_to_ppm(const char *fn)
{
  uint8NDArray img = capture(1, 0)(0).uint8_array_value();
  Matrix per(3,1);
  per(0) = 2;
  per(1) = 1;
  per(2) = 0;
  img = img.permute(per);  
  
  unsigned char* p=reinterpret_cast<unsigned char*>(img.fortran_vec());
  FILE *fout = fopen(fn, "w");
  if (!fout)
    {
      error("v4l2_handler::capture_to_ppm cannot open file %s", fn);
    }
  fprintf(fout, "P6\n%d %d 255\n",
          img.dim2(), img.dim3());
  fwrite(p, img.numel(), 1, fout);
  fclose(fout);
}

/*!
 * - Set pixelformat to V4L2_PIX_FMT_RGB24
 * - Set field to V4L2_FIELD_INTERLACED
 * - Requests buffers
 * - mmap the buffers
 * - enque the buffers
 * - start streaming
 */
void v4l2_handler::streamon(unsigned int n)
{
  if(streaming)
    {
      warning("v4l2_handler::streamon streaming already enabled. Buffer size unchanged.");
    }
  else
    {
      // set needed pixelformat and field
      s_fmt(0, 0);
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
void v4l2_handler::streamoff()
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

void v4l2_handler::munmap()
{
  if(buffers)
    {
      for (unsigned int i = 0; i < n_buffer; ++i)
        v4l2_munmap(buffers[i].start, buffers[i].length);
      free(buffers);
      buffers = 0;
    }
}

void v4l2_handler::close()
{
  streamoff();
  if (fd >= 0)
    v4l2_close(fd);
  fd = -1;
}

v4l2_handler* get_v4l2_handler_from_ov(octave_value ov)
{
  static bool type_loaded = false;
  if (!type_loaded)
    {
      v4l2_handler::register_type();
      type_loaded = true;
    }

  if (ov.type_id() != v4l2_handler::static_type_id())
    {
      error("get_v4l2_handler_from_ov: not a valid v4l2_handler");
      return 0;
    }

  v4l2_handler* imgh = 0;
  const octave_base_value& rep = ov.get_rep();
  imgh = &((v4l2_handler &)rep);
  return imgh;
}
