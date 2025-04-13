#图床项目
**技术栈**
- 采用reactor网络模型
- 使用线程池
- 使用连接池提升mysql/redis访问效率
- 使用spdlog作为日志库
- fastdfs存储文件
- http上传下载原理

**第一节课 fastdfs架构分析和配置 源码和课件**
- 源码：tc-src 一定要跑起来
- 课件：课程资料/1

**第二节课 文件传输和接口设计 源码和课件**
- 源码：tc-mini2  尽量参考课上的方式自己手动添加和调试代码
- 课件：课程资料/2


# 编译-这里只是供参考，具体的编译方式参考课件

编译spdlog

```
mkdir build
cmake -CMAKE_BUILD_TYPE=Debug ..
make -j4
sudo make install
```
默认安装路径：
头文件：/usr/local/include/spdlog
库文件：/usr/local/lib/x86_64-linux-gnu/libspdlogd.a

编译tc-src服务程序
```
cd tc-src
mkdir build
cd build
cmake ..
make

编译完成后，将tuchuang/tc-src 下 的tc_http_server.conf拷贝到build目录
后台程序默认从当前路径加载配置（相当于写死了）

cp ../tc_http_server.conf .

目前先用前台运行的方式方便观察打印信息

./tc_http_server
```

如果在编译tc-src的时候出现protobuf头文件的报错，比如
```
/usr/local/include/google/protobuf/parse_context.h:534:17: error: expected unqualified-id before '=’token  
   uint32 t byte:= ptr[1];
```
则把parse_context.h:534 行位置相应的byte变量改成byte2，主要是这个byte的定义和其他开源组件有冲突，将byte修改成byte2不会影响protobuf的功能，因为在protobuf这里他只是一个变量。**需要注意的是这个函数用到byte变量的地方都相应修改为byte2**
