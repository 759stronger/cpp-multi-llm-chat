#include "ILLMProvider.h"
#include "common.h"

namespace chatsdk {
    class DeepSeekProvider : public LLMProvider {
    public:
        DeepSeekProvider() = default;
        ~DeepSeekProvider() = default;

        // 初始化模型
        virtual bool initModel(const std::map<std::string , std::string>& model_config) override;
        //检测模型是否有用
        virtual bool isAvailable() override;
        //获取模型名称
        virtual std::string getModelName() const override;
        //获取模型描述信息
        virtual std::string getModelDesc() const override;

        // 发送消息
        std::string sendMessage(const std::vector<Message>& messages, const std::map<std::string , std::string>& model_config) override;
        
        std::string sendStreamMessage(const std::vector<Message>& messages, 
            const std::map<std::string , std::string>& request_param, 
            std::function<void(const std::string& , bool)> callback) override;
    };
}