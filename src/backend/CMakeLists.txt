file(GLOB src "*.cpp")

set(trg ${CMAKE_PROJECT_NAME})

AddLLVMTrgExe(${trg} ${src})
ChgExeOutputDir(${trg})

target_link_libraries(${trg} PRIVATE
	front spdlog::spdlog semantix
)

target_include_directories(${trg} PRIVATE
	"include"
)

#include(Doxygen)
#Doxygen("${CMAKE_CURRENT_SOURCE_DIR}/app" "${CMAKE_SOURCE_DIR}/docs")
#
#include(Install)

