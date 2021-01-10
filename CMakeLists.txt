project (dart LANGUAGES C CXX)
cmake_minimum_required (VERSION 3.1)

include (CTest)
enable_testing()

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wextra")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror")


add_subdirectory (cmake)
include (add_macros)
include (beautify_find_boost)
include (beautify_find_GPISpace)

find_GPISpace (REVISION 8f22a53d55539b5f205ccd5103f04fdf05859714)

find_boost (1.61 REQUIRED COMPONENTS unit_test_framework test_exec_monitor)

find_package(OpenSSL REQUIRED)

#Note: works with soft/anaconda3/5.0.1
set(Python_ADDITIONAL_VERSIONS 3.6)

find_package (Threads REQUIRED)
find_package (PythonInterp)

if (PYTHONINTERP_FOUND)
  if (PYTHON_VERSION_MAJOR EQUAL 3)
    find_package(Boost COMPONENTS python${PYTHON_VERSION_SUFFIX})
    find_package(PythonInterp 3)
    find_package(PythonLibs 3 REQUIRED)
  else()
    find_package(Boost COMPONENTS python)
    find_package(PythonInterp)
    find_package(PythonLibs REQUIRED)
  endif()
else()
  message("Python not found")
endif()

message(STATUS "PYTHON_LIBRARIES = ${PYTHON_LIBRARIES}")
message(STATUS "PYTHON_EXECUTABLE = ${PYTHON_EXECUTABLE}")
message(STATUS "PYTHON_INCLUDE_DIRS = ${PYTHON_INCLUDE_DIRS}")

add_definitions( -DPYTHON_EXECUTABLE="${PYTHON_EXECUTABLE}" )
add_definitions( -DPYTHON_LIBRARIES="${PYTHON_LIBRARIES}" )

INCLUDE_DIRECTORIES (${PYTHON_INCLUDE_DIRS})

set (BUNDLE_ROOT "libexec/bundle")
bundle_GPISpace (DESTINATION "${BUNDLE_ROOT}/gpispace"
  COMPONENTS runtime 
)

find_program (CHRPATH_BINARY NAMES chrpath)
if (NOT CHRPATH_BINARY)
  message (FATAL_ERROR "Unable to find chrpath (CHRPATH_BINARY)")
endif()

add_custom_command (OUTPUT "${bundle_path}"
  COMMAND "${CMAKE_SOURCE_DIR}/cmake/bundle.sh"
          "${bundle_path}"
          "${CHRPATH_BINARY}"
  COMMAND "${CMAKE_COMMAND}"
          -E copy_if_different
  COMMAND "${CHRPATH_BINARY}" -r '\$\$ORIGIN/bundle'
  DEPENDS "${CMAKE_SOURCE_DIR}/cmake/bundle.sh"
)
add_custom_target (bundle-impl ALL
  DEPENDS "${bundle_path}"
)

include_directories ("${CMAKE_SOURCE_DIR}/src")

extended_add_library(NAME log
  NAMESPACE dart
  SOURCES "src/log/log.cpp"
  LIBRARIES Boost::system Threads::Threads
  POSITION_INDEPENDENT
)

extended_add_library(NAME net
  NAMESPACE dart
  SOURCES "src/net/io_pool.cpp"
  LIBRARIES Boost::system Threads::Threads dart::log
  POSITION_INDEPENDENT
)

extended_add_library(NAME http
  NAMESPACE dart
  SOURCES "src/http/client.cpp"
          "src/http/message.cpp"
          "src/http/message_builder.cpp"
          "src/http/server.cpp"
          "src/http/status_code.cpp"
  LIBRARIES Boost::system dart::net dart::log OpenSSL::SSL OpenSSL::Crypto
  POSITION_INDEPENDENT
)

extended_add_library(NAME rest
  NAMESPACE dart
  SOURCES "src/rest/message.cpp"
          "src/rest/router.cpp"
  LIBRARIES Boost::system dart::http dart::log
  POSITION_INDEPENDENT
)

#extended_add_library (NAME installation
#  NAMESPACE dart
#  SOURCES "src/installation.cpp"
#  LIBRARIES GPISpace::execution #Util::Generic
#  POSITION_INDEPENDENT
#)

