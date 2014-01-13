## 13.01.2014 Andreas Weber
## Simple test script for __xxx__.oct
## with focus on streaming an preview

if(!exist("x", "var"))
 x = __imaq_handler_open__("/dev/video0");
endif

__imaq_handler_s_fmt__(x, 640, 480);
__imaq_handler_streamon__(x, 5);

l = 300;
t = zeros(l, 1);
for i=1:l
  [img, seq, timestamp] = __imaq_handler_capture__(x, 1);
  t(i) = timestamp.tv_sec + timestamp.tv_usec./1e6;
endfor

__imaq_handler_streamoff__(x);
__imaq_handler_s_fmt__(x, 1280, 960);
__imaq_handler_streamon__(x, 5);
for i=1:l
  [img, seq, timestamp] = __imaq_handler_capture__(x, 1);
  t(i) = timestamp.tv_sec + timestamp.tv_usec./1e6;
endfor

__imaq_handler_streamoff__(x);
