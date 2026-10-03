#include <ai_chat_sdk/LLMManager.h>
#include <ai_chat_sdk/util/my_logger.h>

namespace chatsdk {

    bool LLMManager::registerProvider(const std::string& name, std::unique_ptr<LLMProvider> provider) {
        if(!provider) {
            ERR("LLMManager::registerProvider: provider is null");
            return false;
        }
        _providers[name] = std::move(provider);
        _models.emplace(name, ModelInfo(name));
        INFO("LLMManager::registerProvider: provider %s registered", name.c_str());
        return true;
    }

    bool LLMManager::initModel(const std::string& model_name, 
        const std::map<std::string , std::string>& model_config) {
        auto it = _providers.find(model_name);
        if (it == _providers.end()) {
            ERR("LLMManager::initModel: model %s not registered", model_name.c_str());
            return false;
        }
        bool success = it->second->initModel(model_config);
        if(success)
        {
            INFO("LLMManager::initModel: model %s initialized", model_name.c_str());
            _models.at(model_name)._description = it->second->getModelDesc();
            _models.at(model_name)._isavailable = true;
        }
        else{
            ERR("LLMManager::initModel: model %s initialization failed", model_name.c_str());
            _models.at(model_name)._isavailable = false;
        }
        return success;
    }

    std::vector<std::string> LLMManager::getAvailableModelList() const {
        std::vector<std::string> available_models;
        for (const auto& model : _models) {
            if(model.second._isavailable) {
                available_models.push_back(model.first);
            }
        }
        return available_models;
    }
        
    bool LLMManager::isModelAvailable(const std::string& model_name) const {
        auto it = _models.find(model_name);
        return it != _models.end() && it->second._isavailable;
    }

    std::string LLMManager::sendMessage(const std::string& model_name, 
        const std::vector<Message>& messages, 
        const std::map<std::string, std::string>& request_params) {
       auto it = _providers.find(model_name);
       if(it == _providers.end()) {
            ERR("LLMManager::sendMessage: model %s not registered", model_name.c_str());
            return "";
       }

        if(!it->second->isAvailable()) {
            ERR("LLMManager::sendMessage: model %s not available", model_name.c_str());
            return "";
        }

        return it->second->sendMessage(messages, request_params);
    }

    std::string LLMManager::sendStreamMessage(const std::string& model_name, 
        const std::vector<Message>& messages, 
        const std::map<std::string, std::string>& request_param, 
        std::function<void(const std::string& , bool)> callback) {
        auto it = _providers.find(model_name);
        if(it == _providers.end()) {
            ERR("LLMManager::sendStreamMessage: model %s not registered", model_name.c_str());
            return "";
        }
        if(!it->second->isAvailable()) {
            ERR("LLMManager::sendStreamMessage: model %s not available", model_name.c_str());
            return "";
        }

        return it->second->sendStreamMessage(messages, request_param, callback);
    }

}
