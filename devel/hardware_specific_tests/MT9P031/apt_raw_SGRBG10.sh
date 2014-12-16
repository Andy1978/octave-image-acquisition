#!/bin/bash -e

## full resolution MT9P031 2592 x 1944 (5.7mm x 4.28mm)
## RAW bayer, 10bit
## The reduction of the bitdepth at the link "mt9p031->OMAP3 ISP CCDC"
## is possible due to a hardware lane shifter

media-ctl -r -l '"mt9p031 2-0048":0->"OMAP3 ISP CCDC":0[1], "OMAP3 ISP CCDC":1->"OMAP3 ISP CCDC output":0[1]'
media-ctl -V '"mt9p031 2-0048":0[SGRBG12 2592x1944], "OMAP3 ISP CCDC":0[SGRBG10 2592x1944], "OMAP3 ISP CCDC":1[SGRBG10 2592x1944]'

media-ctl --print-dot | dot -Tpng -o apt_raw_SGRBG10.png
media-ctl -p > apt_raw_SGRBG10.log

rm -f img*
#/usr/sbin/yavta -f SGRBG10 -s 2592x1944 -n 4 --capture=4 --skip 3 -F `./media-ctl -e "OMAP3 ISP CCDC output"`
#/usr/sbin/yavta -f SGRBG10 -s 2592x1944 -n 4 --capture=4 --skip 3 -F /dev/video2
/usr/sbin/yavta -f SGRBG10 -s 2592x1944 -n 4 --capture=4 --file="$(pwd)/img#.bin" -F /dev/video2

## convert RAW with https://gitorious.org/raw2rgbpnm
## raw2rgbpnm -f SGRBG10 img000000.bin out1.pnm
