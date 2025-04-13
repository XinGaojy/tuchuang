#include "api_register.h"
 

//解析用户注册信息的json包
/*json数据如下
    {
        userName:xxxx,
        nickName:xxx,
        firstPwd:xxx,
        phone:xxx,
        email:xxx
    }
    */
int decodeRegisterJson(const std::string &str_json, string &user_name,
                       string &nick_name, string &pwd, string &phone,
                       string &email) {
    bool res;
    Json::Value root;
    Json::Reader jsonReader;
    res = jsonReader.parse(str_json, root);
    if (!res) {
        LogError("parse reg json failed ");
        return -1;
    }

    // 用户名
    if (root["userName"].isNull()) {
        LogError("userName null\n");
        return -1;
    }
    user_name = root["userName"].asString();

    // 昵称
    if (root["nickName"].isNull()) {
        LogError("nickName null\n");
        return -1;
    }
    nick_name = root["nickName"].asString();

    //密码
    if (root["firstPwd"].isNull()) {
        LogError("firstPwd null\n");
        return -1;
    }
    pwd = root["firstPwd"].asString();

    //电话  非必须
    if (root["phone"].isNull()) {
        LogWarn("phone null\n");
    } else {
        phone = root["phone"].asString();
    }

    //邮箱 非必须
    if (root["email"].isNull()) {
        LogWarn("email null\n");
    } else {
        email = root["email"].asString();
    }

    return 0;
}

// 封装注册用户的json
int encodeRegisterJson(int ret, string &str_json) {
    Json::Value root;
    root["code"] = ret;
    Json::FastWriter writer;
    str_json = writer.write(root);
    return 0;
}

int registerUser(string &user_name, string &nick_name, string &pwd,
                 string &phone, string &email) {
    int ret = 0;
    uint32_t user_id;
    CDBManager *db_manager = CDBManager::getInstance();
    CDBConn *db_conn = db_manager->GetDBConn("tuchuang_master");
    AUTO_REL_DBCONN(db_manager, db_conn);       //栈上构建一个对象 退出的时候自动把连接规划连接池
    // 先查看用户是否存在
    string str_sql = FormatString("select * from user_info where user_name='%s'", user_name.c_str());
    LogInfo("执行: {}", str_sql);
    CResultSet *result_set = db_conn->ExecuteQuery(str_sql.c_str());
    if (result_set && result_set->Next()) { // 检测是否存在用户记录
        // 存在在返回
        LogWarn("id: {}, user_name: {}  已经存在", result_set->GetInt("id"), result_set->GetString("user_name"));
        delete result_set;
        ret = 2;
    } else { // 如果不存在则注册
        time_t now;
        char create_time[TIME_STRING_LEN];
        //获取当前时间
        now = time(NULL);
        strftime(create_time, TIME_STRING_LEN - 1, "%Y-%m-%d %H:%M:%S",
                 localtime(&now));
        str_sql = "insert into user_info "
                 "(`user_name`,`nick_name`,`password`,`phone`,`email`,`create_"
                 "time`) values(?,?,?,?,?,?)";
        LogInfo("执行: {}", str_sql);
        // mysql操作 如果不熟悉可以参考：https://www.yuque.com/linuxer/linux_senior/rcz4xl?singleDoc# 《mysql api c客户端》
         CPrepareStatement *stmt = new CPrepareStatement();
        if (stmt->Init(db_conn->GetMysql(), str_sql)) {
            uint32_t index = 0;
            string c_time = create_time;
            stmt->SetParam(index++, user_name);
            stmt->SetParam(index++, nick_name);
            stmt->SetParam(index++, pwd);
            stmt->SetParam(index++, phone);
            stmt->SetParam(index++, email);
            stmt->SetParam(index++, c_time);
            bool bRet = stmt->ExecuteUpdate();
            if (bRet) {
                ret = 0;
                user_id = db_conn->GetInsertId();
                LogInfo("insert user_id: {}", user_id); //用户id是自增id
            } else {
                LogError("insert user_info failed. {}", str_sql);
                ret = 1;
            }
        }
        delete stmt;
    }

    return ret;
}

 
int ApiRegisterUser(string &post_data, string &resp_json) {
     int ret = 0;
    string user_name;
    string nick_name;
    string pwd;
    string phone;
    string email;

    LogInfo("post_data: {}", post_data);

    // 判断数据是否为空
    if (post_data.empty()) {
        LogError("decodeRegisterJson failed");
        // 封装注册结果
        encodeRegisterJson(1, resp_json);
        return -1;
    }
    // 解析json
    if (decodeRegisterJson(post_data, user_name, nick_name, pwd, phone, email) < 0) {
        LogError("decodeRegisterJson failed");
        // 封装注册结果
        encodeRegisterJson(1, resp_json);
        return -1;
    }

    // 注册账号
    ret = registerUser(user_name, nick_name, pwd, phone, email);
 
    // 封装注册结果
    ret = encodeRegisterJson(ret, resp_json);

    return 0;
}