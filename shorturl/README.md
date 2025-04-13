# 短链服务
1. shorturl-server功能
 - 根据原始链接生成短链；
 - 根据短链返回原始链接；
2. shorturl-proxy功能
 - 提供http接口访问；
 - 将短链接重定向到原始链接； 

# go开发环境安装

## 下载go安装包
golang官网下载地址： https://golang.google.cn/dl/
目前使用的是1.20.5版本
```
wget https://dl.google.com/go/go1.20.5.linux-amd64.tar.gz
```
## 解压和设置环境变量

将其解压缩到/usr/local/(会在/usr/local中创建一个go目录)
```
sudo tar -C /usr/local -xzf go1.20.5.linux-amd64.tar.gz
```
## 添加环境变量
sudo vim /etc/profile
在打开的文件最后添加：
```
export GOPATH=~/go # 在home目录下的go目录
export GOROOT=/usr/local/go
export PATH=$PATH:/usr/local/go/bin
export PATH=$PATH:$GOPATH:$GOROOT:$GOPATH/bin
```
wq保存退出后source一下
```
source /etc/profile
```
查看版本
```
go version
```
## 设置代理(非常重要)和开启GO111MODULE
go提供了带量的第三方库，单不少都是国外的源导致国内下载速度非常慢，所以需要设置代理

设置代理
```
go env -w GOPROXY=https://goproxy.cn,direct
```
开启go module
```
go env -w GO111MODULE=on
```
## 测试使用
在你的工作区创建hello.go
```
package main
import "fmt"
func main() {
    fmt.Printf("hello, world\n")
}
```
构建项目 
```
go build hello.go
```
会生成一个名为hello的可执行文件
执行项目
```
$ ./hello
hello, world
```




