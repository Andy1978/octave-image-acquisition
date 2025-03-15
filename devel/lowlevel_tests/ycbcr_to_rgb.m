# Playground to develop the C++ code for YCbCr to RGB24 conversion

clear all
load_imaq

load ("cube_ycbcr.bin", "img");

print_stats = @(X) printf ("%s, min: %i, mean: %i, max: %i\n", typeinfo(X), min(X(:)), mean(X(:)), max(X(:)));

print_stats (img.Y)
print_stats (img.Cb)
print_stats (img.Cr)

# image package
pkg load image
tmp = cat(3, img.Y, kron (img.Cb, [1 1]), kron (img.Cr, [1 1]));
rgb = ycbcr2rgb (tmp, "601");
figure (1); imshow(rgb)

# check the C++ implementation
tic
for k = 1:10
  r = __imaq_handler_YCbCr_to_RGB__ (img, 601);
endfor
100 * toc ()
figure (2); imshow(r)

d = double (rgb) - double (r);
assert (! any (abs(d(:))>1))
