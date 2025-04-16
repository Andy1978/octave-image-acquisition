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

#include "config.h"
#ifdef HAVE_MFAPI_H
#include <cassert>
#include <dirent.h>
#include <algorithm> // for transform
#include "cl_mf_handler.h"

#define CHECK(hr) if (!SUCCEEDED(hr)) fprintf (stderr, "%s:%s:%i failed with %li\n", __FILE__, __FUNCTION__, __LINE__, GetLastError());

mf_handler::mf_handler ()
  : imaq_handler(),
    device (0),
    reader (0)
    /*streaming(0), preview_window(0)*/
{
  //octave_stdout << "mf_handler C'Tor, type_id() = " << type_id() << std::endl;

  HRESULT hr;

  hr = CoInitializeEx(0, COINIT_MULTITHREADED);
  CHECK(hr);

  hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
  CHECK(hr);

}

mf_handler::mf_handler (const mf_handler& m)
  : imaq_handler(),
    device (0),
    reader (0)
{
  octave_stdout << "mf_handler: the copy constructor shouldn't be called" << std::endl;
}

mf_handler::~mf_handler ()
{
  //octave_stdout << "mf_handler D'Tor " << endl;
  // stop streaming, unmap & free buffers, close device
  close();

  MFShutdown();
  CoUninitialize();
}

void
mf_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class mf_handler" << std::endl;
  if (reader)
  {
    octave_scalar_map fmt = g_fmt ();
    std::string fourcc = fmt.contents("fourcc").string_value ();
    uint32NDArray s = fmt.contents("size").uint32_array_value ();

    os << "  fourcc = " << fourcc << std::endl;
    os << "  size = " << s(0) << "x" << s(1) << std::endl;
  }
  else
    os << "  closed (reader == NULL)" << std::endl;
}

// https://learn.microsoft.com/en-us/archive/msdn-magazine/2016/september/c-unicode-encoding-conversions-with-stl-strings-and-win32-apis
std::string wchar_to_utf8 (LPWSTR val)
{
  int len = WideCharToMultiByte(CP_UTF8, 0, val, -1, 0, 0, 0, 0);
  //printf ("DEBUG: len = %i\n", len);
  // FIXME/TODO: check if the conversion can be made directly into std::string
  char buf[len];
  WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, len, 0, 0);
  //printf ("DEBUG: wchar_to_utf8 returns '%s'\n", buf);
  return buf;
}

std::string GetAllocatedString (IMFActivate* device, REFGUID guidKey)
{
  UINT32 length;
  LPWSTR val;
  HRESULT hr = device->GetAllocatedString(guidKey, &val, &length);
  CHECK(hr);
  std::string ret = wchar_to_utf8 (val);
  CoTaskMemFree(val);
  return ret;
}

std::wstring utf8_to_wstring (const std::string &in)
{
  int len = MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, 0, 0);
  if (len == 0)
    {
      fprintf (stderr, "ERROR: in utf8_to_wchar, MultiByteToWideChar returned len = %i\n", len);
      return std::wstring();
    }
  else
    {
      std::wstring wide_str (len, 0);
      MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, &wide_str[0], len);

      //wcout << L"DEBUG: Converted wide string: " << wide_str << std::endl;
      return wide_str;
    }
}

octave_map
mf_handler::enum_devices ()
{
  octave_map retval;

  HRESULT hr;

  // Wie viele Geräte gibt es?
  UINT32 count;
  IMFActivate** devices;
  {
    IMFAttributes* attr;

    hr = MFCreateAttributes(&attr, 1);
    CHECK(hr);

    attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    CHECK(hr);

    hr = MFEnumDeviceSources(attr, &devices, &count);
    CHECK(hr);

    attr->Release();
  }
  //printf("Detected %u devices...\n", count);

  // Über alle erkannten devices loopen
  for (UINT32 i = 0; i < count; i++)
    {
      octave_scalar_map dev;
      dev.assign ("name", GetAllocatedString (devices[i], MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME));
      dev.assign ("symlink", GetAllocatedString (devices[i], MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK));

      //uint32_t tmp;
      //hr = devices[i]->GetUINT32 (MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_MAX_BUFFERS, &tmp);
      //CHECK (hr);
      //dev.assign ("max_buffers", tmp);

      retval.assign(octave_idx_type(i), dev);
      devices[i]->Release();
    }
  CoTaskMemFree(devices);

  return retval;
}

