include (parse_arguments)

function (find_GPISpace)
  set (options ALLOW_DIFFERENT_GIT_SUBMODULES)
  set (one_value_options REVISION)
  set (multi_value_options)
  set (required_options)
  _parse_arguments (FIND "${options}" "${one_value_options}" "${multi_value_options}" "${required_options}" ${ARGN})

  set (_components)

  if (FIND_ALLOW_DIFFERENT_GIT_SUBMODULES)
    list (APPEND _components ALLOW_DIFFERENT_GIT_SUBMODULES)
  endif()

  if (FIND_REVISION)
    list (APPEND _components "REVISION=${FIND_REVISION}")
  endif()

  find_package (GPISpace REQUIRED COMPONENTS ${_components})

  #! \note Since this is a funtion, all variables set by find_package
  #! are local. To behave like find_package (GPISpace) would have been
  #! called instead of this wrapper, re-export variables.
  set (GPISpace_FOUND ${GPISpace_FOUND} PARENT_SCOPE)
  set (GSPC_HOME ${GSPC_HOME} PARENT_SCOPE)
  set (GSPC_XPNET_XSD ${GSPC_XPNET_XSD} PARENT_SCOPE)
  set (GSPC_REVISION_FILE ${GSPC_REVISION_FILE} PARENT_SCOPE)
  set (GSPC_GIT_SUBMODULES_FILE ${GSPC_GIT_SUBMODULES_FILE} PARENT_SCOPE)
  set (GSPC_REVISION ${GSPC_REVISION} PARENT_SCOPE)
endfunction()
