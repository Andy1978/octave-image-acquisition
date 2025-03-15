.PHONY: clean

TARGETS= load_imaq.m
#standalone_check.exe

all: $(TARGETS)

load_imaq.m: ../../src/__imaq_handler__.cc
	echo 'addpath ("../../src/");' > $@
	echo 'addpath ("../../inst"); # for __test__device__' >> $@
	grep DEFUN_DLD $< | grep -oP "__.*__" | xargs -I{} printf 'autoload ("%s", which ("__imaq_handler__.oct"));\n' {} >> $@

#standalone_check.exe: standalone_check.cc ../cl_mf_handler.cc ../cl_imaq_handler.cc ../cl_img_win.cc
#	mkoctfile --link-stand-alone $^ -lmfplat -lmf -lmfreadwrite -lmfuuid -lshlwapi -lole32 -lksguid -lamstrmid -ljpeg -lfltk -lole32 -lcomctl32 -o $@
#	#mkoctfile --link-stand-alone $^ -o $@

clean:
	rm -f $(TARGETS)
