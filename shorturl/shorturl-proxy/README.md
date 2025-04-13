# shorturl-proxy 短链代理
本项目提供HTTP代理，主要目的将shorturl-server生成的短链重定向到原始链接。

总体步骤：
1. 接收短链接；
2. 通过grpc接口GetOriginalUrl调用shorturl-server获取原始链接；
3. 重定向到2.步骤返回的原始链接。

## 生成代码
``` 
# 在当前目录
#  go mod tidy的作用是把项目所需要的依赖添加到go.mod
go mod tidy

# 编译执行文件
go build -o shorturl-proxy 
```

## 修改配置文件
具体修改参数见当前目录的dev.config.yaml配置文件。
 
## 启动shorturl-proxy
```
./shorturl-proxy --config=dev.config.yaml
```
如果还在调试阶段则可以直接run代码，这样新修改的代码能直接起作用
```
go run  main.go --config dev.config.yaml
```