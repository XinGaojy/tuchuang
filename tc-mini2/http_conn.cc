/*
 * HttpConn.cpp
 *  Modify on: 2024-07-22
 * 		Author: darren
 *  Created on: 2022-10-30
 *      Author: darren
 */

#include "http_conn.h"
#include "api_common.h"
#include "api_login.h"
#include "api_myfiles.h"
#include "api_register.h"
#include "http_parser_wrapper.h"

CHttpConn *FindHttpConnByHandle(uint32_t handle);
typedef hash_map<uint32_t, CHttpConn *> HttpConnMap_t;
// conn_handle 从0开始递增，可以防止因socket handle重用引起的一些冲突
static uint32_t g_conn_handle_generator = 0;

static HttpConnMap_t g_http_conn_map;

 
CHttpConn *FindHttpConnByHandle(uint32_t conn_handle) {
    CHttpConn *pConn = NULL;
    HttpConnMap_t::iterator it = g_http_conn_map.find(conn_handle);
    if (it != g_http_conn_map.end()) {
        pConn = it->second;
    }
    return pConn;
}

// 连接的处理函数 
void httpconn_callback(void *callback_data, uint8_t msg, uint32_t handle,
                       uint32_t uParam, void *pParam) {
    NOTUSED_ARG(uParam);
    NOTUSED_ARG(pParam);
    
    // convert void* to uint32_t, oops
    uint32_t conn_handle = *((uint32_t *)(&callback_data)); //获取连接标识
    CHttpConn *pConn = FindHttpConnByHandle(conn_handle);   //根据连接标识找到对应的http对象
    if (!pConn) {   //如果没有则返回
        // LogWarn("no find conn_handle: {}", conn_handle);
        return;
    }
    pConn->AddRef();        //添加引用计数
    switch (msg) {
    case NETLIB_MSG_READ:   //可读事件
        pConn->OnRead();
        break;
    case NETLIB_MSG_WRITE:  //可写事件
        pConn->OnWrite();
        break;
    case NETLIB_MSG_CLOSE:
        pConn->OnClose();   //关闭事件
        break;
    default:
        LogError("!!!httpconn_callback error msg:{}", msg); 
        //fix me，可以考虑如果这里触发，直接关闭该连接
        // pConn->Close();
        break;
    }
    pConn->ReleaseRef(); //释放引用计数，如果数据发送完毕，对应的http 连接在这个位置为析构对象
}
 
 
 
CHttpConn::CHttpConn() {
    busy_ = false;
    socket_handle_ = NETLIB_INVALID_HANDLE;
    state_ = CONN_STATE_IDLE;

    conn_handle_ = ++g_conn_handle_generator;
    if (conn_handle_ == 0) {
        conn_handle_ = ++g_conn_handle_generator;
    }

   
    LogInfo("conn_handle_: {:X}", conn_handle_);
}

CHttpConn::~CHttpConn() {
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
}

int CHttpConn::Send(void *data, int len) {
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    if (busy_) {
        out_buf_.Write(data, len);
        return len;
    }

    int ret = netlib_send(socket_handle_, data, len);
    if (ret < 0)
        ret = 0;

    if (ret < len) {
        out_buf_.Write( (char *)data + ret, len - ret); // 保存buffer里面，下次reactor write触发后再发送
        busy_ = true;
        LogInfo("not send all, remain= {}", out_buf_.GetWriteOffset());
    } else {
        OnWriteComlete();
    }

    return len - ret;
}

void CHttpConn::Close() {
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    state_ = CONN_STATE_CLOSED;

    g_http_conn_map.erase(conn_handle_);
    netlib_close(socket_handle_);

    ReleaseRef();
}

void CHttpConn::OnConnect(net_handle_t socket_handle) {
    socket_handle_ = socket_handle; //保存socket fd，后续接收、发送数据需要，关闭socket需要
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    state_ = CONN_STATE_CONNECTED;
    g_http_conn_map.insert(make_pair(conn_handle_, this));  // 插入map，后续回调函数时使用
    // 最核心在于这里，要绑定一个函数处理各种socket事件，绑定函数为httpconn_callback
    netlib_option(socket_handle_, NETLIB_OPT_SET_CALLBACK, (void *)httpconn_callback);
    // 绑定连接对象的标识conn_handle_，回调时用来区别是哪个连接
    netlib_option(socket_handle_, NETLIB_OPT_SET_CALLBACK_DATA, reinterpret_cast<void *>(conn_handle_));
    //获取对端ip，在当前程序没有使用这个变量，先不理会
    netlib_option(socket_handle_, NETLIB_OPT_GET_REMOTE_IP, (void *)&peer_ip_);
}

