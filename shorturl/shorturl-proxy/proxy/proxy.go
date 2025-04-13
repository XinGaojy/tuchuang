package proxy

import (
	"context"
	"github.com/gin-gonic/gin"
	"net/http"
	"net/http/httputil"
	"net/url"
	"shorturl-proxy/pkg/config"
	"shorturl-proxy/pkg/log"
	"shorturl-proxy/services"
	"shorturl-proxy/services/short_url"
	"shorturl-proxy/services/short_url/proto"
)

type proxy struct {
	config *config.Config
	log    log.ILogger
}

func NewProxy(config *config.Config, logger log.ILogger) *proxy {
	return &proxy{
		config: config,
		log:    logger,
	}
}

func (p *proxy) PublicProxy(ctx *gin.Context) {
	p.browserRedirection(ctx, true)
}
func (p *proxy) UserProxy(ctx *gin.Context) {
	p.browserRedirection(ctx, false)
}

// 通过301/302状态码，重定向资源
// 301用于永久性重定向，而302用于临时性重定向
// 301 Moved Permanently：表示请求的资源已经被永久移动到新的URL地址。搜索引擎会将原始URL中的权重转移到新的URL上，并且在未来访问时直接使用新的URL。
// 302 Found (or Moved Temporarily)：表示请求的资源暂时性地移动到了一个不同的URL地址。搜索引擎会保留原始URL，并且在未来访问时继续使用原始URL。
func (p *proxy) browserRedirection(ctx *gin.Context, isPublic bool) {
	shortKey := ctx.Param("short_key")
	originalUrl, err := p.getOriginalUrl(shortKey, isPublic)
	if err != nil {
		p.log.Error(err)
		ctx.JSON(http.StatusInternalServerError, nil)
		return
	}
	//此处采用302临时重定向。如果采用301多次访问同一个短链URL将不经过代理服务直接进入目标地址，无法统计访问次数
	ctx.Redirect(http.StatusFound, originalUrl)
	return
}

// 反向代理
func (p *proxy) proxy(ctx *gin.Context, isPublic bool) {
	shortKey := ctx.Param("short_key")
	originalUrl, err := p.getOriginalUrl(shortKey, isPublic)
	if err != nil {
		p.log.Error(err)
		ctx.JSON(http.StatusInternalServerError, nil)
		return
	}
	target, _ := url.Parse(originalUrl)
	rp := httputil.NewSingleHostReverseProxy(target)
	rp.Director = func(req *http.Request) {
		req.URL.Path = target.Path
		req.URL.Scheme = target.Scheme
		req.URL.Host = target.Host
		req.Host = target.Host
	}
	rp.ServeHTTP(ctx.Writer, ctx.Request)
}

func (p *proxy) getOriginalUrl(shortKey string, isPublic bool) (string, error) {
	grpcClientPool := short_url.GetShortUrlClientPool()
	conn := grpcClientPool.Get()
	defer grpcClientPool.Put(conn)
	client := proto.NewShortUrlClient(conn)
	ctx := services.AppendBearerTokenToContext(context.Background(), p.config.DependOn.ShortUrl.AccessToken)
	res, err := client.GetOriginalUrl(ctx, &proto.ShortKey{
		Key:      shortKey,
		IsPublic: isPublic,
	})
	if err != nil {
		p.log.Error(err)
		return "", err
	}
	return res.Url, err
}
