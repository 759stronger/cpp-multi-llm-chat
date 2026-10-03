#pragma once
#include <string>
#include <vector>
#include <ctime>

namespace chatsdk {
    
    //消息结构
    struct Message {
        std::string id; // 消息ID
        std::string role; // 消息角色，user/assistant/system
        std::string content; // 消息内容
        std::time_t timestamp; // 消息时间戳

        Message( const std::string& r, const std::string& c)
            : role(r), content(c), timestamp(std::time(nullptr))
        {}
    };

    //调用模型时配置信息
    struct Config {
       virtual ~Config() = default;
       std::string model_name; // 模型名称
       double temperature = 0.7; // 温度参数
       int max_tokens = 2048; // 最大token数
       
    };
    
    //api key 配置结构
    struct ApiConfig : public Config {
        std::string api_key;
    };

    //Ollama 配置结构
    struct OllamaConfig : public Config {
        std::string endpoint;
        std::string model_desc;
    };

    //会话结构
    struct Session {
        std::string session_id; // 会话ID
        std::string model_name; // 模型名称
        std::vector<Message> messages; // 会话消息
        std::time_t create_time; // 会话创建时间戳
        std::time_t update_time; // 会话更新时间戳

        Session( const std::string& n)
            : model_name(n), create_time(std::time(nullptr)), update_time(std::time(nullptr))
        {}
    };

    //LLM模型信息
    struct ModelInfo {
       std::string model_name; // 模型名称
       std::string _description; // 模型描述
       std::string _provider; // 模型提供方
       std::string _endpoint; // 模型接口地址
       bool _isavailable = false; // 是否可用

       ModelInfo( const std::string& n, const std::string& d = "", const std::string& p = "", const std::string& e = "")
            : model_name(n), _description(d), _provider(p), _endpoint(e), _isavailable(false)
        {}
    };
}