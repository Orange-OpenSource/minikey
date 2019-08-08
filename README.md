# minikey

Minikey provides a command line interface to send and receive keyboard events on [KaiOS](https://www.kaiostech.com/) devices.
Minikey implements the mapping between physical keyboard and KaiOS internal input events via the [libevdev](https://www.freedesktop.org/wiki/Software/libevdev/) interface.

Minikey may be used by any test tool for test automation. You can record keyboard events and then replay events as you want. 

Note: Minikey has been tested only on Nokia 8110 with the following [mapping table] (MAPPING.md).


## Building

Building requires [NDK](https://developer.android.com/tools/sdk/ndk/index.html) (revision 10 at least).
Project include [LIBEVDEV](http://www.freedesktop.org/wiki/Software/libevdev/) as a GIT submodule to initialize first:

```
git submodule init
git submodule update
```

Then use the command `ndk-build` from your Android SDK (see your directory $ANDROID_SDK/ndk-bundle).

```
ndk-build
```
You should now have binaries available in a directory `./libs`.


## Deploy on device

You can use the included [run.sh](run.sh) script to build and deploy the right binary on your device.

Or manually, you have to first detect which ABI (Application Binary Interface) your device supports:

```bash
ABI=$(adb shell getprop ro.product.cpu.abi | tr -d '\r')
```

Now, push the appropriate binary to the device:

```bash
adb push libs/$ABI/minikey /data/local/tmp/
```

Note that for SDK < 16, you will have to use the `minikey-nopie` executable which comes without [PIE](http://en.wikipedia.org/wiki/Position-independent_code#Position-independent_executables) support. Check [run.sh](run.sh) for a scripting example.

## Usage

Display the command help:

```bash
adb shell /data/local/tmp/minikey -h

Usage: /data/local/tmp/minikey [-h] [-w key] [-r input] [-i] [-f file]
  -w <key>  : Write keys.
  -r <input>: Read keys from a given input (eg. '/dev/input/event0' or 'all' for all inputs at a time).
  -i        : Uses STDIN to send key values.
  -f <file> : Runs a file with a list of commands, doesn't start socket.
  -h        : Show help.
```

Send one or several key press events (see also the [mapping table] (MAPPING.md) for special keys):

```bash
adb shell /data/local/tmp/minikey -w 123pdo
```

Send one or several key press events from a file:

```bash
adb shell /data/local/tmp/minikey -f keys.txt
```

Send one or several key press events from STDIN (press <ENTER> to send values):

```bash
adb shell /data/local/tmp/minikey -i
```

Read key events from a given input:

```bash
adb shell /data/local/tmp/minikey -r /dev/input/event0
```

Read key events from any input event:

```bash
adb shell /data/local/tmp/minikey -r all
```


## License

See [LICENSE](LICENSE.txt).

Copyright © Orange, Inc. All Rights Reserved.
