#include "api_myfiles.h"


//解析的json包, 登陆token

int decodeCountJson(string &str_json, string &user_name, string &token) {
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res) {
        LogError("parse reg json failed ");
        return -1;
    }
    int ret = 0;

    // 用户名
    if (root["user"].isNull()) {
        LogError("user null\n");
        return -1;
    }
    user_name = root["user"].asString();

    //密码
    if (root["token"].isNull()) {
        LogError("token null\n");
        return -1;
    }
    token = root["token"].asString();

    return ret;
}

int encodeCountJson(int ret, int total, string &str_json) {
    Json::Value root;
    root["code"] = ret;
    if (ret == 0) {
        root["total"] = total; // 正常返回的时候才写入token
    }
    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}

//获取用户文件个数
int getUserFilesCount(CDBConn *db_conn, CacheConn *cache_conn,
                      string &user_name, int &count) {
    int ret = 0;

    // 从mysql加载
    count = 0;
    if (DBGetUserFilesCountByUsername(db_conn, user_name, count) < 0) {
        LogError("DBGetUserFilesCountByUsername failed");
        return -1;
    }
        
    return ret;
}

int handleUserFilesCount(string &user_name, int &count) {
    CDBManager *db_manager = CDBManager::getInstance();
    CDBConn *db_conn = db_manager->GetDBConn("tuchuang_slave");
    AUTO_REL_DBCONN(db_manager, db_conn);
    CacheManager *cache_manager = CacheManager::getInstance();
    CacheConn *cache_conn = cache_manager->GetCacheConn("token");
    AUTO_REL_CACHECONN(cache_manager, cache_conn);

    int ret = getUserFilesCount(db_conn, cache_conn, user_name, count);
    return ret;
}

//解析的json包
// 参数
// {
// "count": 2,
// "start": 0,
// "token": "3a58ca22317e637797f8bcad5c047446",
// "user": "qingfu"
// }
int decodeFileslistJson(string &str_json, string &user_name, string &token,
                        int &start, int &count) {
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res) {
        LogError("parse reg json failed ");
        return -1;
    }
    int ret = 0;

    // 用户名
    if (root["user"].isNull()) {
        LogError("user null\n");
        return -1;
    }
    user_name = root["user"].asString();

    //密码
    if (root["token"].isNull()) {
        LogError("token null\n");
        return -1;
    }
    token = root["token"].asString();

    if (root["start"].isNull()) {
        LogError("start null\n");
        return -1;
    }
    start = root["start"].asInt();

    if (root["count"].isNull()) {
        LogError("count null\n");
        return -1;
    }
    count = root["count"].asInt();

    return ret;
}

template <typename... Args>
static std::string FormatString(const std::string &format, Args... args) {
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...) + 1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(),  buf.get() + size - 1); // We don't want the '\0' inside
}


int getUserFileList(string &cmd, string &user_name, int &start, int &count,
                    string &str_json) {
    LogInfo("getUserFileList into");
    int ret = 0;
    int total = 0;
    string str_sql;
    CDBManager *db_manager = CDBManager::getInstance();
    CDBConn *db_conn = db_manager->GetDBConn("tuchuang_slave");
    AUTO_REL_DBCONN(db_manager, db_conn);
    CacheManager *cache_manager = CacheManager::getInstance();
    CacheConn *cache_conn = cache_manager->GetCacheConn("token");
    AUTO_REL_CACHECONN(cache_manager, cache_conn);

    ret = getUserFilesCount(db_conn, cache_conn, user_name,
                            total); // 总共的文件数量
    if (ret < 0) {
        LogError("getUserFilesCount failed");
        return -1;
    } else {
        if (total == 0) {
            Json::Value root;
            root["code"] = 0;
            root["count"] = 0;
            root["total"] = 0;
            Json::FastWriter writer;
            str_json = writer.write(root);
            LogWarn("getUserFilesCount = 0");
            return 0;
        }
    }

    //多表指定行范围查询
    if (cmd == "normal") //获取用户文件信息
    {
        str_sql = FormatString(
            "select user_file_list.*, file_info.url, file_info.size,  file_info.type from file_info, user_file_list where user = '%s' \
            and file_info.md5 = user_file_list.md5 limit %d, %d",
            user_name.c_str(), start, count);
    } else {
        LogError("unknown cmd: {}", cmd);
        return -1;
    }

    LogInfo("执行: {}", str_sql);
    CResultSet *result_set = db_conn->ExecuteQuery(str_sql.c_str());
    if (result_set) {
        // 遍历所有的内容
        // 获取大小
        int file_index = 0;
        Json::Value root, files;
        root["code"] = 0;
        while (result_set->Next()) {
            Json::Value file;
            file["user"] = result_set->GetString("user");
            file["md5"] = result_set->GetString("md5");
            file["create_time"] = result_set->GetString("create_time");
            file["file_name"] = result_set->GetString("file_name");
            file["share_status"] = result_set->GetInt("shared_status");
            file["pv"] = result_set->GetInt("pv");
            file["url"] = result_set->GetString("url");
            file["size"] = result_set->GetInt("size");
            file["type"] = result_set->GetString("type");
            files[file_index] = file;
            file_index++;
        }
        root["files"] = files;
        root["count"] = file_index;
        root["total"] = total;
        Json::FastWriter writer;
        str_json = writer.write(root);
        delete result_set;
        return 0;
    } else {
        LogError("{} 操作失败", str_sql);
        return -1;
    }
}
 
