package cache

// 默认缓存时长为31天
const DefaultTTL = 86400 * 31

type KVCache interface {
	Get(key string) (string, error)
	Set(key, value string, ttl int) error
	Expire(key string, ttl int) error
	Destroy()
}
