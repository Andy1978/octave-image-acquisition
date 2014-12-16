## 0x00980911 is Exposure
## see yavta /dev/v4l-subdev8 -l
## ex is an integer value (1 = 56.60 µs)

function ex = get_mt9p031_exposure()

  [output, text] = system ('yavta -r 0x00980911 /dev/v4l-subdev8');
  ex = str2double(strsplit(text){7});

endfunction
