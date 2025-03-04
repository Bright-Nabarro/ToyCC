set(test_dirs
	"return"
	"arithmetic"
)

GetExePathName(exe_path)

foreach(dir ${test_dirs})
	message(STATUS "add ${dir} func test")
	add_test(
		NAME ${dir}_test
		COMMAND bash ./driver.sh ${exe_path} ${dir} "${dir}/test.c" 
		WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
	)
endforeach()

