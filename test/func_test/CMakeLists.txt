set(test_dirs
	"return"
	"arithmetic"
	"block"
	"if_else"
)

GetExePathName(exe_path)

foreach(dir ${test_dirs})
	message(STATUS "add ${dir} func test")
	add_test(
		NAME ${dir}_test
		COMMAND bash ./driver.sh ${exe_path}
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/${dir}
	)
endforeach()

