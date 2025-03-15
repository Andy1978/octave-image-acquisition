# check controls (only brightness in this test)

clear all
load_imaq

__test__device__

x = __imaq_handler_open__(__test__device__{:});
ctrls = __imaq_handler_queryctrl__(x)

if (isfield(ctrls, "brightness"))
  min_brightness = ctrls.brightness.min;
  max_brightness = ctrls.brightness.max;

  __imaq_handler_s_ctrl__(x, ctrls.brightness.id, min_brightness);
  assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), min_brightness)

  __imaq_handler_s_ctrl__(x, ctrls.brightness.id, max_brightness);
  assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), max_brightness)

  v = round(max_brightness/2);
  __imaq_handler_s_ctrl__(x, ctrls.brightness.id, v);
  assert(__imaq_handler_g_ctrl__(x, ctrls.brightness.id), v);
endif

 __imaq_handler_queryctrl__(x).gain
old_gain = __imaq_handler_g_ctrl__(x, ctrls.gain.id)

__imaq_handler_s_ctrl__(x, ctrls.gain.id, 100)
new_gain = __imaq_handler_queryctrl__(x).gain

# back to auto
__imaq_handler_s_ctrl__(x, ctrls.gain.id, [])

# it looks like you have to poll images to adjust auto-gain
do
  [img, seq, timestamp] = __imaq_handler_capture__(x, 1, 0);
  auto_gain = __imaq_handler_queryctrl__(x).gain.value
until (! __imaq_preview_window_is_shown__(x))
