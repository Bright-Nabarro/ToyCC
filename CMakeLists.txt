cmake_minimum_required(VERSION 3.25)
project(tinyc VERSION 0.0.1)

option(DEBUG_MODE ON)
option(ENABLE_TEST OFF)

# llvm项目使用clang作为编译器
#set(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_COMPILER clang++)

execute_process(
    COMMAND ${CMAKE_CXX_COMPILER} --version
    OUTPUT_VARIABLE CLANG_VERSION_OUTPUT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
message(STATUS "Clang compiler version: ${CLANG_VERSION_OUTPUT}")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

if(DEBUG_MODE)
	set(CMAKE_BUILD_TYPE Debug)
else()
	set(CMAKE_BUILD_TYPE Release)
endif()

add_compile_options(
	-Wall
	-Wextra
	-Wuninitialized
	-fno-rtti
#	-stdlib=libc++
)

#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lc++ -lc++abi")

find_package(LLVM REQUIRED CONFIG)
message(STATUS "LLVM version: ${LLVM_PACKAGE_VERSION}")

include(FetchContent)
FetchContent_Declare(yq_easylog
	GIT_REPOSITORY https://github.com/Bright-Nabarro/easylog.git
	GIT_TAG main
)
FetchContent_MakeAvailable(yq_easylog)

get_target_property(easylog_type easylog TYPE)
message(STATUS "easylog type: ${easylog_type}")

#[[
# 	正式构建程序
#]]
add_subdirectory("src")

if (ENABLE_TEST)
	enable_testing()
	include(Utils)
	add_subdirectory("test")
endif()

add_custom_target(cp_compile_commands ALL
	COMMAND ${CMAKE_COMMAND} -E copy_if_different
			${CMAKE_BINARY_DIR}/compile_commands.json
			${CMAKE_SOURCE_DIR}/compile_commands.json
	DEPENDS ${CMAKE_BINARY_DIR}/compile_commands.json
)

