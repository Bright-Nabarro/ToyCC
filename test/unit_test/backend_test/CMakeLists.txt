file(GLOB src "*.cpp")

add_library(backend_test OBJECT ${src})
target_link_libraries(backend_test PUBLIC
  	GTest::gmock
	GTest::gtest
	backend
)

