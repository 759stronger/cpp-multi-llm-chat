#include <gflags/gflags.h>
#include "chat_server.h"
#include <fstream>
#include <ai_chat_sdk/util/my_logger.h>
#include <thread>


// gflags命令行参数定义
DEFINE_string(host, "0.0.0.0", "Server host address");
DEFINE_int32(port, 8080, "HTTP server port");
DEFINE_string(log_level, "DEBUG", "Log level (DEBUG, INFO, WARN, ERROR)");
DEFINE_double(temperature, 0.7, "AI model temperature");
DEFINE_int32(max_tokens, 2048, "Maximum tokens for AI response");
DEFINE_string(ollama_endpoint, "http://127.0.0.1:11434", "Ollama Endpoint");
DEFINE_string(ollama_model_name, "deepseek-r1:1.5b", "Ollama Model Name");
DEFINE_string(ollama_model_desc, "本地部署的DeepSeek, DeepSeek 推出的旗舰级开源大模型(128K上下文), 性能强大, 专注于深度理解与推理", "Ollama Model Desc");
DEFINE_string(config_file, "chatServer.conf", "配置文件路径");
DEFINE_bool(enable_ssl, false, "是否启用SSL");


void printUsage() {
std::cout << "AI聊天服务器 - 使用说明\n";
std::cout << "============================\n\n";
std::cout << "基本用法:\n";
std::cout << " ./ai_chat_server [选项]\n\n";
std::cout << "主要选项:\n";
std::cout << " --port=8080 HTTP服务器端口 (默认: 8080)\n";
std::cout << " --host=0.0.0.0 服务器监听地址 (默认:0.0.0.0)\n";
std::cout << " --deepseek_api_key=YOUR_KEY DeepSeek API密钥\n";
std::cout << " --gpt_api_key=YOUR_KEY Chat-GPT API密钥\n";
std::cout << " --gemini_api_key=YOUR_KEY Gemini API密钥\n";
std::cout << " --ollama_endpoint=YOUR_ENDPOINT Ollama endpoint端点\n";
std::cout << " --ollama_model_name=YOUR_MODEL_NAME Ollama 模型\n";
std::cout << " --ollama_model_desc=YOUR_MODEL_DESC Ollama 模型\n";
std::cout << " --log_level=INFO 日志级别(DEBUG/INFO/WARN/ERROR)\n";
std::cout << " --temperature=0.7 AI模型温度参数 (0.0-1.0)\n";
std::cout << " --max_tokens=2048 最大回复token数量\n";
std::cout << "示例:\n";
std::cout << " # 基础启动 (demo模式)\n";
std::cout << " ./ai_chat_server\n\n";
std::cout << " # 使用真实API密钥\n";
std::cout << " ./ai_chat_server --deepseek_api_key=sk-xxx --port=8080\n\n";
std::cout << " # 调试模式\n";
std::cout << " ./ai_chat_server --log_level=DEBUG\n\n";
std::cout << "API端点:\n";
std::cout << " GET / - 服务器状态\n";
std::cout << " GET /api/models - 获取可用模型\n";
std::cout << " POST /api/session - 创建会话\n";
std::cout << " POST /api/message - 发送消息(全量返回)\n";
std::cout << " POST /api/message/async - 发送消息(流式返回)\n";
std::cout << " GET /api/sessions - 获取会话列表\n";
std::cout << " GET /api/session/{id}/history - 获取会话历史\n";
std::cout << " DELETE /api/session/{id} - 删除会话\n\n";
}

// 全局服务器实例
std::unique_ptr<ai_chat_server::ChatServer> g_server;
std::atomic<bool> g_running{true};

// 信号处理函数
void signalHandler(int signal) {
    if(g_server)
    { 
        g_server->stop();
    }
    g_running.store(false);
    exit(0);
}


