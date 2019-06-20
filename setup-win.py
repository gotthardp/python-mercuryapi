# python setup-win.py bdist_wininst -p win32
# python setup-win.py bdist_wininst -p amd64

from glob import glob;
from setuptools import setup, find_packages
from setuptools.extension import Extension

# you need to download https://www.jadaktech.com/documentation/rfid/mercuryapi
# https://www.jadaktech.com/wp-content/uploads/2018/11/mercuryapi-1.31.2.zip
# FIXME: make sure the patch matches the mercuryapi version you have
mercuryapi = "mercuryapi-1.31.2.40/c/src/"

setup(name="mercuryapi", version="0.5.1",
      ext_modules=[Extension("mercury",
                             [mercuryapi + "api/hex_bytes.c",
                              mercuryapi + "api/llrp_reader.c",
                              mercuryapi + "api/llrp_reader_l3.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_array.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_connection.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_element.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_encdec.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_error.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_framedecode.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_frameencode.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_frameextract.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_genout.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_hdrfd.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_typeregistry.c",
                              mercuryapi + "api/ltkc_win32/src/ltkc_xmltextencode.c",
                              mercuryapi + "api/ltkc_win32/src/tm_ltkc_genout.c",
                              mercuryapi + "api/osdep_win32.c",
                              mercuryapi + "api/serial_reader.c",
                              mercuryapi + "api/serial_reader_l3.c",
                              mercuryapi + "api/serial_transport_tcp_win32.c",
                              mercuryapi + "api/serial_transport_win32.c",
                              mercuryapi + "api/tmr_loadsave_configuration.c",
                              mercuryapi + "api/tmr_param.c",
                              mercuryapi + "api/tmr_strerror.c",
                              mercuryapi + "api/tmr_utils.c",
                              mercuryapi + "api/tm_reader.c",
                              mercuryapi + "api/tm_reader_async.c",
                              "mercury.c"],
                             define_macros=[('WIN32', '1'),
                                            ('HAVE_STRUCT_TIMESPEC', None),
                                            ('_CRT_SECURE_NO_WARNINGS', None)],
                             include_dirs=[mercuryapi + 'api',
                                           mercuryapi + 'api/ltkc_win32/inc',
                                           mercuryapi + 'pthreads-win32/pthreads.2'],
                             libraries=["pthreadVC2"],
                             # FIXME: update to include the path to pthreadVC2.lib
                             library_dirs=[mercuryapi + 'pthreads-win32/x86']
                  )],
      # FIXME: update to include the path to pthreadVC2.dll
      data_files = [('', [mercuryapi + 'pthreads-win32/x86/pthreadVC2.dll'])])
