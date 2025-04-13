/*
 *  a wrap for non-block socket class for Windows, LINUX and MacOS X platform
 */

#ifndef __SOCKET_H__
#define __SOCKET_H__

#include "ostype.h"
#include "util.h"

enum {
    SOCKET_STATE_IDLE,
    SOCKET_STATE_LISTENING,
    SOCKET_STATE_CONNECTING,
    SOCKET_STATE_CONNECTED,
    SOCKET_STATE_CLOSING
};

class CBaseSocket : public CRefObject {
  public:
    CBaseSocket();

    virtual ~CBaseSocket();

    SOCKET GetSocket() { return socket_; }
    void SetSocket(SOCKET fd) { socket_ = fd; }
    void SetState(uint8_t state) { state_ = state; }

    void SetCallback(callback_t callback) { callback_ = callback; }
    void SetCallbackData(void *data) { callback_data_ = data; }
    void SetRemoteIP(char *ip) { remote_ip_ = ip; }
    void SetRemotePort(uint16_t port) { remote_port_ = port; }
    void SetSendBufSize(uint32_t send_size);
    void SetRecvBufSize(uint32_t recv_size);

    const char *GetRemoteIP() { return remote_ip_.c_str(); }
    uint16_t GetRemotePort() { return remote_port_; }
    const char *GetLocalIP() { return local_ip_.c_str(); }
    uint16_t GetLocalPort() { return local_port_; }

  public:
    int Listen(const char *server_ip, uint16_t port, callback_t callback,
               void *callback_data);

    net_handle_t Connect(const char *server_ip, uint16_t port,
                         callback_t callback, void *callback_data);

    int Send(void *buf, int len);

    int Recv(void *buf, int len);

    int Close();

  public:
    void OnRead();
    void OnWrite();
    void OnClose();

  private: // 私有函数以_ 开头
    int _GetErrorCode();
    bool _IsBlock(int error_code);

    void _SetNonblock(SOCKET fd);
    void _SetReuseAddr(SOCKET fd);
    void _SetNoDelay(SOCKET fd);
    void _SetAddr(const char *ip, const uint16_t port, sockaddr_in *addr);

    void _AcceptNewSocket();

  private:
    string remote_ip_;
    uint16_t remote_port_;
    string local_ip_;
    uint16_t local_port_;

    callback_t callback_;
    void *callback_data_;

    uint8_t state_;
    SOCKET socket_;
};

CBaseSocket *FindBaseSocket(net_handle_t fd);

#endif
