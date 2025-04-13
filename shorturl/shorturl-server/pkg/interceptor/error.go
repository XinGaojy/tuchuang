package interceptor

import (
	"context"
	"google.golang.org/grpc"
	"shorturl/pkg/log"
	"shorturl/pkg/xerrors"
)

type interface1 interface {
	f1()
}
type s1 struct {
}

func (s *s1) f1() {}

var i1 interface1 = new(s1)

func StreamErrorInterceptor(srv interface{}, ss grpc.ServerStream, info *grpc.StreamServerInfo, handler grpc.StreamHandler) error {
	err := handler(srv, ss)
	switch err.(type) {
	case *xerrors.Error:
		//对error进行一些处理，例如：多语言和统一错误消息
		log.Error(err)
		err = xerrors.New("触发了业务流限制")
	default:
		//其他类型，暂不处理
	}
	return err
}

func UnaryErrorInterceptor(ctx context.Context, req interface{}, info *grpc.UnaryServerInfo, handler grpc.UnaryHandler) (resp interface{}, err error) {
	resp, err = handler(ctx, req)
	switch err.(type) {
	case *xerrors.Error:
		//对error进行一些处理，例如：多语言和统一错误消息
		log.Error(err)
		err = xerrors.New("触发了业务流限制")
	default:
		//其他类型，暂不处理
	}
	return resp, err
}
