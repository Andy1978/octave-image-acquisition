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

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h> // für MF_E_NO_MORE_TYPES

#define CHECK(hr) if (!SUCCEEDED(hr)) fprintf (stderr, "failed with %li\n", GetLastError());

mf_handler::mf_handler ()
  : imaq_handler() /*,
    fd(-1), n_buffer(0), buffers(0), streaming(0),
    preview_window(0), _is_video_capture (0), _is_meta_capture(0)*/
{
  octave_stdout << "mf_handler C'Tor" << endl;
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

  // stop streaming, unmap & free buffers, close v4l2 device
  close();
}

void
mf_handler::print (std::ostream& os, bool pr_as_read_syntax = false)
{
  os << "This is class mf_handler" << endl;
  //os << "dev = " << dev << ", fd = " << fd << ", n_buffer = " << n_buffer << ", streaming = " << ((streaming)? "true":"false") << endl;
}

typedef std::vector<std::string> dev_vec;

// ohje: https://learn.microsoft.com/en-us/archive/msdn-magazine/2016/september/c-unicode-encoding-conversions-with-stl-strings-and-win32-apis
string GetAllocatedString (IMFActivate* device, REFGUID guidKey)
{
	UINT32 length;
	LPWSTR val;
	HRESULT hr = device->GetAllocatedString(guidKey, &val, &length);
	CHECK(hr);
	
	int len = WideCharToMultiByte(CP_UTF8, 0, val, -1, 0, 0, 0, 0);
	//printf ("len = %i\n", len);

	char buf[len];

	WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, len,0,0);
  CoTaskMemFree(val);

	//printf ("buf = '%s'\n", buf);
	return buf;
}

