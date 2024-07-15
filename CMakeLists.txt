cmake_minimum_required(VERSION 3.10)

# 定义项目名称和语言
project(ethan LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 设置编译选项
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

# 
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations")

# 使 make 显示详细编译信息
set(CMAKE_VERBOSE_MAKEFILE ON)

#??????
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# 自定义cmake函数
include (cmake/utils.cmake)

# 添加头文件搜索路径
include_directories("/usr/include/")
include_directories("/src")
include_directories("/http")


# 找到 jsoncpp 库
find_package(jsoncpp REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
include_directories(${JSONCPP_INCLUDE_DIRS})
message(STATUS ${JSONCPP_INCLUDE_DIRS})

# 搜索 yaml-cpp 库
find_library(YAMLCPP NAMES yaml-cpp PATHS "/home/d")

#
find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

#find_package(MySQL REQUIRED)
#if(MYSQL_FOUND)
#    include_directories(${MYSQL_INCLUDE_DIR})
#endif()

#find_package(Protobuf)
#if(Protobuf_FOUND)
#    include_directories(${Protobuf_INCLUDE_DIRS})
#endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

find_package(ZLIB REQUIRED)
if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIR})
endif()



# 定义源文件变量，排除头文件
file(GLOB SOURCES "src/*.cpp" "src/utils/*.cpp"  "http/*.cpp"  "http/servlets/*.cpp"  "chatroom/*.cpp" )

#.rl complier,先定义SOURCE再添加
ragelmaker(http/http11_parser.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/http)
ragelmaker(http/httpclient_parser.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/http)
ragelmaker(src/uri.rl SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src)



# 用于创建一个共享库（也称为动态链接库）并为其指定源文件
add_library(sylar SHARED ${SOURCES})

# 链接库文件
set(LIBS sylar yaml-cpp ${OPENSSL_LIBRARIES} ${JSONCPP_LIBRARIES} ${ZLIB_LIBRARIES}) 

# 设置输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)


# add_executable(可执行文件)
# add_dependencies ("add_library->sylar->add_dependencies"  replace  target_sources->SOURCES)(以添加共享库的方式代替添加源文件)
# target_link_libraries(外部库)
sylar_add_executable(test_wss "test/test_ws_server.cpp" sylar "${LIBS}")
sylar_add_executable(test_wsc "test/test_ws_client.cpp" sylar "${LIBS}")
sylar_add_executable(test_app "test/test_application.cpp" sylar "${LIBS}")





