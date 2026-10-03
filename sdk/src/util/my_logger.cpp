#include <ai_chat_sdk/util/my_logger.h>
namespace my_logger {
    // 日志记录器成员变量
    std::shared_ptr<spdlog::logger> Logger::_logger = nullptr;
    // 日志记录器互斥锁
    std::mutex Logger::_mutex;
    // 日志记录器
    Logger::Logger () {}

    // 日志记录器初始化函数
    void Logger::init_logger(const std::string& logger_name , const std::string& logger_file , spdlog::level::level_enum logger_level) {
        // 初始化日志记录器
        if(_logger == nullptr) {
            
            std::lock_guard<std::mutex> lock(_mutex);// 加锁
          
            if(_logger == nullptr) {// 初始化日志记录器
               
                spdlog::flush_on(logger_level);// 初始化日志记录器
               
                spdlog::init_thread_pool(32768,1);// 初始化线程池

                if(logger_file=="stdout") { 
                    
                    _logger = spdlog::stdout_color_mt(logger_name);
                } else {
                    _logger = spdlog::basic_logger_mt<spdlog::async_factory>(logger_name,logger_file);
                }

                // 设置日志格式和级别   
                _logger->set_pattern("%H:%M:%S [%n][%-7l]%v");
                // 设置日志级别
                _logger->set_level(logger_level);
               
            }
           
        }
      
    }

    // 日志记录器获取函数
    std::shared_ptr<spdlog::logger> Logger::get_logger() {
        return _logger;
    }
   
}