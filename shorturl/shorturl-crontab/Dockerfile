# 编译阶段
FROM golang:1.20 as stage0
RUN go env -w GOPROXY=https://goproxy.cn,https://proxy.golang.com.cn,direct
ADD ./ /src/shorturl-crontab
WORKDIR /src/shorturl-crontab
RUN CGO_ENABLED=0 GOOS=linux GOARCH=amd64 go build -o shorturl-crontab .

FROM alpine:latest as stage1
MAINTAINER nick
WORKDIR /app/
ADD ./dev.config.yaml /app/config.yaml
COPY --from=stage0 /src/shorturl-crontab/shorturl-crontab ./
# 指定入口程序
ENTRYPOINT ["./shorturl-crontab"]
# 指定容器的启动命令或者入口程序的参数
CMD ["--config=config.yaml"]