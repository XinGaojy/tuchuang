#include "api_common.h"

string s_dfs_path_client;
// string s_web_server_ip;
// string s_web_server_port;
string s_storage_web_server_ip;
string s_storage_web_server_port;
string s_shorturl_server_address;
string s_shorturl_server_access_token;
// 比如加载mysql相关的记录到redis
// https://blog.csdn.net/lxw1844912514/article/details/121528746
/*对于 Redis 在内存中的数据，需要定期的同步到磁盘中，但对于 Redis
异常重启，就没有办法了。比如在 Redis 中插入后，Redis 重启
，数据没有持久化到硬盘。这时可以在重启 Redis 后，从数据库执行下 count(*)
操作,然后更新到 Redis 中。一次全表扫描还是可行的。*/
template <typename... Args>
std::string FormatString(const std::string &format, Args... args) {
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...) +
                1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(),
                       buf.get() + size - 1); // We don't want the '\0' inside
}
 

int CacheSetCount(CacheConn *cache_conn, string key, int64_t count) {
    string ret = cache_conn->Set(key, std::to_string(count));
    if (!ret.empty()) {
        return 0;
    } else {
        return -1;
    }
}

int CacheGetCount(CacheConn *cache_conn, string key, int64_t &count) {
    count = 0;
    string str_count = cache_conn->Get(key);
    if (!str_count.empty()) {
        count = atoll(str_count.c_str());
        return 0;
    } else {
        return -1;
    }
}

int CacheIncrCount(CacheConn *cache_conn, string key) {
    int64_t count = 0;
    int ret = cache_conn->Incr(key, count);
    if (ret < 0) {
        return -1;
    }
    LogInfo("{}-{}", key, count);
    
    return 0;
}
// 这里最小我们只允许为0
int CacheDecrCount(CacheConn *cache_conn, string key) {
    int64_t count = 0;
    int ret = cache_conn->Decr(key, count);
    if (ret < 0) {
        return -1;
    }
    LogInfo("{}-{}", key, count);
    if (count < 0) {
        LogError("{} 请检测你的逻辑 decr  count < 0  -> {}", key , count);
        ret = CacheSetCount(cache_conn, key, 0); // 文件数量最小为0值
        if (ret < 0) {
            return -1;
        }
    }
    return 0;
}

//获取用户文件个数
int DBGetUserFilesCountByUsername(CDBConn *db_conn, string user_name,
                                  int &count) {
    count = 0;
    int ret = 0;
    // 先查看用户是否存在
    string str_sql;

    str_sql = FormatString("select count(*) from user_file_list where user='%s'", user_name.c_str());
    LogInfo("执行: {}", str_sql);
    CResultSet *result_set = db_conn->ExecuteQuery(str_sql.c_str());
    if (result_set && result_set->Next()) {
        // 存在在返回
        count = result_set->GetInt("count(*)");
        LogInfo("count: {}", count);
        ret = 0;
        delete result_set;
    } else if (!result_set) { // 操作失败
        LogError("{} 操作失败", str_sql);
        LogError("{} 操作失败", str_sql);
        ret = -1;
    } else {
        // 没有记录则初始化记录数量为0
        ret = 0;
        LogInfo("没有记录: count: {}", count);
    }
    return ret;
}
 
 