# hints libssh2 pkgconfig dir

# library libssh2::libssh2


set (_pkgname libssh2)
set (_libnamespace ${_pkgname})

set (_libname ${_pkgname})

if (TARGET ${_libnamespace}::${_libname})
  return()
endif()

include (detail/pkgconfig_helper)

_pkgconfig_find_library_module (${_pkgname} ${_libnamespace} ${_libname})







_pkgconfig_find_library_module_finalize()
