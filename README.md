QNFCd is a C++/Qt daemon that exposes NFC devices through D-Bus.

**This project have WIP status and now looks for maintainer.**

# Introduction #

QNFCd is a daemon which access NFC Devices and Targets through libnfc. It offers D-Bus services to get devices list, targets list, target content...

QNFCd is written in C++/Qt, libfreefare, libndef and libnfc.

# Requirements #
  * CMake (>= 2.6)
  * Qt4 core (>= 4.3)
  * [libnfc](http://www.libnfc.org/documentation/installation) (>= 1.7.0);
  * [libfreefare](http://code.google.com/p/libfreefare) (>= 0.3.4);
  * [libndef](http://code.google.com/p/libndef/source/browse/trunk/INSTALL) (>= 1.0.1).

# Build #
```
git clone https://code.google.com/p/qnfcd
cd qnfcd
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make
sudo make install
```

QNFCd use D-Bus and D-Bus needs policy file to allow NFCd to publish on system bus:
```
sudo cp nfcd.conf /etc/dbus-1/system.d/
```

# Run #
Once these commands succeed, just type:
```
sudo nfcd
```

Note: ATM, QNFCd provide D-Bus interface on system bus.

# Debug #
You need to activate special flags in order to debug, to do this you simply need to activate "Debug" target using CMake
```
cmake .. -DCMAKE_BUILD_TYPE=Debug
```
