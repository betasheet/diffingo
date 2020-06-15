cmake_minimum_required(VERSION 3.1.0 FATAL_ERROR)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

find_package(Boost 1.40 COMPONENTS filesystem regex REQUIRED)

include_directories(BEFORE ${CMAKE_SOURCE_DIR} ${CMAKE_BINARY_DIR})
link_directories(${Boost_LIBRARY_DIR} ${CMAKE_BINARY_DIR}/lib)

add_subdirectory( tests )

set(kode_LIB_SRCS
   code.cpp
   enum.cpp
   style.cpp
   printer.cpp
   license.cpp
   file.cpp
   class.cpp
   function.cpp
   variable.cpp
   membervariable.cpp
   typedef.cpp
   statemachine.cpp
   automakefile.cpp )

add_library( kode SHARED ${kode_LIB_SRCS} )
target_link_libraries( kode ${Boost_LIBRARIES} )

file(GLOB header_files RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.h)
install(FILES ${header_files} DESTINATION include/kode)
