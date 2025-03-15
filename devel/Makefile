.PHONY:check
.PHONY:clean

check:
	$(MAKE) -C ../src
	octave -q run_tests.m

debug:
	$(MAKE) -C ../src clean
	$(MAKE) -C ../src $@

clean:
	rm -f fntest.log
	rm -f ex1_diary.log *.png
	rm -f octave-core octave-workspace
