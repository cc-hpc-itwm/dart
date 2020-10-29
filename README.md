How to use DART:

1) install dart (see section Install)

2) create a nodefile to list all available computing resources e.g. using 
   the scripts installed in $DART_HOME/bin

3) when necessary, use force stopping the runtime manually,
   using the script stop_dart.sh installed in $DART_HOME/bin.
   The script requires as argument the absolute path to the nodefile

**Install DART**

```
cd pip-package

python -m pip install . --no-deps --ignore-installed -vv
```

**Build DART**

Requirements:

GPI-Space v20.09.1 (https://github.com/cc-hpc-itwm/gpispace.git)
anaconda3
boost 1.6+
cmake 3.*+
chrpath 0.16+
c++ compiler with c++17 support

How to build:
```
export GSPC_HOME=<path to GPI-Space install dir>
export DART_HOME=<DART install dir>

cd $DART_HOME/../build

cmake .. \
       -DPYTHON_EXECUTABLE=$(which python3) \
       -DPYTHON_INCLUDE_DIR=$(python -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())")  \
       -DPYTHON_LIBRARY=$(python -c "import distutils.sysconfig as sysconfig; import os; print(os.path.join(sysconfig.get_config_var('LIBDIR'), sysconfig.get_config_var('LDLIBRARY')))") \
       -DCMAKE_INSTALL_PREFIX=$DART_HOME \
       -DCMAKE_BUILD_TYPE=Release \
       -DGSPC_HOME=$GSPC_HOME \
       -DALLOW_ANY_GPISPACE_VERSION=TRUE \
       -DPYTHON_VERSION=3 

make install -j 10
```
**Examples**

find examples in DART_HOME/example
