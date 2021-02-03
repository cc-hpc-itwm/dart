#!/bin/bash

export GSPC_HOME=/gspc-install/

export DART_HOME=/build/dart/
export DART_BUILD=/build/dart-build/
export DART_INSTALL=/dart/

mkdir $DART_INSTALL
mkdir $DART_BUILD
cd $DART_BUILD

cmake $DART_HOME \
       -DCMAKE_INSTALL_PREFIX=$DART_INSTALL \
       -DPYTHON_EXECUTABLE=$(which python) \
       -DPYTHON_INCLUDE_DIR=$(python -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())")  \
       -DPYTHON_LIBRARY="/usr/lib64/libpython2.6.so" \
       -DCMAKE_INSTALL_PREFIX=$DART_INSTALL \
       -DCMAKE_BUILD_TYPE=Release \
       -DGSPC_HOME=$GSPC_HOME \
       -DALLOW_ANY_GPISPACE_VERSION=TRUE
	   
make install

tar -czvf /dart.tar.gz $DART_INSTALL