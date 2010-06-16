from distribute_setup import use_setuptools
use_setuptools()

from setuptools import setup, find_packages, Extension
setup(name='Mumoro',
      version='0.0.1b',
      author= 'Tristram Graebener',
      author_email = 'tristramg@gmail.com',
      url = 'http://github.com/Tristramg/mumoro/',
      description = 'Multimodal and multiobjective routing',
      licence = 'GPLv3',
      packages = ['lib', 'lib.core', 'web'],

      install_requires = ['cherrypy', 'genshi', 'simplejson', 'transitfeed', 'setuptools-git', 'osm4routing'],
      ext_modules = [
          Extension("lib.core._mumoro",
              sources=["lib/core/martins.cpp", "lib/core/graph_wrapper.cpp", "lib/core/mumoro.i"],
              swig_opts=['-c++'],
              include_dirs=['lib/core/'])
          ]

      )

