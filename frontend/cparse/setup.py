from distutils.core import setup, Extension

cparse_module = Extension('cparse',
	include_dirs = ['./../../include'],
	extra_compile_args = ["-std=c++17"],
	sources = ['codeparse.cpp'])

setup (name = 'cparse',
       version = '1.0',
       description = 'Helper functions for code parsing.',
       ext_modules = [cparse_module])

