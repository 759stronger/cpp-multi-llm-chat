#pragma once
#include "LLMManager.h"
#include "dataManager.h"
#include "session_manager.h"
#include "common.h"
#include <memory>
#include <vector>
#include <string>
#include <ctime>
#include <functional>
#include <unordered_map>

namespace chatsdk
{
    class chat_sdk
    {
    public:
    //初始化模型
    bool initModels(const std::vector<std::shared_ptr<Config>>& configs);

    //创建session
    std::string createSession(const std::string& model_name);

    //获取会话
    std::shared_ptr<Session> getSession(const std::string& session_id);

    //获取所有会话列表
    std::vector<std::string> getSessionList();

    //删除会话
    bool deleteSession(const std::string& session_id);

    //获取可用模型列表
    std::vector<std::string> getAvailableModels();

    //发送消息 全量返回
    std::string sendMessage(const std::string& session_id, const std::string& message);

    //发送消息 流式返回
    void sendStreamMessage(const std::string& session_id, const std::string& message,
    std::function<void(const std::string& ,bool)> callback);
private:
        
    //注册所有模型提供者
    void registerAllProviders(const std::vector<std::shared_ptr<Config>>& configs);

    //初始化所有模型提供者
    void initAllProviders(const std::vector<std::shared_ptr<Config>>& configs);

    //初始化模型提供者 通过api调用
    bool initModelProviderByApi(const std::string& model_name ,const std::shared_ptr<ApiConfig>& apiconfig);

    //初始化通过ollama
    bool initModelProviderByOllama(const std::string& model_name ,const std::shared_ptr<OllamaConfig>& ollamaconfig);

    private:
        bool _initialized;

        std::unordered_map<std::string, std::shared_ptr<Config>> _configs;
        LLMManager _llm_manager;
        SessionManager _session_manager;
    };
}