octave_map
mf_handler::list_devices ()
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

	int len = MultiByteToWideChar(CP_UTF8, 0, d.c_str(), -1, 0, 0);
	if (len == 0)
	{
			cerr << "Error in MultiByteToWideChar: "
					 << GetLastError() << endl;
	}
	
  // Convert the string
  wstring wide_str(len, 0);
  MultiByteToWideChar(CP_UTF8, 0, d.c_str(), -1, &wide_str[0], len);

  // Output the wide string
  wcout << L"Converted wide string: " << wide_str << endl;

  // create device from symlink
  IMFMediaSource* device;
  {
    IMFAttributes* attr;

    hr = MFCreateAttributes(&attr, 2);
    CHECK(hr);

    hr = attr->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    CHECK(hr);

    hr = attr->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, wide_str.c_str());
    CHECK(hr);

    hr = MFCreateDeviceSource(attr, &device);
    CHECK(hr);

    attr->Release();
  }

  // create reader
  IMFSourceReader* reader;

  hr = MFCreateSourceReaderFromMediaSource(device, NULL, &reader);
  CHECK(hr);
  device->Release();

  // enumerating output formats from
  // https://learn.microsoft.com/en-us/windows/win32/medfound/processing-media-data-with-the-source-reader

  DWORD dwStreamIndex = MF_SOURCE_READER_FIRST_VIDEO_STREAM;
  DWORD dwMediaTypeIndex = 0;

  // https://stackoverflow.com/questions/7968547/how-to-get-a-list-of-all-microsoft-media-foundation-transforms-mfts-available

  while (SUCCEEDED(hr))
    {
      IMFMediaType *pType = NULL;
      hr = reader->GetNativeMediaType(dwStreamIndex, dwMediaTypeIndex, &pType);
      if (hr == MF_E_NO_MORE_TYPES)
        {
          hr = S_OK;
          break;
        }
      else if (SUCCEEDED(hr))
        {
          UINT32 w = 0, h = 0;
          hr = MFGetAttributeSize(pType, MF_MT_FRAME_SIZE, &w, &h);
					CHECK (hr)
          printf ("w = %4i, h = %4i ", w, h);

					GUID sub;
					// https://learn.microsoft.com/en-us/windows/win32/medfound/video-subtype-guids
					hr = pType->GetGUID (MF_MT_SUBTYPE, &sub);
					
					OLECHAR* guidString;
					StringFromCLSID(sub, &guidString);
					printf ("guidString = '%S', ", guidString);
					::CoTaskMemFree(guidString);
					
					// von Andy: die ersten bytes scheinen FOURCC zu sein
					printf ("%c", int8_t (sub.Data1 & 0xff));
					printf ("%c", int8_t ((sub.Data1 & 0xff00) >> 8));
					printf ("%c", int8_t ((sub.Data1 & 0xff0000) >> 16));
					printf ("%c, ", int8_t ((sub.Data1 & 0xff000000) >> 24));

					if (sub == MFVideoFormat_MJPG)
						printf ("MFVideoFormat_MJPG\n");
					else if (sub == MFVideoFormat_YUY2)
						printf ("MFVideoFormat_YUY2\n");
					else if (sub == MFVideoFormat_NV12)
						printf ("MFVideoFormat_NV12\n");
				  else
 					  printf ("sub = 0x%lx\n", sub.Data1);

          pType->Release();


					//if (sub == MFVideoFormat_RGB32)
					//	printf ("ffooooo");

        }
      ++dwMediaTypeIndex;

			//printf ("MFVideoFormat_RGB32 = %i\n", MFVideoFormat_RGB32.Data1);
      //printf ("MFVideoFormat_RGB32 = %i\n", MFVideoFormat_RGB32);
			
    }

  // ende enumerate
	
	printf ("----------\n\n");

	#define out(x) printf (#x" %lx\n", x.Data1)

	out (MFVideoFormat_AI44);
	out (MFVideoFormat_AYUV);
	out (MFVideoFormat_I420);
	out (MFVideoFormat_IYUV);
	out (MFVideoFormat_NV11);
	out (MFVideoFormat_NV12);
	out (MFVideoFormat_NV21);
	out (MFVideoFormat_UYVY);
	out (MFVideoFormat_Y41P);
	out (MFVideoFormat_Y41T);
	out (MFVideoFormat_Y42T);
	out (MFVideoFormat_YUY2);
	out (MFVideoFormat_YVU9);
	out (MFVideoFormat_YV12);
	out (MFVideoFormat_YVYU);


  // this assumes camera can provide mjpeg output
  // typically webcams provide YUV2 format, you'll need to convert it to
  // RGB yourself or with help of IMFTransform
  // you can enumerate all supported types with IMFSourceReader_GetNativeMediaType
	
	// Andy: das müsste in eine eigene Methode
  {
    IMFMediaType* type;

    hr = MFCreateMediaType(&type);
    CHECK(hr);

    hr = type->SetGUID (MF_MT_MAJOR_TYPE, MFMediaType_Video);
    CHECK(hr);

    hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_MJPG);
    //hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_YUY2);
    //hr = type->SetGUID (MF_MT_SUBTYPE, MFVideoFormat_NV12);
    CHECK(hr);

    // you can also set desired width/height here

    hr = reader->SetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, NULL, type);
    CHECK(hr);

    type->Release ();
  }

  UINT32 width;
  UINT32 height;

  // get width/height
  {
    IMFMediaType* type;

    hr = reader->GetCurrentMediaType (MF_SOURCE_READER_FIRST_VIDEO_STREAM, &type);
    CHECK(hr);

    UINT64 tmp;

    hr = type->GetUINT64(MF_MT_FRAME_SIZE, &tmp);
    CHECK(hr);

    width = (UINT32)(tmp >> 32);
    height = (UINT32)(tmp);

    type->Release ();
  }

  printf("Size = %ux%u\n", width, height);
	

  // read one frame and save it to file
  {
    IMFSample* sample;

    DWORD stream;
    DWORD flags;
    LONGLONG timestamp;

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

      {
        HANDLE h = CreateFileA("image.jpg", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        assert(h != INVALID_HANDLE_VALUE);

        DWORD written;
        BOOL ok = WriteFile(h, data, size, &written, NULL);
        assert(ok && written == size);

        CloseHandle(h);
      }

      buffer->Unlock ();

      buffer->Release ();
    }

    sample->Release ();
  }

  reader->Release ();

  MFShutdown();

  CoUninitialize();

  return ret;
}

void
mf_handler::close ()
{
  //~ streamoff();
  //~ if (fd >= 0)
    //~ v4l2_close(fd);
  //~ fd = -1;
}

//~ v4l2_handler*
//~ get_v4l2_handler_from_ov (octave_value ov)
//~ {
  //~ if (ov.type_id() != v4l2_handler::static_type_id())
    //~ {
      //~ error("get_v4l2_handler_from_ov: Not a valid v4l2_handler");
      //~ return 0;
    //~ }

  //~ v4l2_handler* imgh = 0;
  //~ const octave_base_value& rep = ov.get_rep();
  //~ imgh = &((v4l2_handler &)rep);
  //~ return imgh;
//~ }
#endif