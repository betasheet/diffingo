cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

##################################################

project(Diffingo CXX)

##################################################

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(MY_CMAKE_DIR "${CMAKE_SOURCE_DIR}/cmake/")
set(MY_CMAKE_INPUT "${MY_CMAKE_DIR}/input/")
set(CMAKE_MODULE_PATH "${MY_CMAKE_DIR}/modules/")

include(FindRequiredPackage)
FindRequiredPackage(FLEX)
FindRequiredPackage(BISON)
find_package(Boost 1.40 COMPONENTS program_options filesystem system REQUIRED)

include("${MY_CMAKE_DIR}/options.cmake")
include("${MY_CMAKE_DIR}/settings.cmake")
include("${MY_CMAKE_DIR}/cpp.cmake")
include("${MY_CMAKE_DIR}/cpack.cmake")
#include("${MY_CMAKE_DIR}/doxygen.cmake")

##################################################
# Build external libs

include(ExternalProject)

set(StlsoftAndVersion stlsoft-1.9.118)
set(StlsoftDirectory ${PROJECT_SOURCE_DIR}/${ExternalLibsDir}/${StlsoftAndVersion})

set(PantheiosAndVersion pantheios-1.0.1-beta214)
set(PantheiosDirectory ${ExternalLibsDir}/${PantheiosAndVersion})
ExternalProject_add(pantheios
    PREFIX ${PantheiosDirectory}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    SOURCE_DIR ${PantheiosDirectory}
    BINARY_DIR ${PantheiosDirectory}
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ./build.sh ${StlsoftDirectory}
    INSTALL_COMMAND ""
    )

set(LibkodeDirectory ${ExternalLibsDir}/libkode)
ExternalProject_add(libkode
    PREFIX ${LibkodeDirectory}
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    SOURCE_DIR ${LibkodeDirectory}
    BINARY_DIR ${LibkodeDirectory}
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    BUILD_COMMAND make
    BUILD_ALWAYS 1
    INSTALL_COMMAND make install
    )

set(TinyformatDirectory ${ExternalLibsDir}/tinyformat)
set(Utf8ProcDirectory ${ExternalLibsDir}/utf8proc)

include_directories(SYSTEM
    ${StlsoftDirectory}/include
    ${PantheiosDirectory}/include
    ${LibkodeDirectory}/include
    ${TinyformatDirectory}
    ${Utf8ProcDirectory}
    ${Boost_INCLUDE_DIRS})

##################################################

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/src ${CMAKE_BINARY_DIR}/src)
link_directories(${PantheiosDirectory}/lib ${Boost_LIBRARY_DIR} ${LibkodeDirectory}/lib)

add_subdirectory(src/)

if(MY_TESTS_ENABLED)
	add_subdirectory(test/)
    add_subdirectory(perfeval/)
endif()
