file(GLOB SRC "*.cpp")

find_package(GTest REQUIRED)
if (NOT GTest_FOUND)
	message(FATAL_ERROR "gtest not found")
endif()

# unit test
add_executable(unit_test ${SRC})

target_link_libraries(unit_test PRIVATE
	GTest::gmock
	GTest::gtest
)

ChgExeOutputDir(unit_test)

