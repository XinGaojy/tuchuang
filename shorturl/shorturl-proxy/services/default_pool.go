package services

import (
	"context"
	"google.golang.org/grpc"
	"google.golang.org/grpc/credentials/insecure"
	"google.golang.org/grpc/metadata"
	"shorturl-proxy/pkg/grpc_client_pool"
	"shorturl-proxy/pkg/log"
)

type DefaultClient struct {
}

func (c *DefaultClient) GetPool(addr string) grpc_client_pool.ClientPool {
	pool, err := grpc_client_pool.GetPool(addr, c.GetOptions()...)
	if err != nil {
		log.Error(err)
		return nil
	}
	return pool
}
func (c *DefaultClient) GetOptions() []grpc.DialOption {
	opts := make([]grpc.DialOption, 0)
	opts = append(opts, grpc.WithTransportCredentials(insecure.NewCredentials()))
	return opts
}

func AppendBearerTokenToContext(ctx context.Context, accessToken string) context.Context {
	md := metadata.Pairs("authorization", "Bearer "+accessToken)
	return metadata.NewOutgoingContext(ctx, md)
}
