## 0x00980911 is Exposure
## see yavta /dev/v4l-subdev8 -l
## yavta -w "0x00980911 1000" /dev/v4l-subdev8
## ex is an integer value (1 = 56.60 µs)

function set_mt9p031_exposure (ex)

  cmd = sprintf('yavta -w "0x00980911 %d" /dev/v4l-subdev8', ex);
  system (cmd);

endfunction
