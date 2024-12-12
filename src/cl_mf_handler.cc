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
	printf ("DEBUG: len = %i\n", len);
	// FIXME/TODO: check if the conversion can be made directly into std::string
	char buf[len];
	WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, len, 0, 0);
	printf ("DEBUG: wchar_to_utf8 returns '%s'\n", buf);
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

		// Output the wide string
		wcout << L"Converted wide string: " << wide_str << endl;
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
  IMFMediaSource* device;
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

		// man könnte überlegen den Code hier mit dem aus enum_devices zusammenzulegen under
		// den Filter nach symlink optional...

    attr->Release();
  }

  // create reader
  hr = MFCreateSourceReaderFromMediaSource(device, NULL, &reader);
  CHECK(hr);
  device->Release();

  enum_formats ();

  s_fmt ("MJPG", 640, 480);

  return ret;
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
		  octave_scalar_map sm;
      //if (hr == MF_E_NO_MORE_TYPES)
      //  {
      //    hr = S_OK;
      //    break;
      //  }
      //else if (SUCCEEDED(hr))
        {
          UINT32 w = 0, h = 0;
          hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &w, &h);
					CHECK (hr)
          printf ("DEBUG: FRAME_SIZE w = %4i, h = %4i ", w, h);

					sm.assign ("frame_width", w);
					sm.assign ("frame_height", h);


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
					CHECK (hr);

					OLECHAR* guidString;
					StringFromCLSID(sub, &guidString);
					printf ("DEBUG: guidString = '%S', ", guidString);
          sm.assign ("subtype_CLSID", wchar_to_utf8 (guidString));

					::CoTaskMemFree(guidString);

					// von Andy: die ersten 4 Bytes von Data1 scheinen FOURCC zu sein
					char tmp_fourcc[5];
					tmp_fourcc[0] = int8_t (sub.Data1 & 0xff);
					tmp_fourcc[1] = int8_t ((sub.Data1 & 0xff00) >> 8);
					tmp_fourcc[2] = int8_t ((sub.Data1 & 0xff0000) >> 16);
					tmp_fourcc[3] = int8_t ((sub.Data1 & 0xff000000) >> 24);
					tmp_fourcc[4] = 0;

          sm.assign ("fourcc", std::string(tmp_fourcc));

					printf ("%s\n", tmp_fourcc);

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
          pType->Release();

					ret.assign(octave_idx_type(dwMediaTypeIndex), sm);
        }
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
				if (fmtstr == "MJPG")
				  hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_MJPG);
				else if (fmtstr == "YUY2")
				  hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_YUY2);
				else if (fmtstr == "NV12")
				  hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_NV12);
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

octave_scalar_map mf_handler::g_fmt ()
{
	UINT32 width;
	UINT32 height;

	// get width/height
	{
		IMFMediaType* type;

		HRESULT hr = reader->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type);
		CHECK(hr);

		UINT64 tmp;
		hr = type->GetUINT64(MF_MT_FRAME_SIZE, &tmp);
		CHECK(hr);

		width = (UINT32)(tmp >> 32);
		height = (UINT32)(tmp);

		type->Release ();
	}

	printf("Readback Size = %ux%u\n", width, height);

  Matrix s(1,2);
  s(0) = width;
  s(1) = height;

  octave_scalar_map ret;
  ret.assign ("size", s);
  ret.assign ("pixelformat", "not yet implemented");
  return ret;
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


		// einfach mal den buffer zurück geben
		{
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
  //~ streamoff();
  //~ if (fd >= 0)
    //~ v4l2_close(fd);
  //~ fd = -1;

	if (reader)
	{
		reader->Release ();
		reader = 0;
	}
}









#endif
