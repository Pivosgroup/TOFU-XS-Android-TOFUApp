include ../../depends/Makefile.include

OBJS  = libxbmad.so
OBJS += libxbnfs.so
OBJS += libxbcurl.so
OBJS += libxbrtmp.so
OBJS += libxbplist.so
OBJS += libxbafpclient.so
OBJS += libxbshairplay.so
OBJS += libxbbluray.so
OBJS += libxbass.so
OBJS += libxbogg.so
OBJS += libxbFLAC.so
OBJS += libxbvorbis.so
OBJS += libxbvorbisfile.so
OBJS += libxbmpeg2.so
OBJS += libxbmpeg2convert.so
OBJS += libxbmodplug.so
OBJS += libxbmp3lame.so

PLATFORM_OBJS =
EXCLUDED_ADDONS  = screensaver.rsxs.euphoria
EXCLUDED_ADDONS += visualization.dxspectrum
EXCLUDED_ADDONS += visualization.milkdrop
EXCLUDED_ADDONS += visualization.projectm
EXCLUDED_ADDONS += repository.xbmc.org
EXCLUDED_ADDONS += repository.pvr-ios.xbmc.org
EXCLUDED_ADDONS += repository.pvr-win32.xbmc.org
EXCLUDED_ADDONS += repository.pvr-osx32.xbmc.org
EXCLUDED_ADDONS += repository.pvr-osx64.xbmc.org
EXCLUDED_ADDONS += repository.pvr-android.xbmc.org
EXCLUDED_ADDONS += skin.confluence

# android branding
XBMCAPP = TOFUApp
XBMCLIB = libtofu.so
XBMCANDROID  = com/pivos/tofu

XBMCROOT = $(shell cd $(CURDIR)/../../..; pwd)
AAPT = $(SDKROOT)/platform-tools/aapt
DX = $(SDKROOT)/platform-tools/dx
GCC_VERSION=$(shell $(CC) -dumpversion)

X86OVERRIDES=XBMC_OVERRIDE_HOST=i686-android-linux XBMC_OVERRIDE_TOOLCHAIN=$(XBMC_X86_TOOLCHAIN)
ARMOVERRIDES=XBMC_OVERRIDE_HOST=arm-linux-androideabi XBMC_OVERRIDE_TOOLCHAIN=$(XBMC_ARM_TOOLCHAIN)

#this fixes an android ndk bork where the paths to
#prebuilt stuff follow different name schemes for
#arm and x86
ifeq ($(findstring i686,$(CPU)),i686)
ARCH=x86
CPU=x86
endif
ifeq ($(findstring arm,$(CPU)),arm)
ARCH=arm
endif

#older ndk x86
GDBPATH=$(NDKROOT)/toolchains/$(ARCH)-$(GCC_VERSION)/prebuilt/gdbserver

ifneq ($(shell test -f $(GDBPATH) && echo 1),1)
#older ndk arm
GDBPATH=$(NDKROOT)/toolchains/$(HOST)-$(GCC_VERSION)/prebuilt/gdbserver
endif

ifneq ($(shell test -f $(GDBPATH) && echo 1),1)
#vanilla ndk >= 8
GDBPATH=$(NDKROOT)/prebuilt/android-$(ARCH)/gdbserver/gdbserver
endif

all: pkg_apk
SRCLIBS = $(addprefix $(PREFIX)/lib/,$(OBJS)) $(addprefix $(PREFIX)/lib/$(SDK_PLATFORM)/,$(PLATFORM_OBJS))
DSTLIBS = $(CPU)/lib/$(XBMCLIB) $(addprefix $(CPU)/lib/,$(OBJS)) $(addprefix $(CPU)/lib/,$(PLATFORM_OBJS))
libs= $(DSTLIBS)

multi: x86 arm
	@cp images/$(XBMCAPP)-debug-skeleton.apk images/$(XBMCAPP)-debug-multi-unaligned.apk
	@cd xbmc; zip -r -q ../images/$(XBMCAPP)-debug-multi-unaligned.apk lib/ assets
	@jarsigner -sigalg MD5withRSA -digestalg SHA1 -keystore ~/.android/debug.keystore -storepass android images/$(XBMCAPP)-debug-multi-unaligned.apk androiddebugkey
	@$(SDKROOT)/tools/zipalign -f 4 images/$(XBMCAPP)-debug-multi-unaligned.apk $(XBMCROOT)/$(XBMCAPP)-multi-debug.apk
	@rm images/$(XBMCAPP)-debug-multi-unaligned.apk
	@echo "$(XBMCROOT)/$(XBMCAPP)-multi-debug.apk created"

