#include "api_login.h"
#include "tc_common.h"
#include "util.h"
#define TEST_LOGIN_COUNT 10000

#undef  API_LOGIN_MUTIL_THREAD
void SingeThreadTestLogin() {
    // 设置传递要解析的json数据
    string post_data = "{\"user\": \"darren\", \"pwd\": "
                       "\"e10adc3949ba59abbe56e057f20f883e\"}";
    string url = "/api/login";
    string str_json;

    // 获取开始时间
    uint64_t cur_time = GetTickCount(); // 单位ms
    for (int i = 0; i < TEST_LOGIN_COUNT; i++) {
        ApiUserLoginTest(url, post_data, str_json);
        LogInfo("str_json: {}",str_json);
    }
    // 获取结束时间
    uint64_t end_time = GetTickCount(); // 单位ms
    double one_login_time = (end_time - cur_time) * 1.0 / TEST_LOGIN_COUNT;
    LogInfo("Login: {} times need the time: {}ms, average time: {}ms, qps: {}",
     TEST_LOGIN_COUNT, end_time - cur_time, one_login_time, long(1000 / one_login_time));         
}
