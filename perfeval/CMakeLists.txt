message("\n*** perfeval/ subdir")

# gtest_SOURCE_DIR is available due to project(gtest)
include_directories(SYSTEM
	${gtest_SOURCE_DIR}/include
	${gtest_SOURCE_DIR}
    ../externals/memcached
)

# Getting the test files
set(EXAMPLES_FILES
    ../src/examples/out_test/memcached.cpp
    ../src/examples/out_test/memcached_inst.cpp
    ../externals/memcached/memcached_parser.cpp
)

# memcached evaluator target
set(MemcachedEvalTarget memcached_eval)
set(MEMCACHED_EVAL_FILES memcached_evaluator.cpp)

add_executable(${MemcachedEvalTarget} ${MEMCACHED_EVAL_FILES} ${EXAMPLES_FILES})

target_link_libraries(${MemcachedEvalTarget}
	${MyProjectLib}
    pantheios.1.core.gcc48
    pantheios.1.be.fprintf.gcc48
    pantheios.1.bec.fprintf.gcc48
    pantheios.1.fe.all.gcc48
    pantheios.1.core.gcc48
    pantheios.1.util.gcc48
	gtest
)

set_target_properties(${MemcachedEvalTarget} PROPERTIES COMPILE_FLAGS "${FLAGS_MY_PROJECT}")

if(CMAKE_COMPILER_IS_GNUC OR CMAKE_COMPILER_IS_GNUCXX)
	target_link_libraries(${MemcachedEvalTarget} pthread)
endif()
