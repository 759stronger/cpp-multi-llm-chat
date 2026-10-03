#pragma once
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "ILLMProvider.h"

namespace chatsdk {

    class LLMManager {
    public:
        LLMManager() = default;
        ~LLMManager() = default;

        //注册LLM提供者
        bool registerProvider(const std::string& name, std::unique_ptr<LLMProvider> provider);
        //初始化指定模型
        bool initModel(const std::string& model_name, 
            const std::map<std::string , std::string>& model_config);
        //获取可用模型列表
        std::vector<std::string> getAvailableModelList() const;
        //检查模型是否可用
        bool isModelAvailable(const std::string& model_name) const;
        //发送信息到指定模型
        std::string sendMessage(const std::string& model_name, 
            const std::vector<Message>& messages, 
            const std::map<std::string, std::string>& request_params);
        //发送信息到指定模型流式输出
        std::string sendStreamMessage(const std::string& model_name, 
            const std::vector<Message>& messages, 
            const std::map<std::string, std::string>& request_param, 
            std::function<void(const std::string& , bool)> callback);

    private:
        //模型名称到提供者的映射
        std::unordered_map<std::string, std::unique_ptr<LLMProvider>> _providers;
        //模型名称到模型描述的映射
        std::unordered_map<std::string, ModelInfo> _models;
    };
}
