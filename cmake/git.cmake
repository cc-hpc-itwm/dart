function (determine_git_revision directory output_variable_name)
  set (revision_file "${directory}/revision")
  if (EXISTS "${revision_file}")
    file (READ "${revision_file}" output)
    string (LENGTH "${output}" revision_length)
    if (NOT revision_length EQUAL 40)
      message (FATAL_ERROR "Found invalid revision '${output}' in file '${revision_file}'. Delete or update it!")
    endif()
    message (STATUS "Using Git revision '${output}' from file '${revision_file}'. Delete it to force getting it from the git repository.")
  else()
    find_package (Git REQUIRED)

    set (command "${GIT_EXECUTABLE}" rev-parse HEAD)
    execute_process (COMMAND ${command}
      WORKING_DIRECTORY "${directory}"
      OUTPUT_VARIABLE output OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_VARIABLE output ERROR_STRIP_TRAILING_WHITESPACE
      RESULT_VARIABLE error_code)

    if (NOT ${error_code} EQUAL 0)
      string (REPLACE ";" " " command_string "${command}")
      message (FATAL_ERROR "could not discover revision info for '${directory}': "
        "`${command_string}` failed with error code ${error_code}. "
        "output: ${output}")
    endif()
  endif()

  set (${output_variable_name} "${output}" PARENT_SCOPE)
endfunction()
