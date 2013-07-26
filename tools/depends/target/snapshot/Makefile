include ../../Makefile.include
#DEPS= ../../Makefile.include Makefile

LIBAPPS=$(PLATFORM)/test

CLEAN_FILES=$(PLATFORM)

all: $(LIBAPPS) .installed-$(PLATFORM)

$(PLATFORM):
	rm -rf $(PLATFORM)/*; mkdir -p $(PLATFORM)
	cp test.c $(PLATFORM)

$(LIBAPPS): $(PLATFORM)
	$(CC) -O2 -o $(PLATFORM)/test $(PLATFORM)/test.c

.installed-$(PLATFORM): $(LIBDYLIB)
	touch $@

clean:
	$(MAKE) -C $(PLATFORM) clean
	rm -f .installed-$(PLATFORM)

distclean::
	rm -rf $(PLATFORM) .installed-$(PLATFORM)