# Note: Does _NOT_ track external dependencies!
add_custom_command (
  OUTPUT "${CMAKE_BINARY_DIR}/dart_workflow.pnet"
         "${CMAKE_BINARY_DIR}/gen/pnetc/op/libexecute_python_task.so"
         "${CMAKE_BINARY_DIR}/gen/pnetc/type/task_result/op.o"
         "${CMAKE_BINARY_DIR}/gen/pnetc/type/config/op.o"
         "${CMAKE_BINARY_DIR}/gen/pnetc/type/location_and_parameters/op.o"
  COMMAND "${GSPC_HOME}/bin/pnetc"
          --search-path="${GSPC_HOME}/share/gspc/xml"
          --search-path="${GSPC_HOME}/share/gspc/xml/lib"
          --gen-cxxflags=-O3
          --gen-cxxflags=-std=c++17
          --gen-cxxflags=-I"${CMAKE_SOURCE_DIR}/src"
          --gen-cxxflags=-I"${PYTHON_INCLUDE_DIRS}"
          --gen-ldflags="-ldl"
          --gen-ldflags="-Wl,--disable-new-dtags"
          --gen-ldflags="-Wl,-rpath='\\$\$\\$\$ORIGIN/../libexec/bundle/lib'"
          --gen-ldflags="-Wl,-rpath='\\$\$\\$\$ORIGIN/../libexec/bundle/gpispace/lib'"
          --force-overwrite-file=true
          --input="${CMAKE_SOURCE_DIR}/workflow/dart_workflow.xpnet"
          --output="${CMAKE_BINARY_DIR}/dart_workflow.pnet"
          --path-to-cpp="${CMAKE_BINARY_DIR}/gen"
          --Wbackup-file=OFF
          --Woverwrite-file=OFF
          &&
          make -C "${CMAKE_BINARY_DIR}/gen"
  DEPENDS "${CMAKE_SOURCE_DIR}/workflow/dart_workflow.xpnet"
         # Util::Generic
)

add_custom_target (dart_workflow.pnet_and_wrapper-library
  ALL
  DEPENDS "${CMAKE_BINARY_DIR}/gen/pnetc/op/libexecute_python_task.so"
          "${CMAKE_BINARY_DIR}/dart_workflow.pnet"
)

install (FILES
  "${CMAKE_BINARY_DIR}/dart_workflow.pnet"
  "${CMAKE_BINARY_DIR}/gen/pnetc/op/libexecute_python_task.so"
  DESTINATION workflow
)

extended_add_executable(NAME dart-server
  SOURCES "src/dart-server/main.cpp"
          "src/dart-server/dart_server.cpp"
          "src/dart-server/installation.cpp"
          "src/dart-server/gspc_interface.cpp"
          "src/dart-server/job_storages/ram_storage.cpp"
          "${CMAKE_BINARY_DIR}/gen/pnetc/type/task_result/op.o" 
          "${CMAKE_BINARY_DIR}/gen/pnetc/type/config/op.o"
          "${CMAKE_BINARY_DIR}/gen/pnetc/type/location_and_parameters/op.o"
  LIBRARIES Boost::system OpenSSL::SSL OpenSSL::Crypto 
            dart::http dart::rest GPISpace::execution
  INCLUDE_DIRECTORIES PUBLIC "${CMAKE_BINARY_DIR}/gen"
  INSTALL
)

 
#extended_add_library ( NAME DART
#  SOURCES src/cpp/dart_module.cpp
#          src/cpp/drts_interface.cpp
#          src/cpp/task_result.cpp
#          src/cpp/result_storages/ram_storage.cpp
#          src/cpp/result_storages/python_storage.cpp
#          "${CMAKE_BINARY_DIR}/gen/pnetc/type/task_result/op.o" 
#          "${CMAKE_BINARY_DIR}/gen/pnetc/type/config/op.o"
#          "${CMAKE_BINARY_DIR}/gen/pnetc/type/location_and_parameters/op.o"
#  TYPE PYTHON_MODULE
#  LIBRARIES #Util::Generic
#            dart::installation
#            GPISpace::execution
#            GPISpace::workflow_development
#            ${PYTHON_LIBRARIES}
#            Boost::python
#  DEPENDS dart_workflow.pnet_and_wrapper-library       
#  INCLUDE_DIRECTORIES PUBLIC "${CMAKE_BINARY_DIR}/gen"
#  INSTALL
#)
            
install (DIRECTORY
  "${CMAKE_SOURCE_DIR}/example"
  DESTINATION  .
)
install (DIRECTORY
  "${CMAKE_SOURCE_DIR}/worker"
  DESTINATION  .
)

install (DIRECTORY
  "${CMAKE_SOURCE_DIR}/certs"
  DESTINATION  .
)

install (DIRECTORY
  "${CMAKE_SOURCE_DIR}/python"
  DESTINATION .
)

install (DIRECTORY
  "${CMAKE_SOURCE_DIR}/bin"
  DESTINATION .
)

#install (FILES
#  "${CMAKE_SOURCE_DIR}/example/dart_task.py"
#  "${CMAKE_SOURCE_DIR}/src/python/dart.py"
#  DESTINATION lib
#)

#install (FILES
#  "${CMAKE_SOURCE_DIR}/utils/dart-stop-all"
#  "${CMAKE_SOURCE_DIR}/utils/dart-stop-all-simple-nodefile"
#  "${CMAKE_SOURCE_DIR}/utils/dart-stop-all-carme"
#  "${CMAKE_SOURCE_DIR}/utils/dart-create-nodefile-carme"
#  "${CMAKE_SOURCE_DIR}/utils/dart-create-nodefile-slurm"
#  "${CMAKE_SOURCE_DIR}/utils/dart-get-hostnames-slurm"
#  "${CMAKE_SOURCE_DIR}/utils/_ports.py"
#  "${CMAKE_SOURCE_DIR}/utils/_sshconfig.py"
#  DESTINATION bin
#  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
#              GROUP_READ GROUP_EXECUTE
#              WORLD_READ WORLD_EXECUTE
#)

add_subdirectory (test)
