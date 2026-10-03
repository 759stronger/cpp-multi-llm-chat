#pragma once
#include <ai_chat_sdk/chat_sdk.h>
#include <string>
#include <httplib.h>

namespace ai_chat_server
{
    //服务器配置信息
    struct ServerConfig
    {
        std::string host = "0.0.0.0"; //服务器绑定ip
        int port = 8000; //服务器绑定端口
        std::string logLevel = "INFO"; //日志级别
        bool enableSSL = false ;  //是否开启ssl

        //模型参数
        double temperature = 0.7; //温度参数
        int maxTokens = 1024; //最大token数
       
        //api_key
        std::string deepseekApiKey;
        std::string chatgptApiKey;
        std::string geminiApiKey;

        //ollama配置
        std::string ollamaModelName; //ollama模型名称
        std::string ollamaDesc;  //ollama模型描述
        std::string ollamaEndpoint; //ollama服务器地址
        
    };

    // AI 服务器主类
    class ChatServer
    {
    public:
        ChatServer(const ServerConfig& config);
        std::string buildErrorResponse(bool isSuccess, const std::string& message , const std::string& data="");
        //服务器实现
        bool start(); //启动服务器
        void stop(); //停止服务器
        bool isRunning()const; //判断服务器是否运行中
    private:
        //处理创建会话请求
        void handleCreateSession(const httplib::Request& req, httplib::Response& resp);
        //获取会话列表
        void handleGetSessions(const httplib::Request& req, httplib::Response& resp);
        //获取支持的模型
        void handleGetModels(const httplib::Request& req, httplib::Response& resp);
        //获取历史会话信息
        void handleGetSessionHistory(const httplib::Request& req, httplib::Response& resp);
        //删除会话
        void handleDeleteSession(const httplib::Request& req, httplib::Response& resp);
        //发送消息 全量返回
        void handleSendMessage(const httplib::Request& req, httplib::Response& resp);
        //发送消息 流式返回
        void handleSendStreamMessage(const httplib::Request& req, httplib::Response& resp);
        //设置路由规则
        void setHTTPRoutes();

     private:
        ServerConfig _config;// 保存服务器配置信息
        std::atomic<bool> _running;// 服务器是否运行中
       std::shared_ptr<chatsdk::chat_sdk> _chatSDK;// 聊天SDK
       std::unique_ptr<httplib::Server> _httpServer;// HTTP服务器
      
    };


}// namespace ai_chat_server
