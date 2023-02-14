# python3 setup.py build
# python3 setup.py sdist upload
from setuptools import setup, Extension
from distutils.command.build import build
import os, platform

class my_build(build):
    def run(self):
        if platform.system() == 'Darwin':
          os.system("make mercuryapi PLATFORM=posix")
        else:
          os.system("make mercuryapi PLATFORM=linux")
        build.run(self)

setup(name="python-mercuryapi", version="0.5.4",
      author="Petr Gotthard",
      author_email="petr.gotthard@centrum.cz",
      description="Python wrapper for the ThingMagic Mercury API",
      long_description=open('long_description.txt').read(),
      url="https://github.com/gotthardp/python-mercuryapi",
      classifiers=[
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Development Status :: 4 - Beta"
      ],
      cmdclass={'build': my_build},
      ext_modules=[Extension("mercury",
                             sources=["mercury.c"],
                             libraries=["mercuryapi", "ltkc", "ltkctm"],
                             include_dirs=['mercuryapi/include'],
                             library_dirs=['mercuryapi/lib'])])
