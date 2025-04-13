# 1 编译说明


编译spdlog
```
mkdir build
cd build
# 这里我们编译debug的版本方便后续debug代码学习
cmake -D CMAKE_BUILD_TYPE=debug ..
#4线程编译（多线程编译更快）
make -j4
make install
```

使用短链时需要生成grpc接口
```
cd tc-src/api
protoc --cpp_out=. --grpc_out=. --plugin=protoc-gen-grpc=/usr/local/bin/grpc_cpp_plugin shorturl.proto

```


```
cd tc-src
mkdir build
cd build
cmake ..
make
```

得到执行文件

```
sudo ./tc_http_server
```
如果之前的配置是root权限，这里启动程序的时候也以root权限启动，这样不会存在权限的问题。（nginx文件上传模块会把文件写到临时目录，这里的服务程序需要进行重命名，需要权限）

**需要将修改的tc_http_server.conf的拷贝到执行目录。**



# 2 导入数据库

```
root@iZbp1h2l856zgoegc8rvnhZ:~/0voice/tc/tuchuang$ mysql -uroot -p          #登录mysql

mysql>
mysql> source /root/tuchuang/tuchuang/0voice_tuchuang.sql;   #导入带索引的数据库，具体看自己存放的路径
mysql> source /root/tuchuang/tuchuang/0voice_tuchuang_noindex.sql;   #导入不带索引的数据库，具体看自己存放的路径
```
带索引和不带索引的数据库，用于后续做性能测试，加深对索引的理解。

# 3 修改配置文件
修改tc_http_server.conf
## 配置是否生成短链,主要是将完整的文件下载路径转成短链
```
# 是否开启短链, 主要是图片分享地址，如果开启需要设置shorturl-server grpc服务地址
enable_shorturl=1
# 因为当前部署是同一台机器所以使用127.0.0.1，注意端口和shorturl-server保持一致
shorturl_server_address=127.0.0.1:50051
shorturl_server_access_token=e8n05nr9jey84prEhw5u43th0yi294780yjr3h7sksSdkFdDngKi
```

## 其他配置
具体见tc_http_server.conf根据自己服务器实际参数修改。


# 4 常见报错
## ERROR - file: shared_func.c, line: 1159, file /etc/fdfs/client.conf not exist
因为 /etc/fdfs/client.conf文件不存在，没有配置。

## 


# 5 代码说明
 
20230611 
1. tc-src 代码风格以谷歌C++风格为主，支持部分函数使用c++11线程池，其他的函数大家可以根据api_register.cc范例做修改。
2. 修正部分头文件重复的问题
3. 大家可以在此版本的基础上进一步优化。
4. 谷歌C++代码风格地址：https://www.bookstack.cn/read/google-cpp-style/1.md



# 6 数据库是否创建索引对比范例

比如user的name字段为例子。



## 字符串（16字符）查询

```
不创建索引：
20221109 12:32:34.065727Z WARN  Reg 10000 times need the time: 7848ms, average time: 0.7848ms, qps: 1274 - TestRegister.cpp:104
20221109 12:32:39.351793Z WARN  SEL 1000 times need the time: 5286ms, average time: 5.286ms, qps: 189

 - TestRegister.cpp:129
   20221109 12:32:46.937852Z WARN  Reg 10000 times need the time: 7586ms, average time: 0.7586ms, qps: 1318 - TestRegister.cpp:104
   20221109 12:32:53.398511Z WARN  SEL 1000 times need the time: 6461ms, average time: 6.461ms, qps: 154

 - TestRegister.cpp:129
   20221109 12:33:00.958943Z WARN  Reg 10000 times need the time: 7560ms, average time: 0.756ms, qps: 1322 - TestRegister.cpp:104
   20221109 12:33:10.246383Z WARN  SEL 1000 times need the time: 9288ms, average time: 9.288ms, qps: 107

创建索引： 对于字符串（16字符）查询，至少能有50倍的差别
20221109 12:35:01.232602Z WARN  Reg 10000 times need the time: 7667ms, average time: 0.7667ms, qps: 1304 - TestRegister.cpp:104
20221109 12:35:01.375095Z WARN  SEL 1000 times need the time: 143ms, average time: 0.143ms, qps: 6993

20221109 12:35:09.628334Z WARN  Reg 10000 times need the time: 8252ms, average time: 0.8252ms, qps: 1211 - TestRegister.cpp:104
20221109 12:35:09.752298Z WARN  SEL 1000 times need the time: 124ms, average time: 0.124ms, qps: 8064

20221109 12:35:17.659541Z WARN  Reg 10000 times need the time: 7907ms, average time: 0.7907ms, qps: 1264 - TestRegister.cpp:104
20221109 12:35:17.780371Z WARN  SEL 1000 times need the time: 121ms, average time: 0.121ms, qps: 8264
```

