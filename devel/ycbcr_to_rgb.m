# Playground to develop the C++ code for YCbCr to RGB24 conversion

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

# nun die C++ Implementation
addpath ("../inst")
addpath ("../src")
autoload ("__imaq_enum_devices__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_open__", which ("__imaq_handler__.oct"));
autoload ("__imaq_handler_YCbCr_to_RGB__", which ("__imaq_handler__.oct"));

x = __imaq_handler_open__(__test__device__{:})
r = __imaq_handler_YCbCr_to_RGB__ (x, img, 601);
figure (2); imshow(r)
assert (!any (rgb(:)-im2uint8(r)(:)))
