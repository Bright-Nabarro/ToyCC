file(GLOB SRC "*.cpp")

add_library(ast OBJECT ${SRC})

target_include_directories(ast PUBLIC "include")

target_link_libraries(ast easylog)
