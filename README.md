[libAlazar wiki](https://qiplab.bbn.com/BUQ-Lab/libAlazar/wikis/home)

# Build Status
______________
[![develop status](https://qiplab.bbn.com/ci/projects/4/status.png?ref=develop)](https://qiplab.bbn.com/ci/projects/4?ref=develop)
[![rjmwork status](https://qiplab.bbn.com/ci/projects/4/status.png?ref=rjmwork)](https://qiplab.bbn.com/ci/projects/4?ref=rjmwork)


# Build Instructions
_____________

## Windows
Assumes msys2 environment is installed with cmake

### Dependencies:
* pacman -S make
* pacman -S mingw64/mingw-w64-x86_64-cmake
* pacman -S mingw64/mingw-w64-x86_64-gcc
* pacman -S mingw64/mingw-w64-x86_64-gdb
* pacman -S mingw64/mingw-w64-x86_64-hdf5
* pacman -S mingw-w64-x86_64-boost

### Instructions:
```
mkdir build
cd build
../src/lib/version.h > ../src/lib/version.h # only needed to update the version string
cmake -G "MSYS Makefiles"  ..
make clean install
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
Install Boost Libraries and cmake
```
sudo apt-get install cmake
sudo apt-get install libboost-all-dev
```
Download and install [Anaconda](https://3230d63b5fc54e62148e-c95ac804525aac4b6dba79b00b39d1d3.ssl.cf1.rackcdn.com/Anaconda2-2.5.0-Linux-x86_64.sh)
```
bash Anaconda2-2.5.0-Linux-x86_64.sh
```

### Instructions
LD_LIBRARY_PATH needs to be set so the unit tests can load the shared library from a non-standard location.  Alternatively this could be added to ~/.bashrc.
```
export LD_LIBRARY_PATH=.:./bin
```

Use cmake to build - the ATS simulator is used in Linux for CI.  Also, if building a release,
running version.sh creates version.h based on the most recent git tag.
```
git submodule update --init
mkdir build
cd build
cmake -DSIM=true ../
sh ../src/lib/version.sh > ../src/lib/version.h
make clean install
./bin/unittest
cd ../src/python
python apiTest.py
```

### OSX


____________

## Library version
Uses the most recent git tag and SHA1.  Format is:

```
R<MAJOR>_<MINOR>-<Commits Since Tag>-<git sha1>[-<dirty>]
```
If present, "dirty" indicates that the code was built from a branch with uncommitted code.


# Matlab Driver
____________________

## Matlab Dependencies for Rebuilding a thunk file
The TDM-GCC-64 Compiler is required to build the thunk file.  This Can be added as a Matlab package:

* http://www.mathworks.com/help/matlab/matlab_external/install-mingw-support-package.html

## Windows 7
Set the MW_MINGW64_LOC environment variable on Windows 7:
* http://www.mathworks.com/help/matlab/matlab_external/compiling-c-mex-files-with-mingw.html

##  OSx
Follow the instructions above to install TDM-GCC-64.

Export the path to the relocatable shared library:
* export DYLD_LIBRARY_PATH=/Users/rmcgurrin/sandbox/q/libAlazar/build/lib

## Rebuilding the thunk File
```
>> cd src/matlab
>> [~,~]=loadlibrary('libAlazar.dll','libAlazarAPI.h','mfilename','libAlazar_pcwin64.m');
```
Then commit
* libAlazar_pcwin64.m
* libAlazar_thunk_pcwin64.dll
* loadLibAlazar.m


# ATS9870 DLL ad SDK
______________________

* Using version 5.10.6 of the dll.  
* Build assumes
it is installed in C:\Windows\System32\ATSApi.dll
* Using v 6.0.3 of the Alazar SDK