## 字符串（32字符）查询



```
20221109 12:39:02.116297Z INFO  db_host:127.0.0.1, db_port:3306, db_dbname:0voice_tuchuang_index, db_username:root, db_password:123456 - DBPool.cpp:635
20221109 12:39:02.137699Z INFO  db_host:127.0.0.1, db_port:3306, db_dbname:0voice_tuchuang_index, db_username:root, db_password:123456 - DBPool.cpp:635
20221109 12:39:09.937993Z WARN  Reg 10000 times need the time: 7785ms, average time: 0.7785ms, qps: 1284 - TestRegister.cpp:104
20221109 12:39:27.018825Z WARN  SEL 1000 times need the time: 17080ms, average time: 17.08ms, qps: 58

 - TestRegister.cpp:129
   20221109 12:39:34.339589Z WARN  Reg 10000 times need the time: 7321ms, average time: 0.7321ms, qps: 1365 - TestRegister.cpp:104
   20221109 12:39:54.142958Z WARN  SEL 1000 times need the time: 19803ms, average time: 19.803ms, qps: 50

 - TestRegister.cpp:129
   20221109 12:40:01.536958Z WARN  Reg 10000 times need the time: 7393ms, average time: 0.7393ms, qps: 1352 - TestRegister.cpp:104
   20221109 12:40:23.985673Z WARN  SEL 1000 times need the time: 22449ms, average time: 22.449ms, qps: 44

创建索引： 对于字符串（32字符）查询，至少能有100倍的差别; 但也看到插入索引对写入性能是有一定的影响10%左右
20221109 12:42:34.041657Z WARN  Reg 10000 times need the time: 7989ms, average time: 0.7989ms, qps: 1251 - TestRegister.cpp:104
20221109 12:42:34.259736Z WARN  SEL 1000 times need the time: 218ms, average time: 0.218ms, qps: 4587

 - TestRegister.cpp:129
   20221109 12:42:42.211293Z WARN  Reg 10000 times need the time: 7951ms, average time: 0.7951ms, qps: 1257 - TestRegister.cpp:104
   20221109 12:42:42.346641Z WARN  SEL 1000 times need the time: 135ms, average time: 0.135ms, qps: 7407

 - TestRegister.cpp:129
   20221109 12:42:50.328924Z WARN  Reg 10000 times need the time: 7981ms, average time: 0.7981ms, qps: 1252 - TestRegister.cpp:104
   20221109 12:42:50.481601Z WARN  SEL 1000 times need the time: 153ms, average time: 0.153ms, qps: 6535



1百万条数据 有索引的插入和查询, 16字符 
20221109 12:35:01.232602Z WARN  Reg 10000 times need the time: 7667ms, average time: 0.7667ms, qps: 1304 - TestRegister.cpp:104
20221109 12:35:01.375095Z WARN  SEL 1000 times need the time: 143ms, average time: 0.143ms, qps: 6993

1百万条数据 没索引的插入和查询，16字符，查询速度令人发指
20221109 12:32:46.937852Z WARN  Reg 10000 times need the time: 7586ms, average time: 0.7586ms, qps: 1318 - TestRegister.cpp:104
20221109 12:32:53.398511Z WARN  SEL 1000 times need the time: 6461ms, average time: 6.461ms, qps: 154


1百万条数据 有索引的插入和查询, 32字符
20221109 12:58:49.877556Z WARN  Reg 1000000 times need the time: 822402ms, average time: 0.822402ms, qps: 1215 - TestRegister.cpp:104
20221109 12:58:50.092348Z WARN  SEL 1000 times need the time: 214ms, average time: 0.214ms, qps: 4672

1百万条数据 没索引的插入和查询，32字符，查询速度令人发指
20221109 13:12:38.593070Z WARN  Reg 1000000 times need the time: 762373ms, average time: 0.762373ms, qps: 1311 - TestRegister.cpp:104
20221109 13:19:22.471249Z WARN  SEL 1000 times need the time: 403877ms, average time: 403.877ms, qps: 2
```

