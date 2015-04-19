## Makefile to simplify Octave Forge package maintenance tasks

PACKAGE = $(shell $(SED) -n -e 's/^Name: *\(\w\+\)/\1/p' DESCRIPTION | $(TOLOWER))
VERSION = $(shell $(SED) -n -e 's/^Version: *\(\w\+\)/\1/p' DESCRIPTION | $(TOLOWER))
DEPENDS = $(shell $(SED) -n -e 's/^Depends[^,]*, \(.*\)/\1/p' DESCRIPTION | $(SED) 's/ *([^()]*),*/ /g')

RELEASE_DIR     = $(PACKAGE)-$(VERSION)
RELEASE_TARBALL = $(PACKAGE)-$(VERSION).tar.gz
HTML_DIR        = $(PACKAGE)-html
HTML_TARBALL    = $(PACKAGE)-html.tar.gz

M_SOURCES   = $(wildcard inst/*.m)
CC_SOURCES  = $(wildcard src/*.cc)
OCT_FILES   = $(patsubst %.cc,%.oct,$(CC_SOURCES))
PKG_ADD     = $(shell grep -Pho '(?<=// PKG_ADD: ).*' $(CC_SOURCES) $(M_SOURCES))

MD5SUM    ?= md5sum
OCTAVE    ?= octave
SED       ?= sed
TAR       ?= tar

TOLOWER = $(SED) -e 'y/ABCDEFGHIJKLMNOPQRSTUVWXYZ/abcdefghijklmnopqrstuvwxyz/'

.PHONY: help dist html release install all check run debug clean

help:
	@echo "Targets:"
	@echo "   dist    - Create $(RELEASE_TARBALL) for release"
	@echo "   html    - Create $(HTML_TARBALL) for release"
	@echo "   release - Create both of the above and show md5sums"
	@echo
	@echo "   install - Install the package in GNU Octave"
	@echo "   all     - Build all oct files"
	@echo "   check   - Execute package tests (w/o install)"
	@echo "   run     - Run Octave with development in PATH (w/o install)"
	@echo "   debug   - Build with debug flags, run and attach gdb (w/o install)"
	@echo
	@echo "   clean   - Remove releases, html documentation, and oct files"

$(RELEASE_DIR): .hg/dirstate
	@echo "Creating package version $(VERSION) release ..."
	-rm -rf $@
	hg archive --exclude ".hg*" --exclude "Makefile" --type files "$@"
	cd "$@" && rm -rf "devel/" && cd "src/" && ./bootstrap && cd - && rm -rf "src/autom4te.cache"
	chmod -R a+rX,u+w,go-w $@

$(RELEASE_TARBALL): $(RELEASE_DIR)
	$(TAR) cf - --posix "$<" | gzip -9n > "$@"

$(HTML_DIR): install
	@echo "Generating HTML documentation. This may take a while ..."
	-rm -rf "$@"
	$(OCTAVE) --silent \
	  --eval "pkg load generate_html; " \
	  --eval "pkg load $(PACKAGE);" \
	  --eval 'generate_package_html ("${PACKAGE}", "$@", "octave-forge");'
	chmod -R a+rX,u+w,go-w $@

$(HTML_TARBALL): $(HTML_DIR)
	tar cf - --posix "$<" | gzip -9n > "$@"

dist: $(RELEASE_TARBALL)
html: $(HTML_TARBALL)

release: dist html
	$(MD5SUM) $(RELEASE_TARBALL) $(HTML_TARBALL)
	@echo "Upload @ https://sourceforge.net/p/octave/package-releases/new/"
	@echo 'Execute: hg tag "release-${VERSION}"'

## Note that in development versions this target may fail if we are dependent
## on unreleased versions.  This is by design, to force possible developers
## to set this up by hand (either using the "-nodeps" option" or changing the
## dependencies on DESCRIPTION.
install: $(RELEASE_TARBALL)
	@echo "Installing package locally ..."
	$(OCTAVE) --silent --eval 'pkg ("install", "${RELEASE_TARBALL}")'

all: $(CC_SOURCES)
	cd src/ && ./configure
	$(MAKE) -C src/


check: all
	$(OCTAVE) --no-window-system --silent \
	  --eval 'addpath (fullfile ([pwd filesep "inst"]));' \
	  --eval 'addpath (fullfile ([pwd filesep "src"]));' \
	  --eval '${PKG_ADD}' \
	  --eval 'run ("./devel/run_tests")'
#FIXME: I haven't yet found a way to run the test in @classes
#--eval 'runtests ("inst"); runtests ("src");' \

run: all
	$(OCTAVE) --no-gui --silent --persist \
	  --eval 'addpath (fullfile ([pwd filesep "inst"]));' \
	  --eval 'addpath (fullfile ([pwd filesep "src"]));' \
	  --eval '${PKG_ADD}'

debug: clean
	cd src/ && ./configure
	$(MAKE) -C src/ debug
	$(OCTAVE) --no-gui --silent --persist \
	  --eval 'addpath (fullfile ([pwd filesep "inst"]));' \
	  --eval 'addpath (fullfile ([pwd filesep "src"]));' \
	  --eval '${PKG_ADD}' \
	  --eval 'system (sprintf ("xfce4-terminal --command \"gdb -p %d\"", getpid ()), "async");'

clean:
	rm -rf $(RELEASE_DIR) $(RELEASE_TARBALL) $(HTML_TARBALL) $(HTML_DIR)
	rm -f ./devel/fntest.log
	-test -e src/Makefile && $(MAKE) -C src clean

