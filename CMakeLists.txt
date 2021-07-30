#This needs to be included before everything.
include(${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.conan.txt)

cmake_minimum_required(VERSION 3.8.0)

function(make_pretty_source_folders BASE_PATH PREFIX_PATH REQUIRED_ARG)
	foreach(SOURCE IN ITEMS ${REQUIRED_ARG})
		get_filename_component(SOURCE_PATH "${SOURCE}" PATH)
		# message("pretty source_path : ${SOURCE_PATH}")
		file(RELATIVE_PATH SOURCE_PATH_REL "${CMAKE_CURRENT_SOURCE_DIR}/${BASE_PATH}" "${SOURCE_PATH}")
		# message("pretty source_path_rel : ${SOURCE_PATH_REL}")
		string(REPLACE "/" "\\" GROUP_PATH "${PREFIX_PATH}/${SOURCE_PATH_REL}")
		# message("pretty group_path : ${GROUP_PATH}")
		source_group("${GROUP_PATH}" FILES "${SOURCE}")
	endforeach()
endfunction()

# Compile options.
function(set_fst_compile_options REQUIRED_ARG EXPOSURE)
	if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
		target_compile_options(${REQUIRED_ARG} ${EXPOSURE} -Wall -Wextra -Wpedantic -Wno-gnu-zero-variadic-macro-arguments)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		target_compile_options(${REQUIRED_ARG} ${EXPOSURE} -Wall -Wextra -Wpedantic -Wno-ignored-qualifiers)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
		# using Intel C++
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		target_compile_definitions(${REQUIRED_ARG} ${EXPOSURE} _SILENCE_CXX17_UNCAUGHT_EXCEPTION_DEPRECATION_WARNING)
		target_compile_options(${REQUIRED_ARG} ${EXPOSURE} /Zc:__cplusplus /Zc:alignedNew /permissive- /W4 /utf-8 )
	endif()
endfunction()

#Conan search paths.
set(CMAKE_MODULE_PATH ${CMAKE_BINARY_DIR} ${CMAKE_MODULE_PATH})
set(CMAKE_PREFIX_PATH ${CMAKE_BINARY_DIR} ${CMAKE_PREFIX_PATH})

set(CMAKE_XCODE_GENERATE_SCHEME ON)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(fst VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_EXTENSIONS OFF)

#User options
option(FST_BUILD_TESTS "Build and run tests." ON) 
option(FST_BUILD_BENCH "Build and run benchmarks." OFF)
option(FST_BUILD_DEV "Build dev." OFF)

set(FST_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(FST_SOURCE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(FST_EXTERN_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/extern)

#All sources.
file(GLOB_RECURSE FST_HEADER_FILES "${FST_INCLUDE_DIRECTORY}/*.h")
file(GLOB_RECURSE FST_SOURCE_FILES "${FST_SOURCE_DIRECTORY}/*.cpp")

#File groups.
make_pretty_source_folders("include/fst" "include" "${FST_HEADER_FILES}")
make_pretty_source_folders("src" "src" "${FST_SOURCE_FILES}")

# -Wall
# -Wpedantic
# -pedantic

add_subdirectory(${FST_EXTERN_DIRECTORY}/zlib)
add_subdirectory(${FST_EXTERN_DIRECTORY}/libzip)

set_target_properties(zlib PROPERTIES FOLDER fst/extern)
set_target_properties(zlibstatic PROPERTIES FOLDER fst/extern)
set_target_properties(distcheck PROPERTIES FOLDER fst/extern)
set_target_properties(dist PROPERTIES FOLDER fst/extern)
set_target_properties(zip PROPERTIES FOLDER fst/extern)

add_library(${PROJECT_NAME} STATIC ${FST_HEADER_FILES} ${FST_SOURCE_FILES})
set_fst_compile_options(${PROJECT_NAME} PRIVATE)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${FST_INCLUDE_DIRECTORY}
  ${FST_EXTERN_DIRECTORY}/libzip/lib
  ${CMAKE_CURRENT_BINARY_DIR}/extern/libzip/
)

target_link_libraries(${PROJECT_NAME} PUBLIC
  zlibstatic
  zip
)

#Tests
if (${FST_BUILD_TESTS})
    find_package(GTest CONFIG REQUIRED)

    #Test Project
    set(FST_TEST_NAME ${PROJECT_NAME}_tests)
    file(GLOB_RECURSE FST_TEST_SOURCES "tests/*.cpp" "tests/*.h")

    add_executable(${FST_TEST_NAME} ${FST_TEST_SOURCES})

    set_fst_compile_options(${FST_TEST_NAME} PRIVATE)

    target_link_libraries(${FST_TEST_NAME} PUBLIC
        GTest::gtest
        fst
    )

    target_include_directories(${FST_TEST_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/tests)

    set(FST_TEST_RESOURCES_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests/resources")
    add_definitions(-DFST_TEST_RESOURCES_DIRECTORY="${FST_TEST_RESOURCES_DIRECTORY}")
endif()

#Benchmarks.
if (${FST_BUILD_BENCH})
    find_package(benchmark CONFIG REQUIRED)

    #Test Project
    set(FST_BENCH_NAME ${PROJECT_NAME}_benchs)
    file(GLOB_RECURSE FST_BENCH_SOURCES "bench/*.cpp")

    add_executable(${FST_BENCH_NAME} ${FST_BENCH_SOURCES})
    set_fst_compile_options(${FST_BENCH_NAME} PRIVATE)

    set_target_properties(${FST_BENCH_NAME} PROPERTIES
        # CXX_STANDARD 17
        # CXX_EXTENSIONS OFF
        # LINKER_LANGUAGE CXX
        CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}"
        # FOLDER dev/
    )

    target_compile_definitions(${FST_BENCH_NAME} PUBLIC
        NDEBUG
    )

    target_link_libraries(${FST_BENCH_NAME} PUBLIC
        fst
        benchmark::benchmark
    )
endif()

if (${FST_BUILD_DEV})
    # find_package(dev CONFIG REQUIRED)

    #Test Project
    set(FST_DEV_NAME ${PROJECT_NAME}_dev)
    file(GLOB_RECURSE FST_DEV_INCLUDE "dev/*.h")
    file(GLOB_RECURSE FST_DEV_SOURCES "dev/*.cpp")

    add_executable(${FST_DEV_NAME} ${FST_DEV_INCLUDE} ${FST_DEV_SOURCES})

    # set_target_properties(${FST_DEV_NAME} PROPERTIES
    #     # CXX_STANDARD 17
    #     # CXX_EXTENSIONS OFF
    #     # LINKER_LANGUAGE CXX
    #     # CMAKE_CXX_FLAGS "-O3 ${CMAKE_CXX_FLAGS}"
    #     # FOLDER dev/
    # )

    target_compile_definitions(${FST_DEV_NAME} PUBLIC
        # NDEBUG
    )

    target_link_libraries(${FST_DEV_NAME} PUBLIC
        fst
        # benchmark::benchmark
    )
endif()