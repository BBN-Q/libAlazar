[libAlazar wiki](https://qiplab.bbn.com/BUQ-Lab/libAlazar/wikis/home)


# Build Dependencies:
* pacman -S mingw-w64-x86_64-boost
* http://www.mathworks.com/help/matlab/matlab_external/install-mingw-support-package.html
* http://www.mathworks.com/help/matlab/matlab_external/compiling-c-mex-files-with-mingw.html

# Build Instructions

##  On Windows:
```
mkdir build
cd build
cmake -G "MSYS Makefiles"  ..
make clean install
```
Uses default values for 
SIM=false and 
ALAZAR_INCLUDE_PATH=../AlazarTech/ATS-SDK/6.0.3/Samples/Include
Use -D option in cmake line to change from defaults.
For example:
cmake -G "MSYS Makefiles" -DSIM=true -DALAZAR_INCLUDE_PATH=<path to SDK inlcude file>

## OSX/Linux
Building with the Simulator in OSX ( not tested on Linux yet):
Don't use the -G option for cmake

## Library version
Uses the most recent git tag and SHA1.  Format is:

```
R<MAJOR>_<MINOR>-<SHA1>-<dirty[OPTIONAL]>
```
"dirty" indicates that the code was built from a branch with uncommitted code.


# Matlab Driver

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
Using version 5.10.6 of the dll.  Temporarily added it to the repo.  Build assumes
it is installed in C:\Windows\System32\ATSApi.dll

todo - add variable dll path to cmake

Assumes Alazar SDK directory is found ../AlazarTech/ATS-SDK relative to the libAlazar
project dir.

todo - add variable to specify path to sdk

Using v 6.0.3 of the Alazar SDK


