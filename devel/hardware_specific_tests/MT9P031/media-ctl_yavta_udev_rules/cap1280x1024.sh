#!/bin/bash -e

## capture SGRBG10 1280x1024, convert to UYVY 1280x1024 with "OMAP3 ISP preview",
## resize to 1024x768 with "OMAP3 ISP resizer"

media-ctl -r -v -l '"mt9p031 2-0048":0->"OMAP3 ISP CCDC":0[1], "OMAP3 ISP CCDC":2->"OMAP3 ISP preview":0[1], "OMAP3 ISP preview":1->"OMAP3 ISP resizer":0[1], "OMAP3 ISP resizer":1->"OMAP3 ISP resizer output":0[1]'
media-ctl -v -V '"mt9p031 2-0048":0 [SGRBG10 1280x1024], "OMAP3 ISP CCDC":2 [SGRBG10 1280x1024], "OMAP3 ISP preview":1 [UYVY 1280x1024], "OMAP3 ISP resizer":1 [UYVY 1024x768]'

media-ctl --print-dot | dot -Tpng -o cap1280x1024.png
media-ctl -p > cap1280x1024.log

rm -f img.*
/usr/sbin/yavta -f UYVY -s 1024x768 --capture=10 --skip=8 --file="$(pwd)/img#.uyvy" /dev/video6

gm mogrify -size 1024x768 -format jpg *.uyvy
