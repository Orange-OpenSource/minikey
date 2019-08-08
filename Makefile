.PHONY: default clean prebuilt

NDKBUILT := \
  libs/arm64-v8a/minikey \
  libs/arm64-v8a/minikey-nopie \
  libs/armeabi/minikey \
  libs/armeabi/minikey-nopie \
  libs/armeabi-v7a/minikey \
  libs/armeabi-v7a/minikey-nopie \
  libs/mips/minikey \
  libs/mips/minikey-nopie \
  libs/mips64/minikey \
  libs/mips64/minikey-nopie \
  libs/x86/minikey \
  libs/x86/minikey-nopie \
  libs/x86_64/minikey \
  libs/x86_64/minikey-nopie \

default: prebuilt

clean:
	ndk-build clean
	rm -rf prebuilt

$(NDKBUILT):
	ndk-build

# It may feel a bit redundant to list everything here. However it also
# acts as a safeguard to make sure that we really are including everything
# that is supposed to be there.
prebuilt: \
  prebuilt/arm64-v8a/bin/minikey \
  prebuilt/arm64-v8a/bin/minikey-nopie \
  prebuilt/armeabi/bin/minikey \
  prebuilt/armeabi/bin/minikey-nopie \
  prebuilt/armeabi-v7a/bin/minikey \
  prebuilt/armeabi-v7a/bin/minikey-nopie \
  prebuilt/mips/bin/minikey \
  prebuilt/mips/bin/minikey-nopie \
  prebuilt/mips64/bin/minikey \
  prebuilt/mips64/bin/minikey-nopie \
  prebuilt/x86/bin/minikey \
  prebuilt/x86/bin/minikey-nopie \
  prebuilt/x86_64/bin/minikey \
  prebuilt/x86_64/bin/minikey-nopie \

prebuilt/%/bin/minikey: libs/%/minikey
	mkdir -p $(@D)
	cp $^ $@

prebuilt/%/bin/minikey-nopie: libs/%/minikey-nopie
	mkdir -p $(@D)
	cp $^ $@
