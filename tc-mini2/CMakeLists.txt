cmake_minimum_required(VERSION 3.0)
PROJECT(tc_http_server)


# AUX_SOURCE_DIRECTORY(./common COMMON_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/base BASE_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp JSON_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/mysql MYSQL_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/redis REDIS_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR}/api API_LIST)
AUX_SOURCE_DIRECTORY(${CMAKE_CURRENT_SOURCE_DIR} SRC_LIST)

#判断编译器类型,如果是gcc编译器,则在编译选项中加入c++14支持
if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "-std=c++14 ${CMAKE_CXX_FLAGS}")
    message(STATUS "optional:-std=c++14")   
endif(CMAKE_COMPILER_IS_GNUCXX)

# 如果要开启警告使用 -Wall
# -Wno-deprecated选项用于关闭编译器产生的关于使用已经被废弃（deprecated）的函数、变量或头文件时产生的警告信息
# -Wno-unused-parameter选项则用于关闭编译器产生的有关未使用函数参数的警告信息。在定义函数时，可能会为了保持一致性而添加某些无用参数。
# 此时编译器会提示这些参数未被使用，但是你又不希望在编译时得到这个警告信息。这种情况下可以使用该选项。
ADD_DEFINITIONS( -g -Wno-deprecated -Wno-unused-parameter -Bsymbolic -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON)

 

INCLUDE_DIRECTORIES(/usr/include/fastdfs /usr/include/fastcommon /usr/local/include/hiredis /usr/include/mysql)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/base)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/jsoncpp)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/mysql)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/redis)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/api)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

# 添加库文件路径，部分Linux版本不自动加载/usr/local/lib
LINK_DIRECTORIES(/usr/local/lib)

#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(tc_http_server ${SRC_LIST} ${BASE_LIST} ${API_LIST} ${LOG_LIST}
        ${MYSQL_LIST} ${REDIS_LIST} ${JSON_LIST})

# 注意添加grpc相关的库gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf
TARGET_LINK_LIBRARIES(tc_http_server pthread   m mysqlclient spdlogd) 
