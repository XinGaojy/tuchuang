#ifndef LOG_H
#define LOG_H
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/daily_file_sink.h>
#include <spdlog/async.h>
#include "spdlog/sinks/stdout_color_sinks.h"
#ifndef SPDLOG_TRACE_ON
#define SPDLOG_TRACE_ON
#endif

#ifndef SPDLOG_DEBUG_ON
#define SPDLOG_DEBUG_ON
#endif

// #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE //必须定义这个宏,才能输出文件名和行号

class DLog
{
public:
    static  DLog* GetInstance()   //单例模式，即使函数被多次调用，也只能返回一个实例
    {
        static DLog dlogger;
        return &dlogger;
    }
    std::shared_ptr<spdlog::logger> getLogger()
	{
		return log_;
	}
    // trace debug info warn err critical off
    static void SetLevel(char *log_level);
    // {
    //     if(strcmp(log_level, "trace") == 0) {
    //         level_ =  spdlog::level::trace;
    //     }else if(strcmp(log_level, "debug") == 0) {
    //         level_ =  spdlog::level::debug;
    //     }else if(strcmp(log_level, "info") == 0) {
    //         level_ =  spdlog::level::info;
    //     }else if(strcmp(log_level, "warn") == 0) {
    //         level_ =  spdlog::level::warn;
    //     }else if(strcmp(log_level, "err") == 0) {
    //         level_ =  spdlog::level::err;
    //     }else if(strcmp(log_level, "critical") == 0) {
    //         level_ =  spdlog::level::critical;
    //     }else if(strcmp(log_level, "off") == 0) {
    //         level_ =  spdlog::level::off;
    //     } else {
    //         printf("level: %s is invalid\n", log_level);
    //     }
    // }
private:
    DLog(/* args */) {
        // spdlog::set_pattern("[%Y-%m-%d %H:%M:%S %e] [%n] [%l] [process %P] [thread %t] %v");
        // 创建一个包含多个日志 sink 的列表
        std::vector<spdlog::sink_ptr> sinkList;   //sinkList 将用于存储多个日志输出的目标（sink）。
    #if 1  //输出日志到控制台
        auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();     //stdout_color_sink_mt 是一个输出到控制台的 sink，带有颜色支持
        consoleSink->set_level(level_);
        //consoleSink->set_pattern("[multi_sink_example] [%^%l%$] %v");
        //consoleSink->set_pattern("[%m-%d %H:%M:%S.%e][%^%L%$]  %v");
        // consoleSink->set_pattern("%Y-%m-%d %H:%M:%S [%l] [%t] - <%s>|<%#>|<%!>,%v");
        // consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S %e] [%n]  [thread %t] %v");
        consoleSink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");   //这个是日志的输出格式
        sinkList.push_back(consoleSink);  //将 consoleSink 对象添加到 sinkList 中
    #endif
        // 输出日志到文件
        auto dailySink = std::make_shared<spdlog::sinks::daily_file_sink_mt>("logs/daily.log", 23, 59);
        dailySink->set_level(level_);
        // dailySink->set_pattern("[%Y-%m-%d %H:%M:%S %e] [%n] [%l] [process %P] [thread %t] %v");
        dailySink->set_pattern("[%Y-%m-%d %H:%M:%S.%e][thread %t][%@,%!][%l] : %v");
        sinkList.push_back(dailySink);

        log_ = std::make_shared<spdlog::logger>("both", begin(sinkList), end(sinkList));   //创建一个新的 spdlog::logger 对象，名称为 "both"，并将 sinkList 中的所有 sink 传递给它。这将使得日志同时输出到控制台和文件中。
        //register it if you need to access it globally
        spdlog::register_logger(log_);   //将创建的 log_ 对象注册到 spdlog 的全局日志管理器中，这样可以在全局范围内访问这个日志记录器
 
        // spdlog::set_pattern("[%Y-%m-%d %H:%M:%S %e] [%n] [%l] [process %P] [thread %t] %v");
        // log_ = spdlog::daily_logger_mt<spdlog::async_factory>("daily_logger", "logs/daily.txt", 0, 1);
        spdlog::flush_every(std::chrono::seconds(1));   //每隔1秒，舒心积攒的日志到输出端
    }
    ~DLog() { }

private:
    std::shared_ptr<spdlog::logger> log_;
    // static spdlog::level::level_enum level_ = spdlog::level::info;
    static spdlog::level::level_enum level_;
};

//用定义的方式调用C++标准库spdlog里面的接口，记录不同级别的日志信息
#define LogTrace(...) SPDLOG_LOGGER_CALL(DLog::GetInstance()->getLogger().get(), spdlog::level::trace, __VA_ARGS__)
#define LogDebug(...) SPDLOG_LOGGER_CALL(DLog::GetInstance()->getLogger().get(), spdlog::level::debug, __VA_ARGS__)
#define LogInfo(...) SPDLOG_LOGGER_CALL(DLog::GetInstance()->getLogger().get(), spdlog::level::info, __VA_ARGS__)
#define LogWarn(...) SPDLOG_LOGGER_CALL(DLog::GetInstance()->getLogger().get(), spdlog::level::warn, __VA_ARGS__)
#define LogError(...) SPDLOG_LOGGER_CALL(DLog::GetInstance()->getLogger().get(), spdlog::level::err, __VA_ARGS__)
#define LogCritical(...) SPDLOG_LOGGER_CALL(DLog::GetInstance()->getLogger().get(), spdlog::level::critical, __VA_ARGS__)
#endif