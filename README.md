# libAlazar

This library wraps the AlazarTech ATS9870 shared library in a simplified
interface appropriate for multi-segment edge-triggered experiments. It also
provides averaging capability around inner (waveforms) or outer (round robins)
loops. Data is passed to the user application through a socket interface,
which is capability of very large bandwidth and is compatible with many
event interfaces through OS file/socket watchers.

# Build Status

[![master status](https://qiplab.bbn.com/ci/projects/4/status.png?ref=master)](https://qiplab.bbn.com/ci/projects/4?ref=master)


# Build Instructions
_____________

## ATS9870 DLL and SDK

Building libAlazar requires a license and copy of the AlazarTech ATS-SDK. For
local users (at BBN), the SDK header files will be downloaded automatically by
the build script. For external users, you must provide cmake a path where it can
find the ATS-SDK header files via `-DALAZAR_INCLUDE_PATH=/path/to/AlazarApi.h`.

We provide an ATS API simulator for testing on machines that do not have an
ATS9870 installed. Build the simulator and link libAlazar to it by passing
`-DSIM=true` to cmake. To build a production version of libAlazar, the AtsApi
shared library must be on the path.

Other notes:
* Tested using version 6.0.3 of the Alazar ATS-SDK
* Tested using version 5.10.6 of the Alazar ATS9870 DLL.
* Build assumes the AST9870 shared library is available on the path (e.g. it is installed in C:\Windows\System32\)

## Windows

Internally, we build libAlazar for windows in an msys2 environment.

### Dependencies:

* pacman -S make
* pacman -S mingw64/mingw-w64-x86_64-cmake
* pacman -S mingw64/mingw-w64-x86_64-gcc

### Instructions:

```
git submodule update --init
mkdir build
cd build
cmake -G "MSYS Makefiles"  ../
make install
```
NOTE:

1. Uses default values for SIM=false and LOG_LEVEL=2 (INFO)
2. Use -D option in cmake line to change from defaults.

    For example:
    ```
    cmake -G "MSYS Makefiles" -DSIM=true -DLOG_LEVEL=3 ../
    ```

## Ubuntu 14.04.1 LTS

### Dependencies:

Install cmake
```
sudo apt-get install cmake
```
Download and install [Anaconda](https://repo.continuum.io/archive/Anaconda3-4.2.0-Linux-x86_64.sh)
```
bash Anaconda3-4.2.0-Linux-x86_64.sh
```

### Instructions

Use cmake to build - the ATS simulator is used in Linux for CI.
```
git submodule update --init
mkdir build
cd build
cmake -DSIM=true ../
make install
./bin/unittest
cd ../src/python
python test_alazar_driver.py
```

### OSX

The process is very similar to linux.

____________

## Library version

Uses the most recent git tag and SHA1.  Format is:

```
v<MAJOR>.<MINOR>.<PATCH>-<commits since tag>-<git sha1>[-<dirty>]
```
If present, "dirty" indicates that the code was built from a branch with uncommitted code.


# Matlab Driver
____________________

## Windows 7/10

The TDM-GCC-64 Compiler can be used to build the thunk file.  This can be added as a Matlab package:

* http://www.mathworks.com/help/matlab/matlab_external/install-mingw-support-package.html

Set the MW_MINGW64_LOC environment variable to point to your TDM-GCC-64 or
mingw-w64 gcc from msys2:
* http://www.mathworks.com/help/matlab/matlab_external/compiling-c-mex-files-with-mingw.html

##  macOS

MATLAB works with the XCode command line tools. You should be prompted to install the required components if you simply execute `clang` at a shell.

## Rebuilding the thunk File

```
>> cd src/matlab
>> [~,~]=loadlibrary('libAlazar.dll','libAlazarAPI.h','mfilename','libAlazar_pcwin64.m');
```
Then commit
* libAlazar_pcwin64.m
* libAlazar_thunk_pcwin64.dll
* loadLibAlazar.m
