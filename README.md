[libAlazar wiki](https://qiplab.bbn.com/BUQ-Lab/libAlazar/wikis/home)

@todo
add stuff

dependencies:
pacman -S mingw-w64-x86_64-boost
http://www.mathworks.com/help/matlab/matlab_external/install-mingw-support-package.html
http://www.mathworks.com/help/matlab/matlab_external/compiling-c-mex-files-with-mingw.html

On MAC 
need to export the path to the relocatable shared library
export DYLD_LIBRARY_PATH=/Users/rmcgurrin/sandbox/q/libAlazar/build/lib

On Windows
Windows matlabSet variable using Windows Control Panel

To set the environment variable on Windows 7:

* Make sure you have administrative privileges.
* Select Computer from the Start menu.
* Choose System properties from the context menu.
* Click Advanced system settings > Advanced tab.
* Click Environment Variables.
* Under System variables, select New.
* In the New System Variable dialog box, type MW_MINGW64_LOC in the Variable name field.
* In the Variable value field, type the location of the MinGW-w64 compiler installation, for example, C:\TDM-GCC-64.
* Click Ok to close the dialog boxes, then close the Control Panel dialog box.

