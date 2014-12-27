make -C devel clean
make -C src realclean
rm -rf release
mkdir -p release/octave-image-acquisition

cp -R inst doc src release/octave-image-acquisition
cp COPYING DESCRIPTION INDEX NEWS PKG_ADD PKG_DEL release/octave-image-acquisition
cd release/octave-image-acquisition/src
make realclean
./bootstrap
cd ../../
IA_PKG_VER=$(awk '$1~/Version:/{print $2}' octave-image-acquisition/DESCRIPTION)
tar czf image-acquisition-$IA_PKG_VER.tar.gz octave-image-acquisition
md5sum image-acquisition-$IA_PKG_VER.tar.gz
octave -q --eval "pkg uninstall image-acquisition"
octave -q --eval "more off; pkg install -verbose image-acquisition-$IA_PKG_VER.tar.gz;"
octave -q --eval "pkg load image-acquisition; pkg load generate_html; graphics_toolkit gnuplot; generate_package_html('image-acquisition', 'image-acquisition-html', 'octave-forge')"
tar czf image-acquisition-html.tar.gz image-acquisition-html
md5sum image-acquisition-html.tar.gz