void CHttpConn::OnRead() // CHttpConn业务层面的OnRead
{
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    // 1. 把能读取的数据都读取出来
    for (;;) {
        uint32_t free_buf_len = in_buf_.GetAllocSize() - in_buf_.GetWriteOffset();
        if (free_buf_len < READ_BUF_SIZE + 1)   //这里多预留一个字节的目的是加上结束符时不会越界
            in_buf_.Extend(READ_BUF_SIZE + 1);
        //读取socket数据
        int ret = netlib_recv(socket_handle_,
                              in_buf_.GetBuffer() + in_buf_.GetWriteOffset(),
                              READ_BUF_SIZE);
        if (ret <= 0)
            break; //没有数据可以读取了

        in_buf_.IncWriteOffset(ret); // 更新下一个接收数据的位置
    }

    // 2.通过http模块解析http请求的数据
    char *in_buf = (char *)in_buf_.GetBuffer();
    uint32_t buf_len = in_buf_.GetWriteOffset();
    in_buf[buf_len] = '\0'; // 末尾加上结束符 方便分析结束位置和打印，这里之所以不越界是因为有预留in_buf_.Extend(READ_BUF_SIZE + 1)
    // 如果buf_len 过长可能是受到攻击，则断开连接 ，目前我们接受的所有数据长度不得大于2K
    if (buf_len > 2048) {
        LogError("get too much data: {}", in_buf);
        Close();
        return;
    }
    LogInfo("buf_len: {}, in_buf: {}", buf_len, in_buf); //将请求的数据都打印出来，方便调试分析http请求
    // 解析http数据
    http_parser_.ParseHttpContent(in_buf, buf_len); // 1. 从socket接口读取数据；2.然后把数据放到buffer in_buf; 3.http解析
    if (http_parser_.IsReadAll()) {
        string url = http_parser_.GetUrl();
        string content = http_parser_.GetBodyContent();
        LogInfo("url: {}", url);                     // for debug
        // 根据url处理不同的业务 
        if (strncmp(url.c_str(), "/api/reg", 8) == 0) { // 注册  url 路由。 根据根据url快速找到对应的处理函数， 能不能使用map，hash
            _HandleRegisterRequest(url, content);
        } else if (strncmp(url.c_str(), "/api/login", 10) == 0) { // 登录
            _HandleLoginRequest(url, content);
        } else if (strncmp(url.c_str(), "/api/myfiles", 10) == 0) { //获取我的文件数量
            _HandleMyfilesRequest(url, content);
        }  else {
            LogError("url unknown, url= {}", url);
            Close();
        }
    }
}

void CHttpConn::OnWrite() {
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    if (!busy_) // busy_ = true代表有数据可以发送，=false代表目前没有数据可以发送
        return;

    // LogInfo("send: {}", m_out_buf.GetWriteOffset());
    int ret = netlib_send(socket_handle_, out_buf_.GetBuffer(),
                          out_buf_.GetWriteOffset());
    if (ret < 0)
        ret = 0;

    int out_buf_size = (int)out_buf_.GetWriteOffset();

    out_buf_.Read(NULL, ret);

    if (ret < out_buf_size) {
        busy_ = true;
        LogInfo("not send all, remain = {}", out_buf_.GetWriteOffset());
    } else {
        OnWriteComlete();
        busy_ = false;
    }
}

void CHttpConn::OnClose() { 
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    Close(); 
}
 

// 账号注册处理
void CHttpConn::_HandleRegisterRequest(string &url, string &post_data) {
    string resp_json;
	int ret = ApiRegisterUser(post_data, resp_json);
	char *http_body = new char[HTTP_RESPONSE_HTML_MAX];
	uint32_t ulen = resp_json.length();
	snprintf(http_body, HTTP_RESPONSE_HTML_MAX, HTTP_RESPONSE_HTML, ulen,
        resp_json.c_str()); 	
    ret = Send((void *)http_body, strlen(http_body));
    delete[] http_body;
    LogInfo("Send remain = {}", ret); 
}
// 账号登陆处理 /api/login
 
void CHttpConn::_HandleLoginRequest(string &url, string &post_data)
{
	string resp_json;
	int ret = ApiUserLogin( post_data, resp_json);
	char *http_body = new char[HTTP_RESPONSE_HTML_MAX];
	uint32_t ulen = resp_json.length();
	snprintf(http_body, HTTP_RESPONSE_HTML_MAX, HTTP_RESPONSE_HTML, ulen,
        resp_json.c_str()); 	
    ret = Send((void *)http_body, strlen(http_body));
    delete[] http_body;
    LogInfo("Send remain = {}", ret); 
}
 
 
 
 
void CHttpConn::_HandleMyfilesRequest(string &url, string &post_data) {
    string resp_json;
    int ret = ApiMyfiles(url, post_data, resp_json);
    char *http_body = new char[HTTP_RESPONSE_HTML_MAX]; // 注意buffer的长度
    uint32_t ulen = resp_json.length();
    // LogInfo("json size: {}", resp_json.size());
    snprintf(http_body, HTTP_RESPONSE_HTML_MAX, HTTP_RESPONSE_HTML, ulen, resp_json.c_str());
    ret = Send((void *)http_body, strlen(http_body));
    delete[] http_body;
    LogInfo("Send remain = {}", ret);  //如果还剩数据没有发送，则需要onWrite触发后发送
}
  

void CHttpConn::OnWriteComlete() {
    LogInfo("conn_handle_ = {}, socket_handle_ = {}", conn_handle_, socket_handle_);
    LogInfo("write complete");
    Close();
}
 
