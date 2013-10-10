"""
Prepare python bindings package for libece.
"""
from distutils.core import setup, Extension

setup(
        name = 'ece',
        version = '0.1',
        description = 'Bindings for libece',
        author = 'Steven Dorigotti',
        author_email = 's.dorigotti@endian.com',
        license = 'Proprietary',
        ext_modules = [Extension(
            '_ece',
            sources = [ 'ece.i' ],
            include_dirs = [ '../include' ],
            libraries = [ 'ece' ],
            library_dirs = [ '../src' ]
            )],
        py_modules = ['ece']
    )
