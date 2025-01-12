# nur die controls testen

autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));


autoload ("__imaq_handler_queryctrl__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_g_ctrl__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_s_ctrl__", which ("__imaq_handler__.oct"));

addpath ("../inst")

__test__device__

x = __imaq_handler_open__(__test__device__{:});
ctrls = __imaq_handler_queryctrl__(x);

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

#{
ctrls =

  scalar structure containing the fields:

    user_controls =

      scalar structure containing the fields:

        id = 9961473
        min = 0
        max = 0

    brightness =

      scalar structure containing the fields:

        id = 9963776
        min = 0
        max = 255
        step = 1
        default = 128

    contrast =

      scalar structure containing the fields:

        id = 9963777
        min = 0
        max = 255
        step = 1
        default = 32

    saturation =

      scalar structure containing the fields:

        id = 9963778
        min = 0
        max = 255
        step = 1
        default = 32

    white_balance_automatic =

      scalar structure containing the fields:

        id = 9963788
        min = 0
        max = 1
        default = 1

    gain =

      scalar structure containing the fields:

        id = 9963795
        min = 0
        max = 255
        step = 1
        default = 0

    power_line_frequency =

      scalar structure containing the fields:

        id = 9963800
        min = 0
        max = 2
        default = 2
        menu = 0:Disabled;1:50 Hz;2:60 Hz;

    white_balance_temperature =

      scalar structure containing the fields:

        id = 9963802
        min = 0
        max = 10000
        step = 10
        default = 4000

    sharpness =

      scalar structure containing the fields:

        id = 9963803
        min = 0
        max = 255
        step = 1
        default = 24

    backlight_compensation =

      scalar structure containing the fields:

        id = 9963804
        min = 0
        max = 1
        step = 1
        default = 1

    camera_controls =

      scalar structure containing the fields:

        id = 1.0093e+07
        min = 0
        max = 0

    auto_exposure =

      scalar structure containing the fields:

        id = 1.0095e+07
        min = 0
        max = 3
        default = 3
        menu = 1:Manual Mode;3:Aperture Priority Mode;

    exposure_time_absolute =

      scalar structure containing the fields:

        id = 1.0095e+07
        min = 1
        max = 10000
        step = 1
        default = 166

    exposure_dynamic_framerate =

      scalar structure containing the fields:

        id = 1.0095e+07
        min = 0
        max = 1
        default = 0


octave:3> ctrls.brightness
ans =

  scalar structure containing the fields:

    id = 9963776
    min = 0
    max = 255
    step = 1
    default = 128

octave:4> exit
#}
