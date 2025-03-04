file(GLOB src "*.cpp")

add_library(langspec_test OBJECT ${src})
target_link_libraries(langspec_test PUBLIC
  	GTest::gmock
	GTest::gtest
	semantix
)

