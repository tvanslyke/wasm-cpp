from distutils.core import setup, Extension

leb128_module = Extension('leb128',
                    sources = ['leb128.c'])

setup (name = 'leb128',
       version = '1.0',
       description = 'Helper functions for leb128 decoding.',
       ext_modules = [leb128_module])

