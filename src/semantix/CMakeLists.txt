file(GLOB src "*.cpp")

AddLLVMTrgLibrary(semantix OBJECT ${src})
target_include_directories(semantix PUBLIC "include")

