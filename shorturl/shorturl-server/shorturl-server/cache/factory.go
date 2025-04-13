package cache

type CacheFactory interface {
	NewKVCache() KVCache
}
