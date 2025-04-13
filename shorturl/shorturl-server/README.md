# shorturl-server
该项目提供以下功能（通过grcp访问）：
- 接收原始链接返回短链接，接口为：GetShortUrl，对应proto文件的
  ```
  rpc GetShortUrl(Url)returns(Url){}
  ```
  比如：图床在生成图片分享后可以将该链接转成短链接。
- 将短链接返回原始链接，接口为：
  ```
  rpc GetOriginalUrl(ShortKey)returns(Url){}
  ```
  比如short-proxy访问shorturl-server获取原始链接，然后short-proxy重定向到原始链接。
## protobuf golang插件安装
``` 
go install google.golang.org/protobuf/cmd/protoc-gen-go@v1.28
go install google.golang.org/grpc/cmd/protoc-gen-go-grpc@v1.2
```
## protoc 生成代码
``` 
# 在当前目录,即是
# 把proto/shorturl.proto生成go代码，对应的go文件在proto目录，cpp也要使用这个shorturl.proto文件
protoc  --go_out=. --go_opt=paths=source_relative --go-grpc_out=. --go-grpc_opt=paths=source_relative ./proto/shorturl.proto

#  go mod tidy的作用是把项目所需要的依赖添加到go.mod
go mod tidy

# 编译执行文件
go build -o short-server ./shorturl-server
```

## 导入数据库
 ```
~/tuchuang/shorturl/shorturl-grpc# mysql -uroot -p  #登录mysql

mysql>
mysql> source /root/tuchuang/shorturl/shorturl-grpc/sql/shorturl.sql; #导入带索引的数据库，具体看自己存放的路径
 ```
## 修改配置文件
具体修改参数见当前目录的dev.config.yaml配置文件。


## 启动shorturl-server
```
./short-server --config=dev.config.yaml
```
如果还在调试阶段则可以直接run代码，这样新修改的代码能直接起作用
```
go run shorturl-server/main.go --config dev.config.yaml
```