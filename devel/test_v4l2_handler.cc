// Copyright (C) 2013 Andreas Weber <andy.weber.aw@gmail.com>
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

#include <iostream>
#include "cl_v4l2_handler.h"

using namespace std;

int main()
{
  cout << __FILE__ << " " << __DATE__ << endl;
  v4l2_handler *m = new v4l2_handler();

  m->open("/dev/video0");

  octave_value caps = m->querycap();
  caps.print(cout, 0);

  octave_value enuminp = m->enuminput();
  m->s_input(0);
  m->s_input(1);
  
  enuminp.print(cout, 0);

  octave_value fmt = m->enum_fmt();
  fmt.print(cout, 0);
  octave_map tmp = fmt.map_value ();
  for (int i=0; i<fmt.numel(); ++i)
    {
      tmp.contents("pixelformat")(i).print(cout, 0);
      tmp.contents("description")(i).print(cout, 0);
      tmp.contents("flags_compressed")(i).print(cout, 0);
      tmp.contents("flags_emulated")(i).print(cout, 0);
    }
  Matrix s = m->enum_framesizes();
  cout << "enum_framesizes" << endl << s;

  s = m->enum_frameintervals(V4L2_PIX_FMT_RGB24, 640, 480);
  cout << "enum_frameintervals" << endl << s;


  octave_value ctrls = m->queryctrl();
  ctrls.print (cout, 0);

  m->s_fmt(640, 480);
  // start streaming with 2 buffers
  m->streamon(2);

  octave_value_list cap = m->capture(3);
  uint8NDArray img = cap(0).uint8_array_value();
  cap(2).print(cout, 0);
  cout << "sequence = " << cap(1).int_value() << endl;

  m->capture_to_ppm("test1.ppm");
  m->s_ctrl(V4L2_CID_BRIGHTNESS, 20);
  m->capture_to_ppm("test2.ppm");
  m->capture_to_ppm("test3.ppm");
  m->s_ctrl(V4L2_CID_BRIGHTNESS, 200);

  cap = m->capture(3);
  cap(2).print(cout, 0);
  cout << "sequence = " << cap(1).int_value() << endl;

  // change size
  m->streamoff();
  m->s_fmt(160, 120);
  m->streamon(3);

  m->capture_to_ppm("test4.ppm");
  m->capture_to_ppm("test5.ppm");

  // change size
  m->streamoff();
  m->s_fmt(640, 480);
  m->streamon(10);
  
  for (int i=0; i<100; ++i)
    m->capture(1, 1);

  m->print(cout, 0);
  /***** test double opens */
  v4l2_handler *m2 = new v4l2_handler();
  m2->open("/dev/video0");
  m2->s_fmt(640, 480);
  
  m->close();
  m2->close();

  delete m;
  return 0;
}
