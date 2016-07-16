from distutils.core import setup

setup(name='libAlazar',
      version='0.0.0',
      py_modules=['libAlazar'],
      data_files=[('/usr/local/lib', ['libAlazar.dylib'])])
