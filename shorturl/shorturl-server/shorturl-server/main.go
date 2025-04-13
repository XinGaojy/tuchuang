package main

import (
	"flag"
	"fmt"
	"net"
	"shorturl/pkg/config"
	"shorturl/pkg/db/mysql"
	"shorturl/pkg/db/redis"
	"shorturl/pkg/interceptor"
	"shorturl/pkg/log"
	"shorturl/proto"
	cache2 "shorturl/shorturl-server/cache"
	"shorturl/shorturl-server/data"
	"shorturl/shorturl-server/server"

	"google.golang.org/grpc"
	"google.golang.org/grpc/health"
	"google.golang.org/grpc/health/grpc_health_v1"
)

var (
	configFile = flag.String("config", "dev.config.yaml", "")
)

func main() {
	flag.Parse()
	//初始化配置
	config.InitConfig(*configFile)
	cnf := config.GetConfig()

	//创建监听
	lis, err := net.Listen("tcp", fmt.Sprintf("%s:%d", cnf.Server.IP, cnf.Server.Port))
	if err != nil {
		log.Fatal(err)
		return
	}

	//初始化日志
	logger := log.NewLogger()
	logger.SetOutput(log.GetRotateWriter(cnf.Log.LogPath))
	logger.SetLevel(cnf.Log.Level)
	logger.SetPrintCaller(true)

	log.SetLevel(cnf.Log.Level)
	log.SetOutput(log.GetRotateWriter(cnf.Log.LogPath))
	log.SetPrintCaller(true)

	//初始化redis
	redis.InitRedisPool(cnf)
	//初始化mysql
	mysql.InitMysql(cnf)

	//初始化data层工厂对象
	urlMapDataFactory := data.NewUrlMapDataFactory(logger, mysql.GetDB(), cnf)
	//初始化cache工厂对象
	cacheFactory := cache2.NewRedisCacheFactory(redis.GetPool())
	//初始化服务
	shortUrlServer := server.NewService(cnf, logger, cacheFactory, urlMapDataFactory)

	data := urlMapDataFactory.NewUrlMapData(true)
	list, err := data.GetAll()
	fmt.Println("list:", list, err) //这里打印已有的短链信息

	//初始化健康检查
	healthCheck := health.NewServer()

	s := grpc.NewServer(grpc.ChainUnaryInterceptor(interceptor.UnaryAuthInterceptor, interceptor.UnaryErrorInterceptor), grpc.ChainStreamInterceptor(interceptor.StreamAuthInterceptor, interceptor.StreamErrorInterceptor))
	proto.RegisterShortUrlServer(s, shortUrlServer)
	grpc_health_v1.RegisterHealthServer(s, healthCheck)
	if err := s.Serve(lis); err != nil {
		log.Fatal(err)
	}
}
