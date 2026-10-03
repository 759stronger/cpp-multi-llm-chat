#include <spdlog/spdlog.h>
#include <mutex>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async.h>

// 日志记录器
namespace my_logger {
    // 日志记录器
    class Logger {
    public:
        // 初始化日志记录器
        static void init_logger(const std::string& logger_name , const std::string& logger_file , spdlog::level::level_enum logger_level);    
        // 获取日志记录器   
        static std::shared_ptr<spdlog::logger> get_logger();
        
    private:
    // 日志记录器构造函数
    // 禁用复制构造函数和赋值运算符
            Logger ();
            Logger (const Logger& )=delete;
            Logger& operator=(const Logger& )=delete;


    private:
    // 日志记录器成员变量
    // 日志记录器
    // 日志记录器互斥锁
        static std::shared_ptr<spdlog::logger> _logger;
        static std::mutex _mutex;

    };

}
// 日志记录器宏定义
// 日志记录器跟踪日志
#define TRACE(fmt, ...) my_logger::Logger::get_logger()->trace(std::string("[{:>10s}:{:<4d}]")+fmt,__FILE__,__LINE__, ##__VA_ARGS__)
// 日志记录器调试日志
#define DBG(fmt, ...) my_logger::Logger::get_logger()->debug(std::string("[{:>10s}:{:<4d}]")+fmt,__FILE__,__LINE__, ##__VA_ARGS__)
// 日志记录器错误日志
#define ERR(fmt, ...) my_logger::Logger::get_logger()->error(std::string("[{:>10s}:{:<4d}]")+fmt,__FILE__,__LINE__, ##__VA_ARGS__)
// 日志记录器信息日志
#define INFO(fmt, ...) my_logger::Logger::get_logger()->info(std::string("[{:>10s}:{:<4d}]")+fmt,__FILE__,__LINE__, ##__VA_ARGS__)
// 日志记录器警告日志
#define WARN(fmt, ...) my_logger::Logger::get_logger()->warn(std::string("[{:>10s}:{:<4d}]")+fmt,__FILE__,__LINE__, ##__VA_ARGS__)
// 日志记录器严重错误日志
#define CRIT(fmt, ...) my_logger::Logger::get_logger()->critical(std::string("[{:>10s}:{:<4d}]")+fmt,__FILE__,__LINE__, ##__VA_ARGS__)


