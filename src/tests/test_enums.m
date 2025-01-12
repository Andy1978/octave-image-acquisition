# Test enumeration of format, framse size and frame interval

clear all
addpath ("../../inst") # for __test__device__
load_imaq

x = __imaq_handler_open__(__test__device__{:})
fmts = __imaq_handler_enum_formats__(x)

# Die Rückgabe unterscheidet sich sehr zwischen v4l2 und mf
# v4l2 gibt die 6 FOURCC BGR3, MJPG, RGB3, YU12, YUYV und YV12 zurück,
# mf   gibt die 3 FOURCC MJPG, NV12 und YUY2
#      und auch frame size, frame_rate und alle Kombinationen (daher numel == 312)

# TODO: sollte man in die Doku zu __imaq_handler_enum_formats__ packen (ins .cc file)
#{
mf:
    size = 640   480
    frame_rate = 30    1
    fourcc = YUY2
    MF_MT_SUBTYPE_CLSID = {32595559-0000-0010-8000-00AA00389B71}
    flags_compressed = 0
    flags_emulated = 0

v4l2:

    type = Video Capture
    description = YUYV 4:2:2
    pixelformat = YUYV
    fourcc = YUYV
    flags_compressed = 0
    flags_emulated = 0
#}

__imaq_handler_enum_framesizes__ (x, "MJPG")

__imaq_handler_enum_frameintervals__ (x, [1024 576], "MJPG")