octave_scalar_map
mf_handler::open (std::string d, bool quiet)
{
  HRESULT hr;
  octave_scalar_map ret;

  //octave_stdout << "DEBUG: mf_handler::open (d = '" << d << "') called" << std::endl;

  // ToDo: Muss man schauen, welche ID unter windoze Sinn macht. Vorerst symlink weil eindeutig
  std::wstring symlink = utf8_to_wstring (d);

  // create device from symlink
  {
    IMFAttributes* attr;

    hr = MFCreateAttributes(&attr, 2);
    CHECK(hr);

    hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    CHECK(hr);

    // Bis hier fast identisch zu enum_devices, jetzt wird weiter eingeschränkt über symlink
    hr = attr->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, symlink.c_str());
    CHECK(hr);

    // bei enum_devices MFEnumDeviceSources...
    hr = MFCreateDeviceSource(attr, &device);
    CHECK(hr);

    // man könnte überlegen den Code hier mit dem aus enum_devices zusammenzulegen und
    // den Filter nach symlink optional...

    attr->Release();
  }

  // create reader
  hr = MFCreateSourceReaderFromMediaSource(device, NULL, &reader);
  CHECK(hr);

  //enum_formats ();
  //s_fmt ("MJPG", 640, 480);

  return ret;
}

octave_value
mf_handler::querycap ()
{
  // ToDo: Is it possible to get some infos from the media foundation device?
  octave_scalar_map st;
  st.assign ("driver",       "uvcvideo");
  st.assign ("card",         "not implemented");
  st.assign ("bus_info",     "not implemented");
  st.assign ("version",      "not implemented");
  st.assign ("capabilities", "not implemented");
  return octave_value (st);
}

octave_value
mf_handler::enum_inputs ()
{
  //octave_stdout << "warning: mf_handler::enum_inputs isn't implemented yet, it always returns one dummy entry" << std::endl;

  octave_map ret;
  octave_scalar_map st;
  st.assign ("name", "dummy input");
  st.assign ("type", "not yet implemented");
  st.assign ("audioset",     (unsigned int) 0);
  st.assign ("tuner",        (unsigned int) 0);
  st.assign ("std",          (unsigned int) 0);
  st.assign ("status",       (unsigned int) 0);
  st.assign ("capabilities", (unsigned int) 0);

  ret.assign(0, st);
  return octave_value(ret);
}

int
mf_handler::get_input ()
{
  //octave_stdout << "warning: mf_handler::get_input isn't implemented yet, it always returns 0" << std::endl;
  return 0;
}

void
mf_handler::set_input (int index)
{
  //octave_stdout << "warning: mf_handler::set_input isn't implemented yet, a function call has no effect" << std::endl;
}

octave_map
mf_handler::loop_native_media_types ()
{
  octave_map ret;
  // enumerating output formats from
  // https://learn.microsoft.com/en-us/windows/win32/medfound/processing-media-data-with-the-source-reader
  // https://stackoverflow.com/questions/7968547/how-to-get-a-list-of-all-microsoft-media-foundation-transforms-mfts-available

  HRESULT hr;
  IMFMediaType *pType = NULL;
  DWORD dwMediaTypeIndex = 0;
  while (SUCCEEDED(hr = reader->GetNativeMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, dwMediaTypeIndex, &pType)))
    {
      //printf ("dwMediaTypeIndex = %i\n", dwMediaTypeIndex);
      ret.assign(octave_idx_type(dwMediaTypeIndex), g_fmt (pType));
      pType->Release();
      ++dwMediaTypeIndex;
    }
  return ret;
}

octave_value
mf_handler::enum_formats ()
{
  Cell tmp = loop_native_media_types().contents("fourcc");
  string_vector sv = tmp.string_vector_value ().sort (true);

  // make struct array for v4l2 compatibility
  octave_map ret;
  for (int k = 0; k < sv.numel (); ++k)
    {
      octave_scalar_map sm;
      sm.assign ("fourcc", sv(k));
      ret.assign(octave_idx_type(k), sm);
    }
  return octave_value(ret);
}

