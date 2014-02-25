"""
Prepare python bindings package for libencloud.
"""
from distutils.core import setup, Extension

setup(
        name = 'encloud',
        version = '0.1',
        description = 'Bindings for libencloud',
        author = 'Steven Dorigotti',
        author_email = 's.dorigotti@endian.com',
        license = 'Proprietary',
        ext_modules = [Extension(
            '_encloud',
            sources = [ 'encloud.i' ],
            include_dirs = [ '../include' ],
            libraries = [ 'encloud' ],
            library_dirs = [ '../src' ]
            )],
        py_modules = ['encloud']
    )
