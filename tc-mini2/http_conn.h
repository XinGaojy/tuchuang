/*
 * http_conn.h
 *
 *  Created on: 2013-9-29
 *      Author: ziteng
 */

#ifndef __HTTP_CONN_H__
#define __HTTP_CONN_H__

#include "http_parser_wrapper.h"
#include "netlib.h"
#include "util.h"
 
 
#define READ_BUF_SIZE 2048

#define HTTP_RESPONSE_HTML                                                     \
    "HTTP/1.1 200 OK\r\n"                                                      \
    "Connection:close\r\n"                                                     \
    "Content-Length:%d\r\n"                                                    \
    "Content-Type:application/json;charset=utf-8\r\n\r\n%s"

#define HTTP_RESPONSE_HTML_MAX 4096

enum {
    CONN_STATE_IDLE,
    CONN_STATE_CONNECTED,
    CONN_STATE_OPEN,
    CONN_STATE_CLOSED,
};

 
// 继承引用计数的基类，当引用计数减到0时析构对象
class CHttpConn : public CRefObject {
public:
    //构造函数
    CHttpConn();
    //析构函数
    virtual ~CHttpConn();
    // 获取当前自定义的唯一handle
    uint32_t GetConnHandle() { return conn_handle_; }
    // 返回对端ip
    char *GetPeerIP() { return (char *)peer_ip_.c_str(); }
    //发送数据，最终调用socket的send
    int Send(void *data, int len);
    //关闭连接，最终调用socket的close
    void Close();
    // accept收到新连接fd，触发OnConnect的调用，并把socket fd传递进来
    void OnConnect(net_handle_t socket_handle);
    // 可读事件触发回调OnRead
    void OnRead();
    // 可写事件触发回调OnWrite
    void OnWrite();
    // 关闭事件触发OnClose
    void OnClose();
    // 数据发送完毕回调OnWriteComlete，自己业务处理的
    void OnWriteComlete();

private:      
    // 账号注册处理
    void _HandleRegisterRequest(string &url, string &post_data);
    // 账号登陆处理
    void _HandleLoginRequest(string &url, string &post_data);
    //获取我的文件列表
    void _HandleMyfilesRequest(string &url, string &post_data);

protected:
    //socket fd
    net_handle_t socket_handle_;  
    // 业务自定义唯一标识
    uint32_t conn_handle_;
    //当前socket写缓存是否有空间，=ture当前socket写缓存已满，=false当前socket写缓存有空间可以写
    bool busy_;
    //连接状态
    uint32_t state_;
    //保存对端ip
    std::string peer_ip_;
    //保存对端端口
    uint16_t peer_port_;
    //缓存从socket读取的数据
    CSimpleBuffer in_buf_;
    //缓存还没有发送出去的数据
    CSimpleBuffer out_buf_;

    // http解析
    CHttpParserWrapper http_parser_;
};



#endif /* IMCONN_H_ */
