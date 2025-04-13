package cache

import (
	"context"
	"github.com/redis/go-redis/v9"
	redis_pkg "shorturl/pkg/db/redis"
	"time"
)

type redisKVCache struct {
	redisClient *redis.Client
	destroy     func()
}

func newRedisKVCache(client *redis.Client, destroy func()) KVCache {
	return &redisKVCache{
		redisClient: client,
		destroy:     destroy,
	}
}
func (c *redisKVCache) Get(key string) (string, error) {
	key = redis_pkg.GetKey(key)
	res, err := c.redisClient.Get(context.Background(), key).Result()
	if err == redis.Nil {
		return "", nil
	}
	return res, err
}
func (c *redisKVCache) Set(key, value string, ttl int) error {
	key = redis_pkg.GetKey(key)
	return c.redisClient.SetEx(context.Background(), key, value, time.Second*time.Duration(ttl)).Err()
}
func (c *redisKVCache) Expire(key string, ttl int) error {
	key = redis_pkg.GetKey(key)
	return c.redisClient.Expire(context.Background(), key, time.Second*time.Duration(ttl)).Err()
}
func (c *redisKVCache) Destroy() {
	if c.destroy != nil {
		c.destroy()
	}
}

type redisCacheFactory struct {
	redisPool redis_pkg.RedisPool
}

func NewRedisCacheFactory(redisPool redis_pkg.RedisPool) CacheFactory {
	return &redisCacheFactory{redisPool: redisPool}
}
func (f *redisCacheFactory) NewKVCache() KVCache {
	redisClient := f.redisPool.Get()
	destroy := func() {
		f.redisPool.Put(redisClient)
	}
	return newRedisKVCache(redisClient, destroy)
}
