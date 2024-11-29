from setuptools import setup, Extension

module = Extension('system_resource_extension',
                   sources=['system_resource_extension.c'])

setup(name='SystemResourceExtension',
      version='1.0',
      description='CPython extension for reading system resource metrics',
      ext_modules=[module])