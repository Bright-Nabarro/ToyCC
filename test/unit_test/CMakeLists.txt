# unit test
include_directories(.)
add_subdirectory(langspec_test)
add_subdirectory(backend_test)
add_executable(unit_test "main.cpp")

target_link_libraries(unit_test PRIVATE
	langspec_test
	#	backend_test
)

add_test(
	NAME unit_test
	COMMAND bin/unit_test
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)

ChgExeOutputDir(unit_test)

