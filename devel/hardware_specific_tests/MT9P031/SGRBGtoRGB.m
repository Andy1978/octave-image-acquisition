## 11.03.2014 Andreas Weber
## quick&dirty conversion without interpolation.
## We should implement demosaic and use this instead.

function rgb = SGRBGtoRGB(img)

  # cast to uint16 to avoid overflow
  # in average calculation g = (g1+g2)/2 below
  g1 = uint16(img(1:2:end, 1:2:end));
  g2 = uint16(img(2:2:end, 2:2:end));
  g = (g1+g2)/2;
  r = img(1:2:end, 2:2:end);
  b = img(2:2:end, 1:2:end);
  rgb = cat(3, r, g, b);

endfunction
