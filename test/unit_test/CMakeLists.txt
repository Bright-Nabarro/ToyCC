find_package(GTest REQUIRED)
if (NOT GTest_FOUND)
	message(FATAL_ERROR "gtest not found")
endif()

# unit test
add_subdirectory(langspec_test)
add_executable(unit_test "main.cpp")

target_link_libraries(unit_test PRIVATE
	langspec_test
)

ChgExeOutputDir(unit_test)

