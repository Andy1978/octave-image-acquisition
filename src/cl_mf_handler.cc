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

#include "config.h"
#ifdef HAVE_MFAPI_H
#include <cassert>
#include <dirent.h>
//#include <sys/types.h>
#include "cl_mf_handler.h"


#define CHECK(hr) if (!SUCCEEDED(hr)) fprintf (stderr, "failed with %li\n", GetLastError());

mf_handler::mf_handler ()
  : imaq_handler(),
    device (0),
    reader (0)


	/*,
    fd(-1), n_buffer(0), buffers(0), streaming(0),
    preview_window(0), _is_video_capture (0), _is_meta_capture(0)*/
{
  //octave_stdout << "mf_handler C'Tor" << endl;
  octave_stdout << "mf_handler C'Tor, type_id() = " << type_id() << std::endl;

  HRESULT hr;

  hr = CoInitializeEx(0, COINIT_MULTITHREADED);
  CHECK(hr);

  hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
  CHECK(hr);

}

mf_handler::mf_handler (const mf_handler& m)
  : imaq_handler()
{
  octave_stdout << "mf_handler: the copy constructor shouldn't be called" << std::endl;
}

mf_handler::~mf_handler ()
{
  octave_stdout << "mf_handler D'Tor " << endl;

  // delete preview_window if active
  if (preview_window)
    {
      delete preview_window;
      preview_window = 0;
    }

  // stop streaming, unmap & free buffers, close device
  close();

  MFShutdown();
  CoUninitialize();
}

void
mf_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class mf_handler" << endl;
  //os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}

// https://learn.microsoft.com/en-us/archive/msdn-magazine/2016/september/c-unicode-encoding-conversions-with-stl-strings-and-win32-apis
string wchar_to_utf8 (LPWSTR val)
{
	int len = WideCharToMultiByte(CP_UTF8, 0, val, -1, 0, 0, 0, 0);
	//printf ("DEBUG: len = %i\n", len);
	// FIXME/TODO: check if the conversion can be made directly into std::string
	char buf[len];
	WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, len, 0, 0);
	//printf ("DEBUG: wchar_to_utf8 returns '%s'\n", buf);
	return buf;
}

string GetAllocatedString (IMFActivate* device, REFGUID guidKey)
{
	UINT32 length;
	LPWSTR val;
	HRESULT hr = device->GetAllocatedString(guidKey, &val, &length);
	CHECK(hr);
	string ret = wchar_to_utf8 (val);
  CoTaskMemFree(val);
	return ret;
}

