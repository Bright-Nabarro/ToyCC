file(GLOB src "*.cpp")

AddLLVMTrgLibrary(semantix OBJECT ${src})
target_include_directories(semantix PUBLIC "include")
target_link_libraries(semantix PUBLIC spdlog::spdlog)