int ApiMyfiles(string &url, string &post_data, string &resp_json) {
    // 解析url有没有命令
    // count 获取用户文件个数
    // display 获取用户文件信息，展示到前端
    char cmd[20];
    string user_name;
    string token;
    int ret = 0;
    int start = 0; //文件起点
    int count = 0; //文件个数

    //解析命令 解析url获取自定义参数
    QueryParseKeyValue(url.c_str(), "cmd", cmd, NULL);
    LogInfo("url: {}, cmd: {} ",url, cmd);

    if (strcmp(cmd, "count") == 0) {
        // 解析json
        if (decodeCountJson(post_data, user_name, token) < 0) {
            encodeCountJson(1, 0, resp_json);
            LogError("decodeCountJson failed");
            return -1;
        }
        //验证登陆token，成功返回0，失败-1
        ret = VerifyToken(user_name, token); // util_cgi.h
        if (ret == 0) {
            // 获取文件数量
            if (handleUserFilesCount(user_name, count) < 0) { //获取用户文件个数
                LogError("handleUserFilesCount failed");
                encodeCountJson(1, 0, resp_json);
            } else {
                LogInfo("handleUserFilesCount ok, count: {}", count);
                encodeCountJson(0, count, resp_json);
            }
        } else {
            LogError("VerifyToken failed");
            encodeCountJson(1, 0, resp_json);
        }
        return 0;
    } else {
        if ((strcmp(cmd, "normal") != 0) && (strcmp(cmd, "pvasc") != 0) &&
            (strcmp(cmd, "pvdesc") != 0)) {
            LogError("unknow cmd: {}", cmd);
            encodeCountJson(1, 0, resp_json);
        }
        //获取用户文件信息 127.0.0.1:80/api/myfiles&cmd=normal
        //按下载量升序 127.0.0.1:80/api/myfiles?cmd=pvasc
        //按下载量降序127.0.0.1:80/api/myfiles?cmd=pvdesc
        ret = decodeFileslistJson(post_data, user_name, token, start,
                                  count); //通过json包获取信息
        LogInfo("user_name: {}, token:{}, start: {}, count: {}", user_name,token, start, count);
        if (ret == 0) {
            //验证登陆token，成功返回0，失败-1
            ret = VerifyToken(user_name, token); // util_cgi.h
            if (ret == 0) {
                string str_cmd = cmd;
                if (getUserFileList(str_cmd, user_name, start, count,
                                    resp_json) < 0) { //获取用户文件列表
                    LogError("getUserFileList failed");
                    encodeCountJson(1, 0, resp_json);
                }
            } else {
                LogError("VerifyToken failed");
                encodeCountJson(1, 0, resp_json);
            }
        } else {
            LogError("decodeFileslistJson failed");
            encodeCountJson(1, 0, resp_json);
        }
    }

    return 0;
}
 