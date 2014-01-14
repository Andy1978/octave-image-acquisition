## 13.01.2014 Andreas Weber
## Simple test script for __xxx__.oct

addpath("../src")
oldval = page_screen_output(0);

x = __imaq_handler_open__("/dev/video0");

caps = __imaq_handler_querycap__(x)

inputs = __imaq_handler_enuminput__(x)

inp = __imaq_handler_g_input__(x)
__imaq_handler_s_input__(x, 0)
try
  __imaq_handler_s_input__(x, 1)
catch ERR
  disp("this should fail if input 1 is out of range")
end_try_catch

__imaq_handler_enum_fmt__(x)
__imaq_handler_enum_fmt__(x).description

fs = __imaq_handler_enum_framesizes__(x)
default_framesize = fs(1,:)

fi = __imaq_handler_enum_frameintervals__(x, default_framesize)
fps = 1./fi
 
__imaq_handler_s_fmt__(x, [1280 960]);
__imaq_handler_streamon__(x, 2);

c_fmt = __imaq_handler_g_fmt__(x)

[img, seq, timestamp] = __imaq_handler_capture__(x);

image(img)

controls = __imaq_handler_queryctrl__(x)

fieldnames(controls)

__imaq_handler_s_ctrl__(x, controls.brightness.id, 10)
__imaq_handler_g_ctrl__(x, controls.brightness.id)
__imaq_handler_s_ctrl__(x, controls.brightness.id, 210)
__imaq_handler_g_ctrl__(x, controls.brightness.id)


%~ __imaq_handler_streamoff__(x);
%~ __imaq_handler_s_fmt__(x, 1280, 960);
%~ __imaq_handler_streamon__(x, 5);
%~ [img, seq, timestamp] = __imaq_handler_capture__(x);

#clear x

page_screen_output(oldval);

########### testing double opens and s_fmt ##############

x2 = __imaq_handler_open__("/dev/video0");
try
  __imaq_handler_s_fmt__(x2, [640 480]);
catch ERR
  disp("INFO: this is expected because /dev/video0 is still streaming")
end_try_catch

__imaq_handler_streamoff__(x);
__imaq_handler_s_fmt__(x2, [640 480]);
__imaq_handler_g_fmt__(x2)

clear x2
clear x
