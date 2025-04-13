#include "base_socket.h"
#include "event_dispatch.h"

typedef hash_map<net_handle_t, CBaseSocket *> SocketMap;
SocketMap g_socket_map;

void AddBaseSocket(CBaseSocket *pSocket) {
    g_socket_map.insert(make_pair((net_handle_t)pSocket->GetSocket(), pSocket));
}

void RemoveBaseSocket(CBaseSocket *pSocket) {
    g_socket_map.erase((net_handle_t)pSocket->GetSocket());
}

CBaseSocket *FindBaseSocket(net_handle_t fd) {
    CBaseSocket *pSocket = NULL;
    SocketMap::iterator iter = g_socket_map.find(fd);
    if (iter != g_socket_map.end()) {
        pSocket = iter->second;
        pSocket->AddRef();
    }

    return pSocket;
}

//////////////////////////////

CBaseSocket::CBaseSocket() {
    // printf("CBaseSocket::CBaseSocket\n");
    socket_ = INVALID_SOCKET;
    state_ = SOCKET_STATE_IDLE;
}

CBaseSocket::~CBaseSocket() {
    // printf("CBaseSocket::~CBaseSocket, socket=%d\n", m_socket);
}

int CBaseSocket::Listen(const char *server_ip, uint16_t port,
                        callback_t callback, void *callback_data) {
    local_ip_ = server_ip;
    local_port_ = port;
    callback_ = callback;
    callback_data_ = callback_data;

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == INVALID_SOCKET) {
        printf("socket failed, err_code=%d, server_ip=%s, port=%u",
               _GetErrorCode(), server_ip, port);
        return NETLIB_ERROR;
    }

    _SetReuseAddr(socket_);
    _SetNonblock(socket_);

    sockaddr_in serv_addr;
    _SetAddr(server_ip, port, &serv_addr);
    int ret = ::bind(socket_, (sockaddr *)&serv_addr, sizeof(serv_addr));
    if (ret == SOCKET_ERROR) {
        printf("bind failed, err_code=%d, server_ip=%s, port=%u",
               _GetErrorCode(), server_ip, port);
        closesocket(socket_);
        return NETLIB_ERROR;
    }

    ret = listen(socket_, 64);
    if (ret == SOCKET_ERROR) {
        printf("listen failed, err_code=%d, server_ip=%s, port=%u",
               _GetErrorCode(), server_ip, port);
        closesocket(socket_);
        return NETLIB_ERROR;
    }

    state_ = SOCKET_STATE_LISTENING;

    printf("CBaseSocket::Listen on %s:%d", server_ip, port);

    AddBaseSocket(this);
    CEventDispatch::Instance()->AddEvent(socket_, SOCKET_READ | SOCKET_EXCEP);
    return NETLIB_OK;
}

net_handle_t CBaseSocket::Connect(const char *server_ip, uint16_t port,
                                  callback_t callback, void *callback_data) {
    printf("CBaseSocket::Connect, server_ip=%s, port=%d", server_ip, port);

    remote_ip_ = server_ip;
    remote_port_ = port;
    callback_ = callback;
    callback_data_ = callback_data;

    socket_ = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_ == INVALID_SOCKET) {
        printf("socket failed, err_code=%d, server_ip=%s, port=%u",
               _GetErrorCode(), server_ip, port);
        return NETLIB_INVALID_HANDLE;
    }

    _SetNonblock(socket_);
    _SetNoDelay(socket_);
    sockaddr_in serv_addr;
    _SetAddr(server_ip, port, &serv_addr);
    int ret = connect(socket_, (sockaddr *)&serv_addr, sizeof(serv_addr));
    if ((ret == SOCKET_ERROR) && (!_IsBlock(_GetErrorCode()))) {
        printf("connect failed, err_code=%d, server_ip=%s, port=%u",
               _GetErrorCode(), server_ip, port);
        closesocket(socket_);
        return NETLIB_INVALID_HANDLE;
    }
    state_ = SOCKET_STATE_CONNECTING;
    AddBaseSocket(this);
    CEventDispatch::Instance()->AddEvent(socket_, SOCKET_ALL);

    return (net_handle_t)socket_;
}

int CBaseSocket::Send(void *buf, int len) {
    if (state_ != SOCKET_STATE_CONNECTED)
        return NETLIB_ERROR;

    int ret = send(socket_, (char *)buf, len, 0);
    if (ret == SOCKET_ERROR) {
        int err_code = _GetErrorCode();
        if (_IsBlock(err_code)) {
#if ((defined _WIN32) || (defined __APPLE__))
            CEventDispatch::Instance()->AddEvent(m_socket, SOCKET_WRITE);
#endif
            ret = 0;
            // printf("socket send block fd=%d", m_socket);
        } else {
            printf("send failed, err_code=%d, len=%d", err_code, len);
        }
    }

    return ret;
}

int CBaseSocket::Recv(void *buf, int len) {
    return recv(socket_, (char *)buf, len, 0);
}

int CBaseSocket::Close() {
    CEventDispatch::Instance()->RemoveEvent(socket_, SOCKET_ALL);
    RemoveBaseSocket(this);
    closesocket(socket_);
    ReleaseRef();

    return 0;
}

void CBaseSocket::OnRead() {
    if (state_ == SOCKET_STATE_LISTENING) {
        _AcceptNewSocket();
    } else {
        u_long avail = 0;
        int ret = ioctlsocket(socket_, FIONREAD, &avail);
        if ((SOCKET_ERROR == ret) || (avail == 0)) {
            callback_(callback_data_, NETLIB_MSG_CLOSE, (net_handle_t)socket_,
                      NULL);
        } else {
            callback_(callback_data_, NETLIB_MSG_READ, (net_handle_t)socket_,
                      NULL);
        }
    }
}

