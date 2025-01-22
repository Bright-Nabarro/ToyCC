option(static_check OFF)

function(addClangTidy target)
	find_program(CLANG-TIDY_PATH clang-tidy REQUIRED)
	set(clang-tidy_args ${CLANG-TIDY_PATH} -checks=*
		#默认禁用clang命名空间检查
		-llvmlibc-implementation-in-namespace)
	set_target_properties(${target}
		PROPERTIES CXX_CLANG_TIDY ${clang-tidy_args}
	)
endfunction()

