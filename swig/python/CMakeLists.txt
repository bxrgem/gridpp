find_package(Python3 REQUIRED COMPONENTS Interpreter Development NumPy)
include_directories(${Python3_INCLUDE_DIRS})

if(SWIG_VERSION GREATER_EQUAL 4)
    set(CMAKE_SWIG_FLAGS "-doxygen")
    # Note: -keyword cannot be used with overload functions, so it is a bit pointless to use here
    # set(CMAKE_SWIG_FLAGS "-keyword")
endif()
set_source_files_properties(../${PROJECT_NAME}.i PROPERTIES CPLUSPLUS ON)
include_directories(../../include)

if(NOT Python3_NumPy_FOUND)
    message(FATAL_ERROR "NumPy not found")
endif()

# Include Numpy Include directory
include_directories(${Python3_NumPy_INCLUDE_DIRS})

# Add swig module
swig_add_library(gridpp_swig TYPE SHARED LANGUAGE python SOURCES ../${PROJECT_NAME}.i)
swig_link_libraries(gridpp_swig gridpp ${Python3_LIBRARIES})

if(APPLE)
    set_property(TARGET gridpp_swig PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)
    set_target_properties(gridpp_swig PROPERTIES
       SUFFIX ".so"
       INSTALL_RPATH"@loader_path;@loader_path/../../${PROJECT_NAME}/.libs"
       )
     set_property(TARGET gridpp_swig APPEND PROPERTY
       LINK_FLAGS "-flat_namespace -undefined suppress"
       )
endif()



# Configure setup.py and copy to output directory
set(SETUP_PY_IN ${CMAKE_CURRENT_SOURCE_DIR}/setup.in.py)
set(SETUP_PY_OUT ${CMAKE_CURRENT_BINARY_DIR}/setup.py)
configure_file(${SETUP_PY_IN} ${SETUP_PY_OUT})


# Install target to call setup.py
add_custom_target(install-python
        DEPENDS gridpp_swig
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY_OUT} install)

add_custom_target(install-python-user
        DEPENDS gridpp_swig
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY_OUT} install --user)

add_custom_target(build-python
        DEPENDS gridpp_swig
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY_OUT} build)

add_custom_target(develop-python
        DEPENDS gridpp_swig
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY_OUT} develop)

add_custom_target(develop-python-user
        DEPENDS gridpp_swig
        COMMAND ${PYTHON_EXECUTABLE} ${SETUP_PY_OUT} develop --user)

if (ENABLE_TESTS)
    # Copy the test programs to be next to the gridpp.py package
    add_custom_target(tests-data
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/tests
        ${CMAKE_CURRENT_BINARY_DIR}/tests)

    add_custom_target(tests COMMAND ${PYTHON_EXECUTABLE} -m coverage run -m unittest discover)
    add_dependencies(tests tests-data build-python)
endif()