void CBaseSocket::OnWrite() {
#if ((defined _WIN32) || (defined __APPLE__))
    CEventDispatch::Instance()->RemoveEvent(m_socket, SOCKET_WRITE);
#endif

    if (state_ == SOCKET_STATE_CONNECTING) {
        int error = 0;
        socklen_t len = sizeof(error);
#ifdef _WIN32

        getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (char *)&error, &len);
#else
        getsockopt(socket_, SOL_SOCKET, SO_ERROR, (void *)&error, &len);
#endif
        if (error) {
            callback_(callback_data_, NETLIB_MSG_CLOSE, (net_handle_t)socket_,
                      NULL);
        } else {
            state_ = SOCKET_STATE_CONNECTED;
            callback_(callback_data_, NETLIB_MSG_CONFIRM, (net_handle_t)socket_,
                      NULL);
        }
    } else {
        callback_(callback_data_, NETLIB_MSG_WRITE, (net_handle_t)socket_,
                  NULL);
    }
}

void CBaseSocket::OnClose() {
    state_ = SOCKET_STATE_CLOSING;
    callback_(callback_data_, NETLIB_MSG_CLOSE, (net_handle_t)socket_, NULL);
}

void CBaseSocket::SetSendBufSize(uint32_t send_size) {
    int ret = setsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &send_size, 4);
    if (ret == SOCKET_ERROR) {
        printf("set SO_SNDBUF failed for fd=%d", socket_);
    }

    socklen_t len = 4;
    int size = 0;
    getsockopt(socket_, SOL_SOCKET, SO_SNDBUF, &size, &len);
    printf("socket=%d send_buf_size=%d", socket_, size);
}

void CBaseSocket::SetRecvBufSize(uint32_t recv_size) {
    int ret = setsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &recv_size, 4);
    if (ret == SOCKET_ERROR) {
        printf("set SO_RCVBUF failed for fd=%d", socket_);
    }

    socklen_t len = 4;
    int size = 0;
    getsockopt(socket_, SOL_SOCKET, SO_RCVBUF, &size, &len);
    printf("socket=%d recv_buf_size=%d", socket_, size);
}

int CBaseSocket::_GetErrorCode() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

bool CBaseSocket::_IsBlock(int error_code) {
#ifdef _WIN32
    return ((error_code == WSAEINPROGRESS) || (error_code == WSAEWOULDBLOCK));
#else
    return ((error_code == EINPROGRESS) || (error_code == EWOULDBLOCK));
#endif
}

void CBaseSocket::_SetNonblock(SOCKET fd) {
#ifdef _WIN32
    u_long nonblock = 1;
    int ret = ioctlsocket(fd, FIONBIO, &nonblock);
#else
    int ret = fcntl(fd, F_SETFL, O_NONBLOCK | fcntl(fd, F_GETFL));
#endif
    if (ret == SOCKET_ERROR) {
        printf("_SetNonblock failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
    }
}

void CBaseSocket::_SetReuseAddr(SOCKET fd) {
    int reuse = 1;
    int ret =
        setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
    if (ret == SOCKET_ERROR) {
        printf("_SetReuseAddr failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
    }
}

void CBaseSocket::_SetNoDelay(SOCKET fd) {
    int nodelay = 1;
    int ret = setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (char *)&nodelay,
                         sizeof(nodelay));
    if (ret == SOCKET_ERROR) {
        printf("_SetNoDelay failed, err_code=%d, fd=%d", _GetErrorCode(), fd);
    }
}

void CBaseSocket::_SetAddr(const char *ip, const uint16_t port,
                           sockaddr_in *addr) {
    memset(addr, 0, sizeof(sockaddr_in));
    addr->sin_family = AF_INET;
    addr->sin_port = htons(port);
    addr->sin_addr.s_addr = inet_addr(ip);
    if (addr->sin_addr.s_addr == INADDR_NONE) {
        hostent *host = gethostbyname(ip);
        if (host == NULL) {
            printf("gethostbyname failed, ip=%s, port=%u", ip, port);
            return;
        }

        addr->sin_addr.s_addr = *(uint32_t *)host->h_addr;
    }
}

void CBaseSocket::_AcceptNewSocket() {
    SOCKET fd = 0;
    sockaddr_in peer_addr;
    socklen_t addr_len = sizeof(sockaddr_in);
    char ip_str[64];
    while ((fd = accept(socket_, (sockaddr *)&peer_addr, &addr_len)) !=
           INVALID_SOCKET) {
        CBaseSocket *pSocket = new CBaseSocket();
        uint32_t ip = ntohl(peer_addr.sin_addr.s_addr);
        uint16_t port = ntohs(peer_addr.sin_port);

        snprintf(ip_str, sizeof(ip_str), "%d.%d.%d.%d", ip >> 24,
                 (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

        // printf("AcceptNewSocket, socket=%d from %s:%d\n", fd, ip_str, port);

        pSocket->SetSocket(fd);
        pSocket->SetCallback(callback_);
        pSocket->SetCallbackData(callback_data_);
        pSocket->SetState(SOCKET_STATE_CONNECTED);
        pSocket->SetRemoteIP(ip_str);
        pSocket->SetRemotePort(port);

        _SetNoDelay(fd);
        _SetNonblock(fd);
        AddBaseSocket(pSocket);
        CEventDispatch::Instance()->AddEvent(fd, SOCKET_READ | SOCKET_EXCEP);
        callback_(callback_data_, NETLIB_MSG_CONNECT, (net_handle_t)fd, NULL);
    }
}
