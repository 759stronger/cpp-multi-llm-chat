#ifndef ILLMPROVIDER_H
#define ILLMPROVIDER_H
#include <string>
#include <map>
#include <functional>
#include <vector>
#include "common.h"



namespace chatsdk {

    class LLMProvider {
    public:
        // 初始化模型
        virtual bool initModel(const std::map<std::string , std::string>& model_config) = 0 ;
        //检测模型是否有用
        virtual bool isAvailable() = 0 ;
        //获取模型名称
        virtual std::string getModelName() const = 0 ;
        //获取模型描述信息
        virtual std::string getModelDesc() const = 0 ;
        //发送信息给模型
        virtual std::string sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& request_params) = 0 ;
        //发送信息给模型流式输出
        virtual std::string sendStreamMessage(const std::vector<Message>& messages, 
            const std::map<std::string, std::string>& request_param  , 
            std::function<void(const std::string& , bool)> callback) = 0 ;
    
    protected:
            
      bool _isAvailable = false;  //模型是否可用
      std::string _api_key ;  //模型API密钥 
      std::string _endpoint ;  //模型API地址 base url
    
    };
}

#endif 