wstring utf8_to_wstring (const string &in)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, 0, 0);
	if (len == 0)
	{
	  fprintf (stderr, "ERROR: in utf8_to_wchar, MultiByteToWideChar returned len = %i\n", len);
		return wstring();
	}
	else
	{
    wstring wide_str (len, 0);
    MultiByteToWideChar(CP_UTF8, 0, in.c_str(), -1, &wide_str[0], len);

		//wcout << L"DEBUG: Converted wide string: " << wide_str << endl;
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
mf_handler::open (string d, bool quiet)
{
  HRESULT hr;
  octave_scalar_map ret;

  octave_stdout << "mf_handler::open (d = '" << d << "') called" << std::endl;
  // ToDo: Muss man schauen, welche ID unter windoze Sinn macht. Vorerst symlink weil eindeutig
  wstring symlink = utf8_to_wstring (d);

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
  //device->Release();

  enum_formats ();

  s_fmt ("MJPG", 640, 480);

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
mf_handler::enum_formats ()
{
  octave_map ret;
  // enumerating output formats from
  // https://learn.microsoft.com/en-us/windows/win32/medfound/processing-media-data-with-the-source-reader

  DWORD dwStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
  DWORD dwMediaTypeIndex = 0;

  // https://stackoverflow.com/questions/7968547/how-to-get-a-list-of-all-microsoft-media-foundation-transforms-mfts-available
  HRESULT hr;
  IMFMediaType *pType = NULL;
  while (SUCCEEDED(hr = reader->GetNativeMediaType(dwStreamIndex, dwMediaTypeIndex, &pType)))
    {
			ret.assign(octave_idx_type(dwMediaTypeIndex), g_fmt (pType));
			pType->Release();
      ++dwMediaTypeIndex;
    }
  return octave_value(ret);
}

void
mf_handler::s_fmt (string fmtstr, __u32 xres, __u32 yres)
{
	cout << "mf_handler::s_fmt (" << fmtstr << ", " << xres << ", " << yres << ") called" << endl;

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


				// FIXME: schauen, wie man das geschickter machen kann
        #define MUX_FMT(X) (fmtstr == #X) hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_ ## X);
				if MUX_FMT(MJPG)
				else if MUX_FMT(YUY2)
				else if MUX_FMT(NV12)
				else if MUX_FMT(AI44)
				else if MUX_FMT(AYUV)
				else if MUX_FMT(I420)
				else if MUX_FMT(IYUV)
				else if MUX_FMT(NV11)
				else if MUX_FMT(NV21)
				else if MUX_FMT(UYVY)
				else if MUX_FMT(Y41P)
				else if MUX_FMT(Y42T)
				else if MUX_FMT(YVU9)
				else if MUX_FMT(YV12)
				else if MUX_FMT(YVYU)
				else
					error ("unknown type %s", fmtstr.c_str());

				CHECK(hr);

				// set frame size
				UINT64 tmp = ((UINT64)xres << 32) | yres;
				hr = type->SetUINT64(MF_MT_FRAME_SIZE, tmp);
				CHECK(hr);

				// finally call SetCurrentMediaType
				hr = reader->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
				CHECK(hr);

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

	UINT64 tmp;
	hr = pType->GetUINT64(MF_MT_FRAME_SIZE, &tmp);
	CHECK(hr);

	UINT32 width;
	UINT32 height;

	width = (UINT32)(tmp >> 32);
	height = (UINT32)(tmp);
	//printf("DEBUG: Readback Size = %ux%u\n", width, height);

  Matrix s(1,2);
  s(0) = width;
  s(1) = height;
  ret.assign ("size", s);

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

  ret.assign ("pixelformat", std::string(tmp_fourcc));

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

  return ret;
}

octave_scalar_map mf_handler::g_fmt ()
{
	octave_scalar_map ret;
	IMFMediaType* pType;

	HRESULT hr = reader->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, &pType);
	CHECK(hr);

  ret = current_fmt = g_fmt (pType);

	pType->Release ();
  return ret;
}


octave_scalar_map get_ctrl_range (IMFMediaSource* device, long src_obj, long prop)
{
  octave_scalar_map ctrl;

  long min, max, step, def, control;

  HRESULT hr = 0;
  if (src_obj == 0)
  {
     IAMCameraControlPtr spCameraControl(device);
     if(spCameraControl)
       hr = spCameraControl->GetRange(prop, &min, &max, &step, &def, &control);
  }
  else if (src_obj == 1)
  {
     IAMVideoProcAmpPtr spVideo(device);
     if(spVideo)
       hr = spVideo->GetRange(prop, &min, &max, &step, &def, &control);
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

octave_value_list mf_handler::capture (int nargout, int preview = 0)
{
	octave_value_list ret;
  HRESULT hr;

	DWORD stream;
	DWORD flags;
	LONGLONG timestamp;
	IMFSample* sample;

	//printf ("reader = %p\n", reader);

	for (;;)
		{
			// this is reading in syncronous blocking mode, MF supports also async calls
			hr = reader->ReadSample (MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, &stream, &flags, &timestamp, &sample);
			CHECK(hr);

			if (flags & MF_SOURCE_READERF_STREAMTICK)
				{
					continue;
				}

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

		printf ("buffer size = %lu\n", size);
		for (int k = 0; k < 10; ++k)
			printf ("%x ", data[k]);
		printf ("\n");

		// get current format
		string fmt = current_fmt.contents("pixelformat").string_value ();
		printf ("DEBUG: fmt = '%s'\n", fmt.c_str());
		uint32NDArray s = current_fmt.contents("size").uint32_array_value ();
		UINT32 width = s(0);
		UINT32 height = s(1);
		printf ("DEBUG: size = [%i %i]\n", width, height);

		if (fmt == "YUY2")
			// YUYV aka YUV 4:2:2 aka YUY2
			// return struct with fields Y, Cb, Cr
			{
				ret(0) = imaq_handler::get_YUYV (data, size, width, height);
			}
		else if (fmt == "NV12")
			{
				ret(0) = imaq_handler::get_NV12 (data, size, width, height);
			}
		else
		{
			// return buffer verbatim
      dim_vector dv (size, 1);
      uint8NDArray img (dv);
      unsigned char *p = reinterpret_cast<unsigned char*>(img.fortran_vec());
      memcpy(p, data, size);
      ret(0) = octave_value(img);
		}


/*
		{
			HANDLE h = CreateFileA("image.jpg", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			assert(h != INVALID_HANDLE_VALUE);

			DWORD written;
			BOOL ok = WriteFile(h, data, size, &written, NULL);
			assert(ok && written == size);

			CloseHandle(h);
		}
*/
		buffer->Unlock ();
		buffer->Release ();
	}

	sample->Release ();

	return ret;
}

void
mf_handler::close ()
{
	octave_stdout << "mf_handler::close called" << std::endl;

  //~ streamoff();
  //~ if (fd >= 0)
    //~ v4l2_close(fd);
  //~ fd = -1;
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
