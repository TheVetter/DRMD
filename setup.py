from distutils.core import setup, Extension

modlue = Extension("deviceModule", sources=["deviceModule"])

setup(name="Device Package",
      version = ".2",
      description="this is the package that works with the device directly",
      ext_modules = [modlue])