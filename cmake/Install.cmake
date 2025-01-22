include(GNUInstallDirs)

function(InstallTarget target )
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/src/${target}"
	DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/${target}"
)

install(TARGETS ${target}
	EXPORT ${target}Lib
	INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${target}
)

install(EXPORT ${target}Lib
	NAMESPACE ${target}::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/${target}/cmake
)

install(FILES "${CMAKE_SOURCE_DIR}/cmake/${target}Config.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/${target}/cmake
)
endfunction()

