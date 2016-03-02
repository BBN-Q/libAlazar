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
* pacman -S mingw-w64-x86_64-boost
* http://www.mathworks.com/help/matlab/matlab_external/install-mingw-support-package.html
* http://www.mathworks.com/help/matlab/matlab_external/compiling-c-mex-files-with-mingw.html

### Instructions:
```
mkdir build
cd build
cmake -G "MSYS Makefiles"  ..
make clean install
```
Uses default values for SIM=false and ALAZAR_INCLUDE_PATH=../AlazarTech/ATS-SDK/6.0.3/Samples/Include
Use -D option in cmake line to change from defaults.
For example:
cmake -G "MSYS Makefiles" -DSIM=true -DALAZAR_INCLUDE_PATH=<path to SDK inlcude file>

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
mkdir build
cd build
cmake -DSIM=true ../
sh ../src/lib/version.sh > ../src/lib/version.h
make clean install
./bin/run_tests
```

### OSX


____________

## Library version
Uses the most recent git tag and SHA1.  Format is:

```
R<MAJOR>_<MINOR>-<SHA1>-<dirty[OPTIONAL]>
```
"dirty" indicates that the code was built from a branch with uncommitted code.


# Matlab Driver
____________________

## Windows
Windows matlabSet variable using Windows Control Panel

##  On MAC 
need to export the path to the relocatable shared library
export DYLD_LIBRARY_PATH=/Users/rmcgurrin/sandbox/q/libAlazar/build/lib



Need TDM-GCC-64 Compiler to build the thunk file.  Can be added as a Matlab 
package


To set the MW_MINGW64_LOC environment variable on Windows 7:

* Make sure you have administrative privileges.
* Select Computer from the Start menu.
* Choose System properties from the context menu.
* Click Advanced system settings > Advanced tab.
* Click Environment Variables.
* Under System variables, select New.
* In the New System Variable dialog box, type MW_MINGW64_LOC in the Variable name field.
* In the Variable value field, type the location of the MinGW-w64 compiler installation, for example, C:\TDM-GCC-64.
* Click Ok to close the dialog boxes, then close the Control Panel dialog box.


# ATS9870 DLL ad SDK
______________________

Using version 5.10.6 of the dll.  Temporarily added it to the repo.  Build assumes
it is installed in C:\Windows\System32\ATSApi.dll

todo - add variable dll path to cmake

Assumes Alazar SDK directory is found ../AlazarTech/ATS-SDK relative to the libAlazar
project dir.

todo - add variable to specify path to sdk

Using v 6.0.3 of the Alazar SDK


