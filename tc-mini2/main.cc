/**
 * 头文件包含规范
 * 1.本类的声明（第一个包含本类h文件，有效减少以来）
 * 2.C系统文件
 * 3.C++系统文件
 * 4.其他库头文件
 * 5.本项目内头文件
 */

// using std::string; // 可以在整个cc文件和h文件内使用using， 禁止使用using
// namespace xx污染命名空间

#include "api_common.h"
#include "config_file_reader.h"
#include "http_conn.h"
#include "netlib.h"
#include "util.h"
#include "dlog.h"
 
void http_callback(void *callback_data, uint8_t msg, uint32_t socket_handle,
                   void *pParam) {
    UNUSED(callback_data);
    UNUSED(pParam);
    if (msg == NETLIB_MSG_CONNECT) {
        // 这里是不是觉得很奇怪,为什么new了对象却没有释放?
        // 实际上对象在被Close时使用delete this的方式释放自己
        CHttpConn *pConn = new CHttpConn();
        pConn->OnConnect(socket_handle);
    } else {
        LogError("!!!error msg:{}", msg);
    }
}


int main(int argc, char *argv[]) {
    // 默认情况下，往一个读端关闭的管道或socket连接中写数据将引发SIGPIPE信号。我们需要在代码中捕获并处理该信号，
    // 或者至少忽略它，因为程序接收到SIGPIPE信号的默认行为是结束进程，而我们绝对不希望因为错误的写操作而导致程序退出。
    // SIG_IGN 忽略信号的处理程序
    signal(SIGPIPE, SIG_IGN); //忽略SIGPIPE信号
    int ret = 0;
    // 1.获取配置文件路径
    char *str_tc_http_server_conf = NULL;
    if(argc > 1) {
        str_tc_http_server_conf = argv[1];  // 指向配置文件路径
    } else {
        str_tc_http_server_conf = (char *)"tc_http_server.conf";
    }

    printf("conf file path: %s\n", str_tc_http_server_conf);
 
    // 2.读取配置文件
    //2.1 解析配置文件
    CConfigFileReader config_file;     //读取配置文件
    if(config_file.ParseConf(str_tc_http_server_conf) != 0) {
        std::cout << str_tc_http_server_conf << " no exist, please check conf file\n";
        return -1;
    }
    // 2.2读取配置文件字段
    // 日志级别
    char *log_level = config_file.GetConfigName("log_level");   //读取日志设置级别
    if (!log_level) {
        LogError("config item missing, exit... log_level:{}", log_level);
        return -1;
    }
    DLog::SetLevel(log_level);   //设置日志打印级别

    // http监听地址和端口
    char *http_listen_ip = config_file.GetConfigName("HttpListenIP");
    char *str_http_port = config_file.GetConfigName("HttpPort");        //8081 -- nginx.conf,当前服务的端口
    // 检测监听ip和端口是否存在
    if (!http_listen_ip || !str_http_port) {
        LogError("config item missing, exit... ip:{}, port:{}", http_listen_ip, str_http_port);
        return -1;
    }
    uint16_t http_port = atoi(str_http_port);

    // 打印提示而已
    LogInfo("main into"); //单例模式 日志库 spdlog
 
    //3.初始化mysql连接池，内部也会读取读取配置文件tc_http_server.conf
    CacheManager::SetConfPath(str_tc_http_server_conf); //设置配置文件路径
    CacheManager *cache_manager = CacheManager::getInstance();
    if (!cache_manager) {
        LogError("CacheManager init failed");
        return -1;
    }
    //4.初始化redis连接池，内部也会读取读取配置文件tc_http_server.conf
    CDBManager::SetConfPath(str_tc_http_server_conf);   //设置配置文件路径
    CDBManager *db_manager = CDBManager::getInstance();
    if (!db_manager) {
        LogError("DBManager init failed");
        return -1;
    }
    
    //5.reactor网络模型, 这里只对Windows系统有作用，Linux环境调用这个函数实际没有做什么
    ret = netlib_init();
    if (ret == NETLIB_ERROR) {
        LogError("netlib_init failed");
        return ret;
    }
    
    //6.监听指定的IP和端口，并绑定accept新连接后的回调函数http_callback, 这里主要是我们http端口
    ret = netlib_listen(http_listen_ip, http_port, http_callback, NULL);
    if (ret == NETLIB_ERROR) {
        LogError("listen {}:{} failed", http_listen_ip, http_port);
        return ret;
    }
        
    LogInfo("server start listen on:For http://{}:{}", http_listen_ip, http_port);

    LogInfo("now enter the event loop...");
    //7.将当前进程id写入文件server.pid, 可以直接cat这个文件获取进程id
    WritePid();
    //8.进入epoll_wait触发的循环流程  
    netlib_eventloop(1);

    return 0;
}
