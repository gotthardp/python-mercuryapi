# python3 setup.py build
# python3 setup.py sdist upload
from setuptools import setup, Extension
setup(name="python-mercuryapi", version="0.4.1",
      author="Petr Gotthard",
      author_email="petr.gotthard@centrum.cz",
      description="Python wrapper for the ThingMagic Mercury API",
      long_description=open('README.md').read(),
      long_description_content_type="text/markdown",
      url="https://github.com/gotthardp/python-mercuryapi",
      classifiers=[
        "Programming Language :: Python :: 2",
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Development Status :: 4 - Beta"
      ],
      ext_modules=[Extension("mercury",
                             sources=["mercury.c"],
                             libraries=["mercuryapi", "ltkc", "ltkctm"],
                             include_dirs=['build/mercuryapi/include'],
                             library_dirs=['build/mercuryapi/lib'])])
