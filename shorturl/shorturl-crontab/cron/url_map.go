package cron

import (
	"context"
	"fmt"
	"shorturl-crontab/data"
	"shorturl-crontab/pkg/db/mysql"
	"shorturl-crontab/pkg/db/redis"
	"shorturl-crontab/pkg/log"
	"time"
)

const DefaultUrlMapTTL = 86400 * 31

func setUrlMapMaxID() {
	tables := []string{"url_map", "url_map_user"}

	redisPool := redis.GetPool()
	redisClient := redisPool.Get()
	defer redisPool.Put(redisClient)

	d := data.NewData(mysql.GetDB())
	for _, t := range tables {
		id, err := d.GetMaxID(t) // 找到最新一条记录的id，设置到缓存里
		if err != nil {
			log.Error(err)
			return
		}
		key := redis.GetKey(fmt.Sprintf("%s_%s", t, "maxid"))
		err = redisClient.SetEx(context.Background(), key, id, time.Second*time.Duration(DefaultUrlMapTTL)).Err()
		if err != nil {
			log.Error(err)
			continue
		}
	}
}