pkg_apk: pkg_assets
	rm -rf images;mkdir -p images
	# setup for apk creation
	mkdir -p tmp/res; $(AAPT) c -S xbmc/res -C tmp/res; cp -r -n xbmc/res tmp/ || true
	$(AAPT) p -f -I $(SDKROOT)/platforms/$(SDK_PLATFORM)/android.jar -S tmp/res/ -M xbmc/AndroidManifest.xml -F images/$(XBMCAPP)-debug-skeleton.apk -J xbmc/src
	@rm -rf tmp/
	# create apk
	@cp images/$(XBMCAPP)-debug-skeleton.apk images/$(XBMCAPP)-debug-$(CPU)-unaligned.apk
	@javac -classpath $(SDKROOT)/platforms/$(SDK_PLATFORM)/android.jar:xbmc/obj -d xbmc/obj -sourcepath xbmc/src xbmc/src/*.java
	@javac -classpath $(SDKROOT)/platforms/$(SDK_PLATFORM)/android.jar:xbmc/obj -d xbmc/obj -sourcepath xbmc/src xbmc/src/$(XBMCANDROID)/*.java
	@$(DX) --dex --output=xbmc/classes.dex xbmc/obj
	@cd xbmc; zip -r -q ../images/$(XBMCAPP)-debug-$(CPU)-unaligned.apk lib/$(CPU) assets classes.dex
	@jarsigner -sigalg MD5withRSA -digestalg SHA1 -keystore ~/.android/debug.keystore -storepass android images/$(XBMCAPP)-debug-$(CPU)-unaligned.apk androiddebugkey
	@$(SDKROOT)/tools/zipalign -f 4 images/$(XBMCAPP)-debug-$(CPU)-unaligned.apk $(XBMCROOT)/$(XBMCAPP)-$(CPU)-debug.apk
	@rm images/$(XBMCAPP)-debug-$(CPU)-unaligned.apk
	@echo "$(XBMCROOT)/$(XBMCAPP)-$(CPU)-debug.apk created"

pkg_assets: pkg_libs
	rm -rf xbmc/assets;mkdir -p xbmc/assets
	cp -rfp $(PREFIX)/share/xbmc/* ./xbmc/assets
	# install and setup python, python stays in the apk (zip), our vfs knows about apk containers
	mkdir -p xbmc/assets/python2.6/lib/
	cp -rfp $(PREFIX)/lib/python2.6 xbmc/assets/python2.6/lib/
	cd xbmc/assets/python2.6/lib/python2.6/; rm -rf test config lib-dynload
	# remove osx junk files
	@find `pwd`/xbmc/assets/ -depth -name ".DS_Store" -exec rm {} \;
	@find `pwd`/xbmc/assets/ -depth -name "._*" -exec rm {} \;
	# remove git and any *.so files from xbmc/assets
	@find `pwd`/xbmc/assets/ -depth -name ".git" -exec rm -rf {} \;
	@find `pwd`/xbmc/assets/ -depth -name "*.so" -exec rm {} \;
	@find `pwd`/xbmc/assets/addons/skin.*/media/* -depth -not -iname "Textures.xbt" -exec rm -rf {} \;
	@find `pwd`/xbmc/assets/system/keymaps/ -depth -name "joystick*.xml" ! -name "joystick.xml" -exec rm {} \;
	# remove Linux junk files
	@rm -f ./xbmc/assets/FEH.py
	# remove any marked excluded addons
	@cd xbmc/assets/addons; rm -rf $(EXCLUDED_ADDONS)
	# rebrand xbmc -> tofu
	@find `pwd`/xbmc/assets/language/ -name strings.po -exec sed -i "s/xbmc/tofu/g" {} \;
	@find `pwd`/xbmc/assets/language/ -name strings.po -exec sed -i "s/XBMC/TOFU/g" {} \;
	# final setup before apk creation
	rm -rf xbmc/res/raw;mkdir -p xbmc/res/raw
	@echo "native_arch=$(ARCH)" > xbmc/res/raw/xbmc.properties
	cp -fp $(XBMCROOT)/media/Splash.png xbmc/res/drawable/splash.png
	cp -fp media/drawable-hdpi/ic_launcher.png xbmc/res/drawable-hdpi/ic_launcher.png
	cp -fp media/drawable-ldpi/ic_launcher.png xbmc/res/drawable-ldpi/ic_launcher.png
	cp -fp media/drawable-mdpi/ic_launcher.png xbmc/res/drawable-mdpi/ic_launcher.png
	cp -fp media/drawable-xhdpi/ic_launcher.png xbmc/res/drawable-xhdpi/ic_launcher.png
	cp -fp media/drawable-xxhdpi/ic_launcher.png xbmc/res/drawable-xxhdpi/ic_launcher.png

pkg_libs: $(PREFIX)/lib/xbmc/$(XBMCLIB)
	rm -rf xbmc/lib/$(CPU);mkdir -p xbmc/lib/$(CPU)
	# copy basic libs
	cp -fp $(SRCLIBS) xbmc/lib/$(CPU)/
	cp -fp $(PREFIX)/lib/xbmc/$(XBMCLIB) xbmc/lib/$(CPU)/
	find $(PREFIX)/lib/xbmc/addons -name "*.so" -exec cp -fp {} xbmc/lib/$(CPU)/ \;
	find $(PREFIX)/lib/xbmc/system -name "*.so" -exec cp -fp {} xbmc/lib/$(CPU)/ \;
	find $(PREFIX)/share/xbmc/addons -name "*.so" -exec cp -fp {} xbmc/lib/$(CPU)/ \;
	# python can have xxx.sofiles that needs to end up in xbmc/lib
	find $(PREFIX)/lib/python2.6 -name "*.so" -exec cp -fp {} xbmc/lib/$(CPU)/ \;
	# android libs are required to start with 'lib', make sure
	find xbmc/lib/$(CPU) -name "*.so" -not -name "lib*.so" -printf "mv '%h/%f' '%h/lib%f'\n" | sh
	# make a copy of xbmc/lib for gdbserver then strip xbmc/lib of unneeded symbols
	rm -rf xbmc/obj;mkdir -p xbmc/obj/local/$(CPU)
	cp -fp xbmc/lib/$(CPU)/* xbmc/obj/local/$(CPU)/
	find xbmc/lib/$(CPU) -name "*.so" -printf "$(STRIP) --strip-unneeded '%h/%f'\n" | sh
	# install and setup gdbserver
	rm -rf xbmc/gdb/$(CPU); mkdir -p xbmc/gdb/$(CPU)
	install -p $(GDBPATH) ./xbmc/gdb/$(CPU)/gdbserver
	echo "set solib-search-path ./obj/local/$(CPU)" > ./xbmc/gdb/$(CPU)/gdb.setup
	echo "directory $(TOOLCHAIN)/sysroot/usr/include $(NDKROOT)/sources/android/native_app_glue" \
	     "$(NDKROOT)/sources/cxx-stl/gnu-libstdc++/$(GCC_VERSION)/include $(XBMCROOT)  $(PREFIX)/include jni" >> ./xbmc/gdb/$(CPU)/gdb.setup
	cp -fp xbmc/gdb/$(CPU)/* xbmc/lib/$(CPU)

$(PREFIX)/lib/xbmc/$(XBMCLIB): $(SRCLIBS)
	$(MAKE) -C ../../depends/target/xbmc

$(SRCLIBS):

arm: armeabi-v7a

armeabi-v7a: $(XBMC_ARM_TOOLCHAIN) force
	XBMC_OVERRIDE_PLATFORM=$@ $(ARMOVERRIDES) $(MAKE) pkg_apk

x86: $(XBMC_X86_TOOLCHAIN) force
	XBMC_OVERRIDE_PLATFORM=$@ $(X86OVERRIDES) $(MAKE) pkg_apk

clean:
	rm -rf tmp images
	rm -rf xbmc/assets xbmc/gdb xbmc/lib xbmc/obj xbmc/res/raw
	rm -f  xbmc/classes.dex xbmc/src/R.java xbmc/res/drawable/splash.png

.PHONY: arm armeabi-v7a x86 force pkg_assets pkg_libs
