package short_url

import (
	"shorturl-proxy/pkg/config"
	"shorturl-proxy/pkg/grpc_client_pool"
	"shorturl-proxy/services"
	"sync"
)

var pool grpc_client_pool.ClientPool
var once sync.Once

type shortUrlClient struct {
	services.DefaultClient
}

func GetShortUrlClientPool() grpc_client_pool.ClientPool {
	once.Do(func() {
		cnf := config.GetConfig()
		client := &shortUrlClient{}
		pool = client.GetPool(cnf.DependOn.ShortUrl.Address)
	})
	return pool
}
