# python setup-win.py bdist --format=wininst

from distutils.core import setup, Extension
setup(name="mercuryapi", version="0.1",
      ext_modules=[Extension("mercury",
                             sources=["mercury.c"],
                             libraries=["mercuryapi"],
                             include_dirs=['mercuryapi-1.27.3.16/c/src/api',
                                           'mercuryapi-1.27.3.16/c/src/arch/win32/include'],
                             library_dirs=['mercuryapi-1.27.3.16/c/proj/MercuryAPI/Release'])],
      data_files = [('', ["mercuryapi-1.27.3.16/c/src/arch/win32/lib/pthreadVC2.dll"])])
