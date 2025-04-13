cmake_minimum_required(VERSION 3.0)
PROJECT(tc_test)

AUX_SOURCE_DIRECTORY(./ SRC_LIST)
AUX_SOURCE_DIRECTORY(../common COMMON_LIST)
AUX_SOURCE_DIRECTORY(../jsoncpp JSON_LIST)
AUX_SOURCE_DIRECTORY(../mysql MYSQL_LIST)
AUX_SOURCE_DIRECTORY(../redis REDIS_LIST)
AUX_SOURCE_DIRECTORY(../base BASE_LIST)
AUX_SOURCE_DIRECTORY(../api API_LIST)
AUX_SOURCE_DIRECTORY(../log LOG_LIST)
AUX_SOURCE_DIRECTORY(../log LOG_LIST)
ADD_DEFINITIONS( -g -Wno-deprecated -Wno-unused-parameter -W -Wall -D_REENTRANT -D_FILE_OFFSET_BITS=64 -DAC_HAS_INFO
-DAC_HAS_WARNING -DAC_HAS_ERROR -DAC_HAS_CRITICAL -DTIXML_USE_STL
-DAC_HAS_DEBUG -DLINUX_DAEMON -std=c++14)

# find_package的作用就是去寻找该库的头文件位置、库文件位置以及库文件名称，并将其设为变量，返回提供给CMakeLists.txt其他部分使用
# grpc相关
find_package(Threads REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

message(STATUS "Using protobuf-${Protobuf_VERSION}")
message(STATUS "Using gRPC-${gRPC_VERSION}")

INCLUDE_DIRECTORIES(/usr/include/fastdfs /usr/include/fastcommon  /usr/include/mysql)
INCLUDE_DIRECTORIES(./ ../include)
INCLUDE_DIRECTORIES(../jsoncpp)
INCLUDE_DIRECTORIES(../mysql)
INCLUDE_DIRECTORIES(../redis)
INCLUDE_DIRECTORIES(../base)
INCLUDE_DIRECTORIES(../api)
# INCLUDE_DIRECTORIES()

# 添加库文件路径，部分Linux版本不自动加载/usr/local/lib
LINK_DIRECTORIES(/usr/local/lib)
#ADD_XXXX必须在TARGET_LINK_LIBRARIES前面，否则会报错
#ADD_LIBRARY(${PROJECTNAME} SHARED/STATIC ${SRC_LIST})
ADD_EXECUTABLE(tc_test  ${BASE_LIST} ${API_LIST} ${LOG_LIST}
        ${MYSQL_LIST} ${REDIS_LIST} ${JSON_LIST} ${SRC_LIST} ${COMMON_LIST})

# 注意添加grpc相关的库gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf
TARGET_LINK_LIBRARIES(tc_test pthread fdfsclient fastcommon  m mysqlclient spdlogd
        gRPC::grpc++ gRPC::grpc++_reflection protobuf::libprotobuf) 