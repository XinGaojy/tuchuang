#include "dlog.h"

spdlog::level::level_enum DLog::level_ = spdlog::level::info; // 默认使用info级别

// DLog* DLog::GetInstance(){
//     static DLog dlogger;
//     return &dlogger;
// }
// std::shared_ptr<spdlog::logger> DLog::getLogger()
// {
//     return log_;
// }
// trace debug info warn err critical off
void DLog::SetLevel(char *log_level) {
    printf("SetLevel log_level:%s\n", log_level);
    fflush(stdout);
    if(strcmp(log_level, "trace") == 0) {
        level_ =  spdlog::level::trace;
    }else if(strcmp(log_level, "debug") == 0) {
        level_ =  spdlog::level::debug;
    }else if(strcmp(log_level, "info") == 0) {
        level_ =  spdlog::level::info;
    }else if(strcmp(log_level, "warn") == 0) {
        level_ =  spdlog::level::warn;
    }else if(strcmp(log_level, "err") == 0) {
        level_ =  spdlog::level::err;
    }else if(strcmp(log_level, "critical") == 0) {
        level_ =  spdlog::level::critical;
    }else if(strcmp(log_level, "off") == 0) {
        level_ =  spdlog::level::off;
    } else {
        printf("level: %s is invalid\n", log_level);
    }
}