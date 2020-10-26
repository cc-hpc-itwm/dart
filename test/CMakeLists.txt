include_directories ("${CMAKE_SOURCE_DIR}/src" 
                     "${Boost_INCLUDE_DIR}"
                     "${CMAKE_BINARY_DIR}/gen"
                    )
  
  
configure_file ( ${CMAKE_SOURCE_DIR}/test/exceptions.py
                 ${CMAKE_BINARY_DIR}/test/
                 COPYONLY
               )
               
configure_file ( ${CMAKE_SOURCE_DIR}/test/parsing_errors.py
                 ${CMAKE_BINARY_DIR}/test/
                 COPYONLY
               )               
      
add_unit_test (NAME errors_and_exceptions
  SOURCES "errors_and_exceptions.cpp"
  USE_BOOST
  LIBRARIES  GPISpace::workflow_development
             "-ldl"
             "${CMAKE_BINARY_DIR}/gen/pnetc/type/task_result/op.o" 
             "${CMAKE_BINARY_DIR}/gen/pnetc/type/config/op.o"
  DEPENDS dart_workflow.pnet_and_wrapper-library
)