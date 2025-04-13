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

#include "cache_pool.h"
#include "config_file_reader.h"
#include "db_pool.h"
#include "netlib.h"
#include "util.h"

#include "api_dealfile.h"
#include "api_upload.h"
#include "dlog.h"

extern void SingeThreadTestLogin();
extern void SingeThreadRegister();
 
// 有索引：./tc_test tc_http_server.conf 
// 无索引：./tc_test tc_http_server_no_index.conf 
int main(int argc, char *argv[]) {
    char *str_tc_http_server_conf = NULL;
    if(argc > 1) {
        str_tc_http_server_conf = argv[1];  // 指向配置文件路径
    } else {
        str_tc_http_server_conf = (char*) "tc_http_server.conf";
    }

    printf("conf file path: %s\n", str_tc_http_server_conf);
    
     // 读取配置文件
    CConfigFileReader config_file(str_tc_http_server_conf);
    char *log_level = config_file.GetConfigName("log_level");
    DLog::SetLevel(log_level);  //设置日志打印级别

    // 初始化mysql、redis连接池，内部也会读取读取配置文件tc_http_server.conf
    CacheManager::SetConfPath(str_tc_http_server_conf);
    CacheManager *cache_manager = CacheManager::getInstance();
    if (!cache_manager) {
        LogError("CacheManager init failed");
        return -1;
    }
    CDBManager::SetConfPath(str_tc_http_server_conf);
    CDBManager *db_manager = CDBManager::getInstance();
    if (!db_manager) {
        LogError("DBManager init failed");
        return -1;
    }

    char *dfs_path_client = config_file.GetConfigName("dfs_path_client");
    char *storage_web_server_ip =
        config_file.GetConfigName("storage_web_server_ip");
    char *storage_web_server_port =
        config_file.GetConfigName("storage_web_server_port");

    // 将配置文件的参数传递给对应模块
    ApiUploadInit(dfs_path_client, storage_web_server_ip, storage_web_server_port, "", "");
    ApiDealfileInit(dfs_path_client);
    
    printf("login test\n");
    // SingeThreadTestLogin();
    printf("register test\n");
    SingeThreadRegister();
    printf("main end\n");
    return 0;
}
