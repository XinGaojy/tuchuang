package main

import (
	"flag"
	"shorturl-crontab/cron"
	"shorturl-crontab/pkg/config"
	"shorturl-crontab/pkg/db/mysql"
	"shorturl-crontab/pkg/db/redis"
)

var (
	configFile = flag.String("config", "dev.config.yaml", "")
)

func main() {
	flag.Parse()
	config.InitConfig(*configFile)
	cnf := config.GetConfig()
	//初始化redis
	redis.InitRedisPool(cnf)
	//初始化mysql
	mysql.InitMysql(cnf)
	cron.Run()
}