Matrix
mf_handler::enum_framesizes (std::string pixelformat)
{
  octave_map tmp = loop_native_media_types();
  dim_vector dv (tmp.numel (), 2);
  Matrix ret(dv);

  int n = 0;
  for (int k = 0; k < tmp.numel (); ++k)
  {
    if (tmp.contents("fourcc")(k).string_value() == pixelformat)
    {
      Matrix frame_size = tmp.contents("size")(k).matrix_value();
      //printf ("%i %f %f\n", k, frame_size(0), frame_size(1));
      ret(n, 0) = frame_size(0);
      ret(n++, 1) = frame_size(1);
    }
  }

  ret.resize(n, 2);

  Array<octave_idx_type> ridx = ret.sort_rows_idx (ASCENDING);
  ret = ret.index (ridx, octave::idx_vector::colon);

  // unique
  for (int k = ret.rows() - 1; k > 0; k--)
  {
    if (ret(k, 0) == ret (k-1, 0) && ret(k, 1) == ret (k-1, 1))
      ret.delete_elements(0, k);
  }
  return ret;
}

Matrix
mf_handler::enum_frameintervals (std::string pixelformat, uint32_t width, uint32_t height)
{
  octave_map tmp = loop_native_media_types();
  dim_vector dv (tmp.numel (), 2);
  Matrix ret(dv);
  int n = 0;
  for (int k = 0; k < tmp.numel (); ++k)
  {
    if (tmp.contents("fourcc")(k).string_value() == pixelformat)
    {
       Matrix frame_size = tmp.contents("size")(k).matrix_value();
       //printf ("%i %f %f\n", k, frame_size(0), frame_size(1));
       if (frame_size (0) == width && frame_size(1) == height)
       {
         Matrix frame_interval = tmp.contents("frame_rate")(k).matrix_value();
         //printf ("%i %f %f\n", k, frame_interval(0), frame_interval(1));
         ret(n, 0) = frame_interval(0);
         ret(n++, 1) = frame_interval(1);
       }
    }
  }
  ret.resize(n, 2);

  Array<octave_idx_type> ridx = ret.sort_rows_idx (ASCENDING);
  ret = ret.index (ridx, octave::idx_vector::colon);

  // unique
  for (int k = ret.rows() - 1; k > 0; k--)
  {
    if (ret(k, 0) == ret (k-1, 0) && ret(k, 1) == ret (k-1, 1))
      ret.delete_elements(0, k);
  }
  return ret;
}

Matrix
mf_handler::get_frameinterval ()
{
  octave_scalar_map tmp = g_fmt ();
  return tmp.contents("frame_rate").matrix_value ();
}

// TODO/FIXME: If the set frame_rate is not within MF_MT_FRAME_RATE_RANGE_MIN and MF_MT_FRAME_RATE_RANGE_MAX,
// it's silently ignored. (get_frameinterval still returns the set frame_rate)
void
mf_handler::set_frameinterval (Matrix timeperframe)
{
  IMFMediaType* type;

  HRESULT hr = MFCreateMediaType(&type);
  CHECK(hr);

  hr = reader->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type);
  CHECK(hr);

  UINT32 unDenominator = timeperframe(0);
  UINT32 unNumerator = timeperframe(1);
  hr = MFSetAttributeRatio(type, MF_MT_FRAME_RATE, unNumerator, unDenominator);
  CHECK (hr);

  hr = reader->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
  CHECK(hr);

  if (! SUCCEEDED (hr))
    error ("mf_handler::set_frameinterval (%u, %u) SetCurrentMediaType failed", unNumerator, unDenominator);

  type->Release ();
}

