function(SetCoverageBuild target)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
		target_compile_options(${target} PUBLIC --coverage)
		target_link_options(${target} PUBLIC --coverage)
		add_custom_command(TARGET ${target} PRE_BUILD
			#COMMENT "remove gaca file for ${target}"
			COMMAND find ${CMAKE_BINARY_DIR} -type f -name '*.gcda' -exec rm {} +
		)
	endif()
endfunction()

function(AddCoverage target)
	if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    	find_program(LCOV_PATH lcov REQUIRED)
    	find_program(GENHTML_PATH genhtml REQUIRED)
    	add_custom_target(coverage
    	    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    	    COMMENT "Running coverage for ${target}..."
    	    COMMAND ${LCOV_PATH} -d . --zerocounters
    	    COMMAND $<TARGET_FILE:${target}>
    	    COMMAND ${LCOV_PATH} -d . --capture -o coverage.info --ignore-errors inconsistent
    	    COMMAND ${LCOV_PATH} -r coverage.info '/usr/include/*' -o filtered.info
    	    COMMAND ${GENHTML_PATH} -o coverage filtered.info --legend
    	    COMMAND rm -rf coverage.info filtered.info
    	)
	endif()
endfunction()

