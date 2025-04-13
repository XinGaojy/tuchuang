#include "api_login.h"
#include "tc_common.h"
#include "util.h"

#include <chrono>
#include <functional>
#include <iomanip>
#include <iostream>
#include <random>
#include <sstream>

//定义执行记录数量
#define TEST_REGISTER_COUNT 5000

// truncate table tablename;

std::string create_uuid() // 这个也不是uuid
{
    std::stringstream stream;
    auto random_seed =
        std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 seed_engine(random_seed);
    std::uniform_int_distribution<std::size_t> random_gen;
    std::size_t value = random_gen(seed_engine);
    stream << std::hex << value;

    return stream.str() + RandomString(16);
}

template <typename... Args>
std::string formatString3(const std::string &format, Args... args) {
    auto size = std::snprintf(nullptr, 0, format.c_str(), args...) +
                1; // Extra space for '\0'
    std::unique_ptr<char[]> buf(new char[size]);
    std::snprintf(buf.get(), size, format.c_str(), args...);
    return std::string(buf.get(),
                       buf.get() + size - 1); // We don't want the '\0' inside
};

void SingeThreadRegister() {
    int ret = 0;
    CDBManager *db_manager = CDBManager::getInstance();
    CDBConn *db_conn = db_manager->GetDBConn("tuchuang_slave");
    AUTO_REL_DBCONN(db_manager, db_conn);
    string user_name;
    string nick_name;
    string pwd = "e10adc3949ba59abbe56e057f20f883e";
    string phone = "18588888888";
    string email = "326873713@qq.com";
    uint32_t user_id;
    string str_sql;
    time_t now;
    char create_time[TIME_STRING_LEN];
    std::vector<string> user_name_vector;
    uint64_t cur_time;
    uint64_t end_time;
    double one_time = 0;

    // 100000
    // 清空表
    // 随机插入10万条语句，计算插入时间
    // 获取开始时间
    cur_time = GetTickCount(); // 单位ms
    for (int i = 0; i < TEST_REGISTER_COUNT; i++) {
        user_name = create_uuid();
        nick_name = user_name;
        //获取当前时间
        now = time(NULL);
        strftime(create_time, TIME_STRING_LEN - 1, "%Y-%m-%d %H:%M:%S",
                 localtime(&now));
        str_sql = "insert into user_info "
                 "(`user_name`,`nick_name`,`password`,`phone`,`email`,`create_"
                 "time`) values(?,?,?,?,?,?)";
        LogDebug("执行: ", str_sql);
        // 必须在释放连接前delete
        // CPrepareStatement对象，否则有可能多个线程操作mysql对象，会crash
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
                user_id = db_conn->GetInsertId();
                LogDebug("insert user {}", user_id);
            } else {
                LogError("insert user_info failed. {}", str_sql);
            }
        }
        delete stmt;
        if (i < 1000) {
            user_name_vector.push_back(move(user_name));
        }
    }
    // 获取结束时间
    end_time = GetTickCount(); // 单位ms
    one_time = (end_time - cur_time) * 1.0 / TEST_REGISTER_COUNT;
    LogInfo("register: {} times need the time: {}ms, average time: {}ms, qps: {}",
        TEST_REGISTER_COUNT, end_time - cur_time, one_time, long(1000 / one_time));  

    // 查询 10万次数据计算耗时
    cur_time = GetTickCount(); // 单位ms
    int select_count = user_name_vector.size();
    for (int i = 0; i < select_count; i++) {
        str_sql =
            formatString3("select password from user_info where user_name='%s'",
                          user_name_vector[0].c_str());
        CResultSet *result_set = db_conn->ExecuteQuery(str_sql.c_str());
        if (result_set && result_set->Next()) {
            // 存在在返回
            string password = result_set->GetString("password");
        } else { // 如果不存在则注册
            ret = -1;
        }
    }

    end_time = GetTickCount(); // 单位ms
    one_time = (end_time - cur_time) * 1.0 / select_count;

    LogInfo("select: {} times need the time: {}ms, average time: {}ms, qps: {}",
        select_count, end_time - cur_time, one_time, long(1000 / one_time));              
}
