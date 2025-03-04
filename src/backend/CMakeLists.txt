file(GLOB src "*.cpp")


AddLLVMTrgLibrary(backend OBJECT ${src})

target_link_libraries(backend PUBLIC
	front spdlog::spdlog semantix
)

target_include_directories(backend PUBLIC
	"include"
)

#include(Doxygen)
#Doxygen("${CMAKE_CURRENT_SOURCE_DIR}/app" "${CMAKE_SOURCE_DIR}/docs")
#
#include(Install)

