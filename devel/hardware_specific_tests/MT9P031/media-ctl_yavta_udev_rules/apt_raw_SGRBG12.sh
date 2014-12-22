#!/bin/bash -e

## full resolution MT9P031 2592 x 1944 (5.7mm x 4.28mm)
## RAW bayer (no preview or resizer), 12bit (max bitdepth)

media-ctl -r -l '"mt9p031 2-0048":0->"OMAP3 ISP CCDC":0[1], "OMAP3 ISP CCDC":1->"OMAP3 ISP CCDC output":0[1]'
media-ctl -V '"mt9p031 2-0048":0 [SGRBG12 2592x1944], "OMAP3 ISP CCDC":1 [SGRBG12 2592x1944]'

media-ctl --print-dot | dot -Tpng -o apt_raw_SGRBG12.png
media-ctl -p > apt_raw_SGRBG12.log

rm -f img*

/usr/sbin/yavta -f SGRBG12 -s 2592x1944 -n 3 --capture=1000 --skip=980 --file="$(pwd)/img#.bin" /dev/video2

## convert RAW with https://gitorious.org/raw2rgbpnm
## raw2rgbpnm -f SGRBG12 img000000.bin out1.pnm
