function(Doxygen output input1)
	find_package(Doxygen)	#include function `doxygen_add_docs`
	if (NOT DOXYGEN_FOUND)
		add_custom_target(doxygen
			COMMAND false
			COMMNENT "Doxygen not found"
		)
		return()
	endif()
	set(readme "${CMAKE_SOURCE_DIR}/README.md")
	set(DOXYGEN_USE_MDFILE_AS_MAINPAGE ${readme})
	set(DOXYGEN_GENERATE_HTML YES)
	set(DOXYGEN_HTML_OUTPUT ${output})
	set(DOXYGEN_HTML_COLORSTYLE "LIGHT")
	UseDoxygenAwesomeCss()

	doxygen_add_docs(doxygen ${readme}
		${input1} 
		${ARGN}
		COMMENT "Generate HTML documentation"
	)
endfunction()

macro(UseDoxygenAwesomeCss)
	include(FetchContent)
	FetchContent_Declare(doxygen-awesome-css
		GIT_REPOSITORY https://github.com/jothepro/doxygen-awesome-css.git
		GIT_TAG v2.3.4
	)
	FetchContent_MakeAvailable(doxygen-awesome-css)
	find_program(GRAPHVIZ_EXECUTABLE NAMES dot)
	if (GRAPHVIZ_EXECUTABLE)
		set(DOXYGEN_HAVE_DOT YES)
	else()
		message(WARNING "graphviz not found when generate documentation")
	endif()
	set(DOXYGEN_DOT_IMAGE_FORMAT svg)
	set(DOXYGEN_GENERATE_TREEVIEW YES)
	set(DOXYGEN_DOT_TRANSPARENT YES)
	set(DOXYGEN_HTML_EXTRA_STYLESHEET
		${doxygen-awesome-css_SOURCE_DIR}/doxygen-awesome.css
	)
endmacro()

