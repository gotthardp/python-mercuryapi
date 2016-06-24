# python3 setup.py build

from distutils.core import setup, Extension
setup(name="mercuryapi", version="0.1",
      ext_modules=[Extension("mercury", libraries=["mercuryapi", "ltkc", "ltkctm"], sources=["mercury.c"])])