static GUID
GetMediaTypeGUIDFromFourCC (const std::string& fourCC)
{
  if (fourCC.length () != 4)
    error ("FourCC must have exactly 4 characters.");

  // corresponds to:
  // #define FCC(ch4) ((((DWORD) (ch4) &0xff) << 24) | (((DWORD) (ch4) &0xff00) << 8) | (((DWORD) (ch4) &0xff0000) >> 8) | (((DWORD) (ch4) &0xff000000) >> 24))
  DWORD fourCCValue = (DWORD)fourCC[0] |
                      ((DWORD)fourCC[1] << 8) |
                      ((DWORD)fourCC[2] << 16) |
                      ((DWORD)fourCC[3] << 24);

  // corresponds to:
  // #define DEFINE_GUID(name,l,w1,w2,b1,b2,b3,b4,b5,b6,b7,b8) EXTERN_C const GUID DECLSPEC_SELECTANY name = { l, w1, w2, { b1, b2, b3, b4, b5, b6, b7, b8 } }

  // #define DEFINE_MEDIATYPE_GUID(name, format) DEFINE_GUID (name, format, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);
  return {fourCCValue, 0x0000, 0x0010,
          {0x80, 0x00, 0x00, 0xAA, 0x00, 0x38, 0x9B, 0x71}};
}

void
mf_handler::s_fmt (std::string fmtstr, uint32_t xres, uint32_t yres)
{
  //cout << "mf_handler::s_fmt (" << fmtstr << ", " << xres << ", " << yres << ") called" << endl;

  // typically webcams provide YUV2 format, you'll need to convert it to
  // RGB yourself or with help of IMFTransform
  // you can enumerate all supported types with IMFSourceReader_GetNativeMediaType

// if (streaming)
//   {
//     error("mf_handler::s_fmt: you have to stop streaming first");
//   }
// else
  {
    {
      IMFMediaType* type;

      HRESULT hr = MFCreateMediaType(&type);
      CHECK(hr);

      hr = type->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
      CHECK(hr);

      // mapping to different FourCC
      if (fmtstr == "YUYV")
        hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_YUY2);
      else if (fmtstr == "AV01")
        hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_AV1);
      else if (fmtstr == "HEVS")
        hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_HEVC_ES);
      else if (fmtstr == "theo")
        hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_Theora);
      else if (fmtstr == "dvc ")
        hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_DVC);
      else
        hr = type->SetGUID (MF_MT_SUBTYPE, GetMediaTypeGUIDFromFourCC (fmtstr));

      CHECK(hr);

      if (xres && yres)
      {
        // set frame size
        UINT64 tmp = ((UINT64)xres << 32) | yres;
        hr = type->SetUINT64(MF_MT_FRAME_SIZE, tmp);
        CHECK(hr);
      }

      // finally call SetCurrentMediaType
      hr = reader->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
      CHECK(hr);

      if (! SUCCEEDED (hr))
        error ("mf_handler::s_fmt (%s, %u, %u) SetCurrentMediaType failed", fmtstr.c_str(), xres, yres);

      type->Release ();
    }

    // ToDo: hier wie bei v4l2 warning, wenn der Treiber das Format geändert hat?
    g_fmt ();
  }
}

