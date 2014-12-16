## 25.06.2014 Andreas Weber
## connect pads with links and set params
## use raw bayer, see also lane shifter

function set_mt9p031_SGRBG_bitdepth (bits)

  ## TODO:
  ## check available bit depths. Only 8, 10, 12? and throw error

  system('media-ctl -v -r -l ''"mt9p031 2-0048":0->"OMAP3 ISP CCDC":0[1], "OMAP3 ISP CCDC":1->"OMAP3 ISP CCDC output":0[1]''');
  cmd = sprintf('media-ctl -v -V ''"mt9p031 2-0048":0 [SGRBG12 2592x1944], "OMAP3 ISP CCDC":0[SGRBG%d 2592x1944], "OMAP3 ISP CCDC":1 [SGRBG%d 2592x1944]''',bits, bits);
  system(cmd);

  ## cropping?
  #system('media-ctl -v -V ''"mt9p031 2-0048":0 [crop:(16,54)/640x480 fmt:SGRBG12/2592x1944], "OMAP3 ISP CCDC":1 [SGRBG12 2592x1944]''');

endfunction
