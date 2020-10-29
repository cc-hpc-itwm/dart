from setuptools import setup, Distribution, find_packages
from distutils.core import setup, Extension

with open("README.md", "r") as fh:
    long_description = fh.read()

class BinaryDistribution(Distribution):
    """Distribution which always forces a binary package with platform name"""
    def has_ext_modules(foo):
        return True

extension_module = Extension(
    'dart',
     ['dart/DART.so']
)

setup(
    name="dart", 
    version="0.0.1",
    author="Jens Krueger",
    author_email="jens.krueger@itwm.fraunhofer.de",
    description="A small DART example package",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/cc-hpc-itwm/dart",
    packages=find_packages(),
	package_data={
        "dart": ["DART.so"]
    },
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: GNU General Public License v3 (GPLv3)",
        "Operating System :: OS Independent",
    ],
    # data_files=[('dart', ['lib/DART.so'])],
    # ext_package='',
    # ext_modules = [extension_module],
    python_requires='>=3.6',
	include_package_data=True,
	distclass=BinaryDistribution,
)
