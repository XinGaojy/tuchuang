package interceptor

import (
	"context"
	"errors"
	"fmt"
	"shorturl/pkg/config"
	"shorturl/pkg/xerrors"
	"strings"
	"shorturl/pkg/log"

	"google.golang.org/grpc"
	"google.golang.org/grpc/metadata"
)

func StreamAuthInterceptor(srv interface{}, ss grpc.ServerStream, info *grpc.StreamServerInfo, handler grpc.StreamHandler) error {
	err := oauth2Valid(ss.Context())
	if err != nil {
		return err
	}
	return handler(srv, ss)
}

func UnaryAuthInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (resp interface{}, err error) {
	if info.FullMethod != "/grpc.health.v1.Health/Check" {
		err = oauth2Valid(ctx)
		if err != nil {
			return nil, err
		}
	}
	return handler(ctx, req)
}

func oauth2Valid(ctx context.Context) error {
	md, ok := metadata.FromIncomingContext(ctx)
	if !ok {
		//log.Error()
		return errors.New("元数据获取失败，身份认证失败")
	}
	authorization := md["authorization"]
	if len(authorization) < 1 {
		return errors.New("身份令牌校验失败，身份认证失败")
	}

	token := strings.TrimPrefix(authorization[0], "Bearer ")
	conf := config.GetConfig()
	if conf.Server.AccessToken != token {
		log.Error("鉴权失败")
		fmt.Println("鉴权失败, conf.Server.AccessToken: ", conf.Server.AccessToken, ", client token:", token)
		return xerrors.New("鉴权失败")
	}
	return nil
}
