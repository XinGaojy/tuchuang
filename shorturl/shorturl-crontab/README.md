# 定时任务
## 开源项目
``` 
https://github.com/robfig/cron
```
## 依赖安装
``` 
go get github.com/robfig/cron/v3@v3.0.0
```
## 导入项目
``` 
import "github.com/robfig/cron/v3"
```
## 参考文档
``` 
https://pkg.go.dev/github.com/robfig/cron
```
 
## 生成代码
``` 
# 在当前目录
#  go mod tidy的作用是把项目所需要的依赖添加到go.mod
go mod tidy

# 编译执行文件
go build -o shorturl-crontab
```

## 修改配置文件
具体修改参数见当前目录的dev.config.yaml配置文件。
 
## 启动shorturl-crontab
```
./shorturl-crontab --config=dev.config.yaml
```
如果还在调试阶段则可以直接run代码，这样新修改的代码能直接起作用
```
go run  main.go --config dev.config.yaml
```