octave_scalar_map mf_handler::g_fmt (IMFMediaType *pType)
{
  octave_scalar_map ret;
  HRESULT hr;

  // get frame size
  {
    UINT64 tmp;
    hr = pType->GetUINT64(MF_MT_FRAME_SIZE, &tmp);
    CHECK(hr);

    UINT32 width = (UINT32)(tmp >> 32);
    UINT32 height = (UINT32)(tmp);
    //printf("DEBUG: Readback Size = %ux%u\n", width, height);

    Matrix s(1,2);
    s(0) = width;
    s(1) = height;
    ret.assign ("size", s);
  }

  // get frame rate range
  {
    UINT32 unNumerator_min = 0;
    UINT32 unDenominator_min = 0;
    hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE_RANGE_MIN, &unNumerator_min, &unDenominator_min);
    CHECK (hr);
    //printf ("MF_MT_FRAME_RATE_RANGE_MIN = %i/%i\n", unNumerator_min, unDenominator_min);

    UINT32 unNumerator_max = 0;
    UINT32 unDenominator_max = 0;
    hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE_RANGE_MAX, &unNumerator_max, &unDenominator_max);
    CHECK (hr);
    //printf ("MF_MT_FRAME_RATE_RANGE_MAX = %i/%i\n", unNumerator_max, unDenominator_max);

    UINT32 unNumerator = 0;
    UINT32 unDenominator = 0;
    hr = MFGetAttributeRatio(pType, MF_MT_FRAME_RATE, &unNumerator, &unDenominator);
    CHECK (hr);
    //printf ("MF_MT_FRAME_RATE = %i/%i\n", unNumerator, unDenominator);

    // TODO: all of my tests with uvcvideo returned equal values for _MIN and _MAX
    // but can we be sure?
    assert (unNumerator_min == unNumerator_max);
    assert (unDenominator_min == unDenominator_max);
    assert (unNumerator_min == unNumerator);
    assert (unDenominator_min == unDenominator);

    // Some items return odd ratios, for example uvcvideo C270
    // MF_MT_SUBTYPE_CLSID = {32595559-0000-0010-8000-00AA00389B71}
    // YUY2 1280x720
    // unNumerator = 10000000, unDenominator = 1333333
    // where v4l2 return 15/2 as expected...

    // FIXME: if all three are equal, there is no need to return them all
    //Matrix fr_min (1, 2);
    //fr_min(0) = unDenominator_min;
    //fr_min(1) = unNumerator_min;
    //ret.assign ("frame_rate_min", fr_min);

    //Matrix fr_max (1, 2);
    //fr_max(0) = unDenominator_max;
    //fr_max(1) = unNumerator_max;
    //ret.assign ("frame_rate_max", fr_max);

    Matrix fr (1, 2);
    fr(0) = unDenominator;
    fr(1) = unNumerator;
    ret.assign ("frame_rate", fr);
  }

  // get FOURCC

  // https://learn.microsoft.com/en-us/windows/win32/medfound/video-subtype-guids
  // https://learn.microsoft.com/en-us/windows/win32/api/guiddef/ns-guiddef-guid
  // Data1: first 8 hexadecimal digits
  // Data2: first group of 4 hexadecimal digits
  // Data3: second group of 4 hexadecimal digits
  // Data4: Array of 8 bytes.
  //        The first 2 bytes contain the third group of 4 hexadecimal digits.
  //        The remaining 6 bytes contain the final 12 hexadecimal digits.
  GUID sub;

  hr = pType->GetGUID (MF_MT_SUBTYPE, &sub);
  CHECK(hr);

  // First 4 bytes are FOURCC
  char tmp_fourcc[5];
  tmp_fourcc[0] = int8_t (sub.Data1 & 0xff);
  tmp_fourcc[1] = int8_t ((sub.Data1 & 0xff00) >> 8);
  tmp_fourcc[2] = int8_t ((sub.Data1 & 0xff0000) >> 16);
  tmp_fourcc[3] = int8_t ((sub.Data1 & 0xff000000) >> 24);
  tmp_fourcc[4] = 0;

  ret.assign ("fourcc", std::string(tmp_fourcc));

  // get CLSID as string (informational purpose only)
  OLECHAR* guidString;
  StringFromCLSID(sub, &guidString);
  //printf ("DEBUG: guidString = '%S'\n", guidString);
  ret.assign ("MF_MT_SUBTYPE_CLSID", wchar_to_utf8 (guidString));
  ::CoTaskMemFree(guidString);

  /*
  // Die Konstanten sind wohl nur die FOURCC
  #define out(x) printf (#x" %lx\n", x.Data1)
  out (MFVideoFormat_MJPG);
  out (MFVideoFormat_YUY2);
  out (MFVideoFormat_NV12);
  out (MFVideoFormat_AI44);
  out (MFVideoFormat_AYUV);
  out (MFVideoFormat_I420);
  out (MFVideoFormat_IYUV);
  out (MFVideoFormat_NV11);
  out (MFVideoFormat_NV21);
  out (MFVideoFormat_UYVY);
  out (MFVideoFormat_Y41P);
  out (MFVideoFormat_Y41T);
  out (MFVideoFormat_Y42T);
  out (MFVideoFormat_YVU9);
  out (MFVideoFormat_YV12);
  out (MFVideoFormat_YVYU);
  */

  {
    int fCompressed = 0;
    hr = pType->IsCompressedFormat (&fCompressed);
    CHECK(hr)
    ret.assign ("flags_compressed", fCompressed);  // TODO/FIXME: not tested yet

    // GetNativeMediaType should only return native formats
    ret.assign ("flags_emulated", false);
  }

  return ret;
}

