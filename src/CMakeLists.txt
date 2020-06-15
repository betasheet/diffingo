message("\n*** src/ subdir")

set(autogen "${CMAKE_CURRENT_BINARY_DIR}/autogen")

###################################################################
# Generate Bison/Flex files

file(MAKE_DIRECTORY ${autogen}/spec/parser/)

bison_target(Parser spec/parser/parser.yy
             ${autogen}/spec/parser/parser.cc
             HEADER ${autogen}/spec/parser/parser.h
             VERBOSE ${autogen}/spec/parser/parser.output
             COMPILE_FLAGS "${BISON_FLAGS}")

flex_target(Scanner spec/parser/scanner.ll ${autogen}/spec/parser/scanner.cc)

add_custom_target(generate_parser
                  DEPENDS ${autogen}/spec/parser/parser.cc ${autogen}/spec/parser/scanner.cc)

###################################################################
message(AUTHOR_WARNING "Getting source files by using glob")
file(GLOB_RECURSE MyCppSources RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *.cpp *.cc)
string(REGEX REPLACE "CMakeFiles/[^;]+;?" "" MyCppSources "${MyCppSources}")
string(REGEX REPLACE "examples/[^;]+;?" "" MyCppSources "${MyCppSources}")

# Create a list with all cpp sources except for main, so the library doesn't contain it
set(MySourcesNoMain ${MyCppSources})
list(REMOVE_ITEM MySourcesNoMain main.cpp)

set(ExecTarget ${MyProjectName}_exec)

###################################################################
# Adding warning and standard flags here so we don't get an insane amount of warnings from
# externals, just from our project
if(MY_SHARED_LIB_ENABLED)
	add_library(${MyProjectLib} SHARED ${MySourcesNoMain})
else()
	add_library(${MyProjectLib} STATIC ${MySourcesNoMain})
endif()
set_target_properties(${MyProjectLib} PROPERTIES COMPILE_FLAGS "${FLAGS_MY_PROJECT}")

###################################################################
# Adding warning and standard flags here so we don't get an insane amount of warnings from
# externals, just from our project
add_executable(${ExecTarget} ${MyCppSources})
set_target_properties(${ExecTarget} PROPERTIES COMPILE_FLAGS "${FLAGS_MY_PROJECT}")
add_dependencies(${ExecTarget} generate_parser)

target_link_libraries(${ExecTarget}
                      pantheios.1.core.gcc48
                      pantheios.1.be.fprintf.gcc48
                      pantheios.1.bec.fprintf.gcc48
                      pantheios.1.fe.all.gcc48
                      pantheios.1.core.gcc48
                      pantheios.1.util.gcc48
                      ${Boost_LIBRARIES}
                      kode)

###################################################################

# Files that are packed with CPack
install(TARGETS ${ExecTarget}
	RUNTIME DESTINATION bin
	COMPONENT executables
)

if(MY_SHARED_LIB_ENABLED)
	install(TARGETS ${MyProjectLib}
		LIBRARY DESTINATION bin
		COMPONENT executables
	)
endif()

# install(DIRECTORY ${PROJECT_SOURCE_DIR}/src
# 	DESTINATION .
# 	COMPONENT headers
# )

# Licensing and stuff like that
install(FILES
	${ReadmeFile}
	${LicenseFile}
	DESTINATION .
)
