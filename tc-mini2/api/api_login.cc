#include "api_login.h"

 
 
#define LOGIN_RET_OK 0   // 成功
#define LOGIN_RET_FAIL 1 // 失败

// 解析登录信息
int decodeLoginJson(const std::string &str_json, string &user_name,
                    string &pwd) {
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res) {
        LogError("parse reg json failed ");
        return -1;
    }
    // 用户名
    if (root["user"].isNull()) {
        LogError("user null");
        return -1;
    }
    user_name = root["user"].asString();

    //密码
    if (root["pwd"].isNull()) {
        LogError("pwd null");
        return -1;
    }
    pwd = root["pwd"].asString();

    return 0;
}

// 封装登录结果的json
int encodeLoginJson(int ret, string &token, string &str_json) {
    Json::Value root;
    root["code"] = ret;
    if (ret == 0) {
        root["token"] = token; // 正常返回的时候才写入token
    }
    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}


/* -------------------------------------------*/
/**
 * @brief  判断用户登陆情况
 *
 * @param username 		用户名
 * @param pwd 		密码
 *
 * @returns
 *      成功: 0
 *      失败：-1
 */
/* -------------------------------------------*/
int verifyUserPassword(string &user_name, string &pwd) {
    int ret = 0;
    CDBManager *db_manager = CDBManager::getInstance();
    CDBConn *db_conn = db_manager->GetDBConn("tuchuang_slave");
    AUTO_REL_DBCONN(db_manager, db_conn);

    // 先查看用户是否存在
    string strSql;

    strSql = FormatString("select password from user_info where user_name='%s'", user_name.c_str());
    CResultSet *result_set = db_conn->ExecuteQuery(strSql.c_str());
    uint32_t nowtime = time(NULL);
    if (result_set && result_set->Next()) {
        // 存在在返回
        string password = result_set->GetString("password");
        LogInfo("mysql-pwd: {}, user-pwd: {}", password, pwd);
        if (result_set->GetString("password") == pwd)
            ret = 0;
        else
            ret = -1;
    } else { // 如果不存在则注册
        ret = -1;
    }

    delete result_set;

    return ret;
}

/* -------------------------------------------*/
/**
 * @brief  生成token字符串, 保存redis数据库
 *
 * @param username 		用户名
 * @param token     生成的token字符串
 *
 * @returns
 *      成功: 0
 *      失败：-1
 */
/* -------------------------------------------*/
int setToken(string &user_name, string &token) {
    int ret = 0;
    CacheManager *cache_manager = CacheManager::getInstance();
    CacheConn *cache_conn = cache_manager->GetCacheConn("token");
    AUTO_REL_CACHECONN(cache_manager, cache_conn);

    token = RandomString(32); // 随机32个字母

    if (cache_conn) {
        //用户名：token, 86400有效时间为24小时
        cache_conn->SetEx(user_name, 86400, token); // redis做超时
    } else {
        ret = -1;
    }

    return ret;
}
 
int ApiUserLogin(string &post_data, string &resp_json) {
    string user_name;
    string pwd;
    string token;

    // 判断数据是否为空
    if (post_data.empty()) {
        return -1;
    }
    // 解析json
    if (decodeLoginJson(post_data, user_name, pwd) < 0) {
        LogError("decodeRegisterJson failed");
        encodeLoginJson(1, token, resp_json);
        return -1;
    }

    // 验证账号和密码是否匹配
    if (verifyUserPassword(user_name, pwd) < 0) {
        LogError("verifyUserPassword failed");
        encodeLoginJson(1, token, resp_json);
        return -1;
    }

    // 生成token并存储在redis
    if (setToken(user_name, token) < 0) {
        LogError("setToken failed");
        encodeLoginJson(1, token, resp_json);
        return -1;
    } else {
        encodeLoginJson(0, token, resp_json);
        return 0;
    }
}
 
 