octave_scalar_map mf_handler::g_fmt ()
{
  octave_scalar_map ret;
  IMFMediaType* pType;

  HRESULT hr = reader->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType);
  CHECK(hr);

  ret = g_fmt (pType);

  pType->Release ();
  return ret;
}


octave_scalar_map get_ctrl_range (IMFMediaSource* device, long src_obj, long prop)
{
  octave_scalar_map ctrl;

  long min, max, step, def, control;
  long current_value = 0;
  long flags = 0;

  HRESULT hr = 0;
  HRESULT hr2 = 0;
  if (src_obj == 0)
    {
      IAMCameraControlPtr spCameraControl(device);
      if(spCameraControl)
        {
          hr = spCameraControl->GetRange(prop, &min, &max, &step, &def, &control);
          hr2 = spCameraControl->Get(prop, &current_value, &flags);
        }
    }
  else if (src_obj == 1)
    {
      IAMVideoProcAmpPtr spVideo(device);
      if(spVideo)
        {
          hr = spVideo->GetRange(prop, &min, &max, &step, &def, &control);
          hr2 = spVideo->Get(prop, &current_value, &flags);
        }
    }

  if(SUCCEEDED(hr))
    {
      ctrl.assign ("id", int((src_obj << 16) + prop)); // von Andy erdacht. Möglicherweise wäre die UUID hier sinnvoller
      ctrl.assign ("min", min);
      ctrl.assign ("max", max);
      ctrl.assign ("step", step);
      ctrl.assign ("default", def);
      ctrl.assign ("control", control);
    }

  if(SUCCEEDED(hr2))
    {
      ctrl.assign ("value", current_value);
      ctrl.assign ("flags", flags);
    }

  return ctrl;
}

octave_value mf_handler::queryctrl ()
{
  octave_scalar_map ctrls;

  // It looks like media foundation has a fixed num of properties, see
  // https://learn.microsoft.com/de-de/windows/win32/api/strmif/ne-strmif-cameracontrolproperty
  // or https://learn.microsoft.com/de-de/windows/win32/api/strmif/ne-strmif-videoprocampproperty
  // as opposed to v4l2 which supports dynamic controls.

  // Die id würde ich zusammenstückeln aus der property und einer Konstante oder der UUID?
  // Ich denke ich sollte hier die interessanten controls aus
  // IAMCameraControl und IAMVideoProcAmp händisch reinpacken, wenn es sie gibt

  ctrls.assign("pan",   get_ctrl_range (device, 0, CameraControl_Pan));
  ctrls.assign("tilt",   get_ctrl_range (device, 0, CameraControl_Tilt));
  ctrls.assign("roll",   get_ctrl_range (device, 0, CameraControl_Roll));
  ctrls.assign("zoom",   get_ctrl_range (device, 0, CameraControl_Zoom));
  ctrls.assign("exposure",   get_ctrl_range (device, 0, CameraControl_Exposure));
  ctrls.assign("iris",   get_ctrl_range (device, 0, CameraControl_Iris));
  ctrls.assign("focus",      get_ctrl_range (device, 0, CameraControl_Focus));

  ctrls.assign("brightness", get_ctrl_range (device, 1, VideoProcAmp_Brightness));
  ctrls.assign("contrast",   get_ctrl_range (device, 1, VideoProcAmp_Contrast));
  ctrls.assign("hue",        get_ctrl_range (device, 1, VideoProcAmp_Hue));
  ctrls.assign("saturation", get_ctrl_range (device, 1, VideoProcAmp_Saturation));
  ctrls.assign("sharpness",  get_ctrl_range (device, 1, VideoProcAmp_Sharpness));
  ctrls.assign("gamma",        get_ctrl_range (device, 1, VideoProcAmp_Gamma));
  ctrls.assign("colorenable",  get_ctrl_range (device, 1, VideoProcAmp_ColorEnable));
  ctrls.assign("white_balance", get_ctrl_range (device, 1, VideoProcAmp_WhiteBalance));
  ctrls.assign("backlightcompensation",  get_ctrl_range (device, 1, VideoProcAmp_BacklightCompensation)); // power_line_frequency in v4l2
  ctrls.assign("gain",  get_ctrl_range (device, 1, VideoProcAmp_Gain));

  // FIXME: currently the not available controls are also returned
  // What yould be the best solution? Remove the empty ones?

  return ctrls;
}