int main(int argc, char **argv)
{
    // 解析命令行参数
    // 设置程序的用途说明，当用户使用--help参数时，会显示这个信息
    gflags::SetUsageMessage("AI聊天服务器");

    // 设置程序版本号，当用户使用--version参数时，会显示这个版本号
    gflags::SetVersionString("1.0.0");



    // 解析传入的命令行参数，true表示从argv中移除已解析的标志，剩余参数可以通过argc和argv访问
    gflags::ParseCommandLineFlags(&argc, &argv, true);

    //检查配置文件是否存在
    std::ifstream configFilePath(FLAGS_config_file);
    if(configFilePath)
    {
        // 加载配置文件，当设置flagfile选项时，gflags会自动从指定文件中加载配置参数
        gflags::SetCommandLineOption("flagfile", FLAGS_config_file.c_str());
    }

    // 显示使用说明
    if(argc>1 && (std::string(argv[1])=="--help" || std::string(argv[1])=="-h"))
    {
        printUsage();
        return 0;
    }

    // 注册信号处理函数
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    // 设置日志级别
    if("DEBUG" == FLAGS_log_level)
    {
        my_logger::Logger::init_logger("aiChatServer" , "stdout" , spdlog::level::debug);
    }
    else if("INFO" == FLAGS_log_level)
    {
        my_logger::Logger::init_logger("aiChatServer" , "stdout" , spdlog::level::info);
    }
    else if("WARN" == FLAGS_log_level)
    {
        my_logger::Logger::init_logger("aiChatServer" , "stdout" , spdlog::level::warn);
    }
    else if("ERROR" == FLAGS_log_level)
    {
        my_logger::Logger::init_logger("aiChatServer" , "stdout" , spdlog::level::err);
    }
    DBG("log_level: {}", FLAGS_log_level);

    // 创建服务器配置
    ai_chat_server::ServerConfig config;
    config.host = FLAGS_host;
    config.port = FLAGS_port;
    config.logLevel = FLAGS_log_level;
    config.enableSSL = FLAGS_enable_ssl;
    config.temperature = FLAGS_temperature;
    config.maxTokens = FLAGS_max_tokens;
    const char* deepseekKey = std::getenv("deepseek_api_key");
    const char* chatgptKey = std::getenv("chatgpt_api_key");
    const char* geminiKey = std::getenv("gemini_api_key");
    config.deepseekApiKey = deepseekKey ? deepseekKey : "";
    config.chatgptApiKey = chatgptKey ? chatgptKey : "";
    config.geminiApiKey = geminiKey ? geminiKey : "";
    config.ollamaModelName = FLAGS_ollama_model_name;
    config.ollamaDesc = FLAGS_ollama_model_desc;
    config.ollamaEndpoint = FLAGS_ollama_endpoint;

    // 验证参数
    if(config.temperature< 0.0 || config.temperature> 1.0)
    {
        ERR("temperature is invalid, must be between 0.0 and 1.0");
        return -1;
    }
    if(config.maxTokens<= 0)
    {
        ERR("max_tokens is invalid, must be greater than 0");
        return -1;
    }
    INFO("模型参数: temperature={}, max_tokens={}", config.temperature, config.maxTokens);

    if(config.deepseekApiKey.empty() || config.chatgptApiKey.empty() || config.geminiApiKey.empty())
    {
        ERR("deepseek_api_key, chatgpt_api_key, gemini_api_key is empty or invalid");
        return -1;
    }   
    if(config.ollamaModelName.empty() || config.ollamaDesc.empty()||config.ollamaEndpoint.empty())
    {
        ERR("ollama_model_name, ollama_model_desc, ollama_endpoint is empty or invalid");
        return -1;
    }

    // 创建并初始化服务器
    INFO("============================");
    INFO("服务器启动中...");
    g_server = std::make_unique<ai_chat_server::ChatServer>(config);
    if(!g_server->start())
    {
        ERR("服务器启动失败");
        return -1;
    }
    INFO("服务器启动成功");
    INFO("按Ctrl+C停止服务器");

    while(g_server->isRunning())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        INFO("服务器运行中...");
    }

    INFO("服务器停止");

    return 0;

}