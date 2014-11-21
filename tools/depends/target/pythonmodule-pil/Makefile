include ../../Makefile.include
DEPS= ../../Makefile.include Makefile \
      Imaging-1.1.7-crosscompiling-0.1.patch \
      Imaging-1.1.7-setuptools-0.1.patch \
      Imaging-1.1.7-access.patch \
      Imaging-1.1.7-zip-include.patch \
      Imaging-1.1.7-remove-zlib-check.patch

# lib name, version
LIBNAME=Imaging
VERSION=1.1.7
SOURCE=$(LIBNAME)-$(VERSION)
ARCHIVE=$(SOURCE).tar.gz

ifeq ($(OS),android)
	CROSSFLAGS=PYTHONXCPREFIX="$(PREFIX)" LDFLAGS="$(LDFLAGS) -L$(PREFIX)/lib/dummy-libxbmc -ltofu -lm"
else
	CROSSFLAGS=PYTHONXCPREFIX="$(PREFIX)" LDFLAGS="$(LDFLAGS)"
endif

LIBDYLIB=$(PLATFORM)/dist/PIL-$(VERSION)-py2.6-$(OS)-$(CPU).egg

all: .installed-$(PLATFORM)

$(TARBALLS_LOCATION)/$(ARCHIVE):
	cd $(TARBALLS_LOCATION); $(RETRIEVE_TOOL) $(RETRIEVE_TOOL_FLAGS) $(BASE_URL)/$(ARCHIVE)

$(PLATFORM): $(TARBALLS_LOCATION)/$(ARCHIVE) $(DEPS)
	rm -rf $(PLATFORM)/*; mkdir -p $(PLATFORM)
	cd $(PLATFORM); $(ARCHIVE_TOOL) $(ARCHIVE_TOOL_FLAGS) $(TARBALLS_LOCATION)/$(ARCHIVE)
	cd $(PLATFORM); patch -p1 < ../Imaging-1.1.7-crosscompiling-0.1.patch
	cd $(PLATFORM); patch -p1 < ../Imaging-1.1.7-setuptools-0.1.patch
	cd $(PLATFORM); patch -p1 < ../Imaging-1.1.7-access.patch
	cd $(PLATFORM); patch -p0 < ../Imaging-1.1.7-zip-include.patch
	cd $(PLATFORM); patch -p0 < ../Imaging-1.1.7-remove-zlib-check.patch
	cd $(PLATFORM); sed -ie "s|"darwin"|"darwinNot"|g" "setup.py"
	cp setup.cfg $(PLATFORM)/

$(LIBDYLIB): $(PLATFORM)
	cd $(PLATFORM); $(CROSSFLAGS) $(NATIVEPREFIX)/bin/python setup.py build -x bdist_egg --plat-name $(OS)-$(CPU)

$(PLATFORM)/build/script.test.pil.zip:
	zip -0 -r $(PLATFORM)/build/script.test.pil.zip script.test.pil

.installed-$(PLATFORM): $(LIBDYLIB) $(PLATFORM)/build/script.test.pil.zip
	rm -rf $(PREFIX)/lib/python2.6/site-packages/PIL; mkdir -p $(PREFIX)/lib/python2.6/site-packages/PIL
	unzip -oq $(LIBDYLIB) -d $(PREFIX)/lib/python2.6/site-packages/PIL/
	echo 'PIL' > $(PREFIX)/lib/python2.6/site-packages/PIL.pth
	touch $@

clean: distclean

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
