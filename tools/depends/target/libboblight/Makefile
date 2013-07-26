include ../../Makefile.include
DEPS= ../../Makefile.include Makefile

LIBNAME=boblight
VERSION=e1667a77a89d6d877fb6d15a3b11322e1bdf8775
GIT_DIR=$(TARBALLS_LOCATION)/$(LIBNAME).git
BASE_URL=git@github.com:Pivosgroup/$(LIBNAME).git
DYLIB=$(PLATFORM)/src/.libs/libboblight.so
XBMC_ADDONSDIR=../../../../addons

#tell git to use the addons repo rather than xbmc's repo
export GIT_DIR
export GIT_WORK_TREE=$(PLATFORM)

# configuration settings
CONFIGURE=./configure --prefix=$(PREFIX) \
	--without-portaudio --without-opengl --without-x11

all: .installed-$(PLATFORM)

$(GIT_DIR)/HEAD:
	cd $(TARBALLS_LOCATION); git clone --bare $(BASE_URL)

$(GIT_DIR)/current/$(VERSION): $(GIT_DIR)/HEAD $(DEPS)
	git rev-list -1 $(VERSION) >/dev/null || git fetch origin "+refs/heads/*:refs/remotes/origin/*"
	git rev-list -1 $(VERSION) >/dev/null
	rm -rf $(GIT_DIR)/current; mkdir -p $(GIT_DIR)/current
	touch $@

$(PLATFORM)/configure: $(GIT_DIR)/current/$(VERSION)
	rm -rf $(PLATFORM); mkdir -p $(PLATFORM)
	git checkout $(VERSION) -- .

$(PLATFORM)/Makefile: $(PLATFORM)/configure
ifeq ($(OS),android)
	cd $(PLATFORM); patch -p1 < ../rm-pthread-check.patch
endif
	cd $(PLATFORM); autoreconf -vif
	cd $(PLATFORM); $(CONFIGURE)

$(DYLIB): $(PLATFORM)/Makefile
	make -C $(PLATFORM)
	touch $@

.installed-$(PLATFORM): $(DYLIB)
	$(MAKE) -C $(PLATFORM) install
ifeq ($(OS),android)
	rm -f $(PREFIX)/lib/libboblight.la $(PREFIX)/lib/libboblight.so $(PREFIX)/lib/libboblight.so.0
	mv -f $(PREFIX)/lib/libboblight.so.0.0.0 $(PREFIX)/lib/libboblight.so
	$(RPL) -e "libboblight.so.0" "libboblight.so\x00\x00" $(PREFIX)/lib/libboblight.so
	-$(READELF) --dynamic $(PREFIX)/lib/libboblight.so | grep ibrary
	zip libboblight.so.zip $(PREFIX)/lib/libboblight.so
endif
	touch $@

clean:
	-make -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)
	rm -f libboblight.so.zip

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)
	rm -rf $(GIT_DIR)/current
	rm -f libboblight.so.zip

