set(trg ${CMAKE_PROJECT_NAME})
AddLLVMTrgExe(${trg} main.cpp)
ChgExeOutputDir(${trg})

target_link_libraries(${trg} PUBLIC
	front backend semantix
)