int mf_handler::g_ctrl (int id)
{
  int src_obj = id >> 16;
  long prop = id & 0xFFFF;

  //printf ("DEBUG: mf_handler::g_ctrl (%i), src_obj = %i, prop = %li\n", id, src_obj, prop);

  octave_scalar_map tmp = get_ctrl_range (device, src_obj, prop);
  return tmp.contents ("value").int_value();
}

void mf_handler::s_ctrl (int id, octave_value val)
{
  // This uses the old dshow API (looks like there is nothin in native media foundation)
  // See also https://github.com/opencv/opencv/blob/14396b802947d69d3cc44f0e809977b891ef8f4a/modules/videoio/src/cap_dshow.cpp#L1948

  // Attention: after switching back to CameraControl_Flags_Auto
  // you need to capture some frames to let the param settle (for example auto gain)
  // See ./devel/check_controls.m

  HRESULT hr = 0;
  int src_obj = id >> 16;
  long prop = id & 0xFFFF;

  //printf ("DEBUG: val.isempty () = %i\n", val.isempty());

  if (src_obj == 0)
    {
      IAMCameraControlPtr spCameraControl(device);
      if(spCameraControl)
      {
        if (val.isempty())
          hr = spCameraControl->Set(prop, 0, CameraControl_Flags_Auto);
        else
          hr = spCameraControl->Set(prop, val.int_value(), CameraControl_Flags_Manual);
      }
    }
  else if (src_obj == 1)
    {
      IAMVideoProcAmpPtr spVideo(device);
      if(spVideo)
      {
        if (val.isempty())
          hr = spVideo->Set(prop, 0, VideoProcAmp_Flags_Auto);
        else
          hr = spVideo->Set(prop, val.int_value(), VideoProcAmp_Flags_Manual);
      }
    }
  CHECK(hr)
}

