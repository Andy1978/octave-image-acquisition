.PHONY: clean

TARGETS= load_imaq.m

all: $(TARGETS)

load_imaq.m: ../__imaq_handler__.cc
	echo 'addpath ("../");' > $@
	grep DEFUN_DLD $< | grep -oP "__.*__" | xargs -I{} printf 'autoload ("%s", which ("__imaq_handler__.oct"));\n' {} >> $@

clean:
	rm -f $(TARGETS)