octave_value_list mf_handler::capture (int nargout, bool preview, bool raw_output)
{
  octave_value_list ret;
  HRESULT hr;

  DWORD stream;
  DWORD flags;
  LONGLONG timestamp;
  IMFSample* sample;

  //printf ("DEBUG  mf_handler::capture (%i, %i, %i)\n", nargout, preview, raw_output);

  for (;;)
    {
      // this is reading in syncronous blocking mode, MF supports also async calls
      hr = reader->ReadSample (MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &stream, &flags, &timestamp, &sample);
      CHECK(hr);
      if (! SUCCEEDED (hr))
        error ("mf_handler::capture IMFSourceReader->ReadSample failed");

      if (flags & MF_SOURCE_READERF_STREAMTICK)
        {
          //printf ("DEBUG: mf_handler::capture: waiting for sample...\n");
          continue;
        }

      //printf ("timestamp = %llu\n", timestamp);
      break;
    }

  // näher anschauen: IMFTransform
  // https://learn.microsoft.com/en-us/windows/win32/medfound/processing-data-in-the-encoder
  // nach 2 Tagen komme ich zu dem Schluss, das MJPG -> RGB24 mit den mitgelieferten MFTs gar nicht möglich ist
  {
    IMFMediaBuffer* buffer;

    hr = sample->ConvertToContiguousBuffer (&buffer);
    CHECK(hr);

    BYTE* data;
    DWORD size;

    hr = buffer->Lock (&data, NULL, &size);
    CHECK(hr);

    //printf ("DEBUG: mf_handler::capture: buffer size = %lu\n", size);
    //for (int k = 0; k < 10; ++k)
    //  printf ("%x ", data[k]);
    //printf ("\n");

    // get current formats
    octave_scalar_map fmt_map = g_fmt ();
    std::string fmt = fmt_map.contents("fourcc").string_value ();
    //printf ("DEBUG: mf_handler::capture: fmt = '%s'\n", fmt.c_str());

    uint32NDArray s = fmt_map.contents("size").uint32_array_value ();
    UINT32 width = s(0);
    UINT32 height = s(1);
    //printf ("DEBUG: mf_handler::capture: size = [%i %i]\n", width, height);

    // basic, underlying color space
    bool is_mjpg  = false;
    bool is_ycbcr = false;

    if (fmt == "YUY2")
      // YUYV aka YUV 4:2:2 aka YUY2
      // return struct with fields Y, Cb, Cr
      {
        ret(0) = imaq_handler::get_YUYV (data, size, width, height);
        is_ycbcr = true;
      }
    else if (fmt == "NV12")
      {
        ret(0) = imaq_handler::get_NV12 (data, size, width, height);
        is_ycbcr = true;
      }
    else
      {
        is_mjpg = (fmt == "MJPG");
        // return buffer verbatim
        dim_vector dv (size, 1);
        uint8NDArray img (dv);
        unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
        memcpy(p, data, size);
        ret(0) = octave_value(img);
      }

    buffer->Unlock ();
    buffer->Release ();

    // Fake other return values, somme not yet implemented
    // TODO/FIXME: try to implement timecode

    static int sequence_nr = 0;
    if (nargout > 1) // sequence
      ret(1) = octave_value(sequence_nr++);

    if (nargout > 2) // timestamp
      {
        // pllTimestamp
        // Receives the time stamp of the sample, or the time of the stream event indicated in pdwStreamFlags.
        // The time is given in 100-nanosecond units.
        ret(2) = timestamp / 1.0e7;
      }

    if (nargout > 3) // timecode
      {
        octave_scalar_map timecode;
        timecode.assign ("type", 0);
        timecode.assign ("flags", 0);
        timecode.assign ("frames", 0);
        timecode.assign ("seconds", 0);
        timecode.assign ("minutes", 0);
        timecode.assign ("hours", 0);
        ret(3) = octave_value(timecode);
      }

    // Do we need an rgb image?
    if (! raw_output || preview)
    {
      octave_value rgb_img;
      //printf ("DEBUG: we need to convert %s to RGB3...\n", fmt.c_str());

      if (is_ycbcr)
        rgb_img = YCbCr_to_RGB (ret(0), ITU_standard);
      else if (is_mjpg)
        rgb_img = JPG_to_RGB (ret(0));
      else
        error ("mf_handler::capture: no conversion from '%s' to RGB3 implemented yet", fmt.c_str());

      if (preview)
        {
          if (!preview_window)
            {
              //printf ("mf_handler::capture: create an new preview_window\n");
              preview_window = new img_win(10, 10, width, height);
              preview_window->show();
            }
          if (preview_window)
            {
              if(preview == 1 && !preview_window->shown())
                preview_window->show();

              uint8NDArray tmp = rgb_img.uint8_array_value();
              Array<octave_idx_type> perm (dim_vector (3, 1));
              perm(0) = 2;
              perm(1) = 1;
              perm(2) = 0;
              tmp = tmp.permute (perm);
              unsigned char *p = reinterpret_cast<unsigned char*>(tmp.fortran_vec());

              preview_window->copy_img(p, width, height, 1);

              // FIXME: dt könnte man auch über QueryPerformanceCounter auf windoze machen
              //preview_window->custom_label(dev.c_str(), sequence_nr, 1.0/dt);
            }
        }
      else if (preview_window)
        {
          delete preview_window;
          preview_window = 0;
        }

      if (! raw_output)
        ret(0) = rgb_img;
    }

  }
  sample->Release ();
  return ret;
}

void
mf_handler::close ()
{
  //octave_stdout << "DEBUG: mf_handler::close called" << std::endl;

  //~ streamoff();
  if (device)
    {
      device->Release();
      device = 0;
    }

  if (reader)
    {
      reader->Release ();
      reader = 0;
    }
}

#endif
