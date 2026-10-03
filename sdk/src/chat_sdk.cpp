#include <ai_chat_sdk/chat_sdk.h>
#include <ai_chat_sdk/DeepSeekProvider.h>
#include <ai_chat_sdk/OllamaDeepSeekProvider.h>
#include <ai_chat_sdk/ChatGPTProvider.h>
#include <ai_chat_sdk/GeminiProvider.h>
#include <ai_chat_sdk/util/my_logger.h>
#include <ai_chat_sdk/common.h>
#include <ai_chat_sdk/session_manager.h>
#include <ai_chat_sdk/dataManager.h>

#include <memory>
#include <string>
#include <unordered_set>

namespace chatsdk {

    //初始化模型
    bool chat_sdk::initModels(const std::vector<std::shared_ptr<Config>>& configs)
    {   
        //注册所有模型提供者
        registerAllProviders(configs);
        //初始化所有模型提供者
        initAllProviders(configs);
        _initialized = true;
        return true;
    }

    //创建session
    std::string chat_sdk::createSession(const std::string& model_name)
    {
        if(!_initialized) {
            ERR("chat sdk not initialized");
            return "";
        }
        std::string sessionId = _session_manager.create_session(model_name);
        INFO("create session: %s", sessionId.c_str());
        return sessionId;

    }

    //获取会话
    std::shared_ptr<Session> chat_sdk::getSession(const std::string& session_id)
    {
        return _session_manager.get_session(session_id);
    }

    //获取所有会话列表
    std::vector<std::string> chat_sdk::getSessionList()
    {
        return _session_manager.get_session_list();
    }

    //删除会话
    bool chat_sdk::deleteSession(const std::string& session_id)
    {
        bool ret = _session_manager.delete_session(session_id);
        if(ret) {
            INFO("delete session: %s", session_id.c_str());
        }
        return ret;
    }

    //获取可用模型列表
    std::vector<std::string> chat_sdk::getAvailableModels()
    {
        return _llm_manager.getAvailableModelList();
    }

    //发送消息 全量返回
    std::string chat_sdk::sendMessage(const std::string& session_id, const std::string& message)
    {
        if(!_initialized) {
            ERR("chat sdk not initialized");
            return "";
        }
        //获取当前会话的session对象
        auto session = _session_manager.get_session(session_id);
        if(!session) {
            ERR("session not found: %s", session_id.c_str());
            return "";
        }
        Message usermsg("user", message);
        _session_manager.add_message(session_id, usermsg);
        //构建请求参数
        auto it = _configs.find(session->model_name);
        if(it == _configs.end()) {
            ERR("model not found: %s", session->model_name.c_str());
            return "";
        }
        
        std::map<std::string, std::string> requestParams;
        requestParams["temperature"] = std::to_string(it->second->temperature);
        requestParams["max_tokens"] = std::to_string(it->second->max_tokens);
        //给模型发消息
        //获取完整历史会话
        std::vector<Message> history = _session_manager.get_session_history(session_id);
        std::string response = _llm_manager.sendMessage(session->model_name, history, requestParams);
        //添加助手响应并更新会话时间
        Message assistantmsg("assistant", response);
        _session_manager.add_message(session_id, assistantmsg);
        _session_manager.update_session_timestamp(session_id);
        INFO("send message: %s", message.c_str());
        return response;
    }

    //发送消息 流式返回
    void chat_sdk::sendStreamMessage(const std::string& session_id, const std::string& message,
    std::function<void(const std::string& ,bool)> callback)
    {
        if(!_initialized) {
            ERR("chat sdk not initialized");
            return;
        }
        auto session = _session_manager.get_session(session_id);
        if(!session) {
            ERR("session not found: %s", session_id.c_str());
            return;
        }
        Message usermsg("user", message);
        _session_manager.add_message(session_id, usermsg);
        //构建请求参数
        auto it = _configs.find(session->model_name);
        if(it == _configs.end()) {
            ERR("model not found: %s", session->model_name.c_str());
            return;
        }

        std::map<std::string, std::string> requestParams;
        requestParams["temperature"] = std::to_string(it->second->temperature);
        requestParams["max_tokens"] = std::to_string(it->second->max_tokens);
        //给模型发消息
        //获取完整历史会话
        std::vector<Message> history = _session_manager.get_session_history(session_id);
        std::string response = _llm_manager.sendStreamMessage(session->model_name, history, requestParams, callback);
        //添加助手响应并更新会话时间
        Message assistantmsg("assistant", response);
        _session_manager.add_message(session_id, assistantmsg);
        _session_manager.update_session_timestamp(session_id);
        INFO("send stream message: %s", message.c_str());

    }

    //注册所有模型提供者
    void chat_sdk::registerAllProviders(const std::vector<std::shared_ptr<Config>>& configs)
    {
        if(!_llm_manager.isModelAvailable("deepseek-v4-pro")) {   
            auto deepseekProvider = std::make_unique<DeepSeekProvider>();
            _llm_manager.registerProvider("deepseek-v4-pro", std::move(deepseekProvider));
            INFO("register model provider: deepseek-v4-pro");
        }
        if(!_llm_manager.isModelAvailable("gpt-5.4-mini")) {   
            auto gptProvider = std::make_unique<ChatGPTProvider>();
            _llm_manager.registerProvider("gpt-5.4-mini", std::move(gptProvider));
            INFO("register model provider: gpt-5.4-mini");
        }
        if(!_llm_manager.isModelAvailable("gemini-3.5-flash")) {   
            auto geminiProvider = std::make_unique<GeminiProvider>();
            _llm_manager.registerProvider("gemini-3.5-flash", std::move(geminiProvider));
            INFO("register model provider: gemini-3.5-flash");
        }
        std::unordered_set<std::string> model_names;
        for(const auto& config : configs) {
            auto ollamaConfig = std::dynamic_pointer_cast<OllamaConfig>(config);
            if(ollamaConfig) {
                const std::string& model_name = ollamaConfig->model_name;
                if(model_names.find(model_name) == model_names.end()) {
                    model_names.insert(model_name);
                    if(!_llm_manager.isModelAvailable(model_name)) {
                        auto ollamaProvider = std::make_unique<OllamaDeepSeekProvider>();
                        _llm_manager.registerProvider(model_name, std::move(ollamaProvider));
                        INFO("register model provider: %s", model_name.c_str());
                    }
                }
               
            }
        }
    }

    //初始化所有模型提供者
    void chat_sdk::initAllProviders(const std::vector<std::shared_ptr<Config>>& configs)
    {
        for(const auto& config : configs) {
            if(auto apiconfig  = std::dynamic_pointer_cast<ApiConfig>(config)) {
               INFO("初始化模型配置信息: modelname - {} apikey - {} temperature -{}", apiconfig->model_name,apiconfig->api_key.substr(0, 4),apiconfig->temperature);
               if(apiconfig->model_name == "deepseek-v4-pro"||apiconfig->model_name == "gpt-5.4-mini"||apiconfig->model_name == "gemini-3.5-flash") {
                    initModelProviderByApi(apiconfig->model_name, apiconfig);
               }
               else{
                  ERR("不支持的API模型配置类型: {}", apiconfig->model_name);
               }
            }
            else if(auto ollamaConfig   = std::dynamic_pointer_cast<OllamaConfig>(config)) {
               INFO("初始化Ollama模型配置: modelname - {} endpoint - {}",ollamaConfig->model_name, ollamaConfig->endpoint);
               initModelProviderByOllama(ollamaConfig->model_name, ollamaConfig);
            }
            else
            {
                ERR("不支持的模型配置类型: {}", config->model_name);
            }
        }
    
    }

    //初始化模型提供者 通过api调用
    bool chat_sdk::initModelProviderByApi(const std::string& model_name ,const std::shared_ptr<ApiConfig>& apiconfig)
    {
        //参数检测
        if(model_name.empty()) {
            ERR("model name is required for chatsdk");
            return false;
        }
        if(!apiconfig || apiconfig->api_key.empty()) {
            ERR("API key is required for chatsdk");
            return false;
        }
        // 检测模型是否已经注册
        if(_llm_manager.isModelAvailable(model_name)) {
            ERR("model {} is already registered", model_name.c_str());
        }
        //初始化模型
        std::map<std::string , std::string> modelParams;
        modelParams["api_key"] = apiconfig->api_key;
        bool initSuccess = _llm_manager.initModel(model_name, modelParams);
        if(!initSuccess) {
            ERR("init model {} failed", model_name.c_str());
            return false;
        }
        //管理模型配置
        _configs[model_name] = apiconfig;
        INFO("chat sdk init {} model provider success with api key", model_name);
        INFO("Config info: temperature - {}, max_tokens - {}", apiconfig->temperature, apiconfig->max_tokens);
        return true;

    }

    //初始化通过ollama
    bool chat_sdk::initModelProviderByOllama(const std::string& model_name ,const std::shared_ptr<OllamaConfig>& ollamaconfig)
    {
        //参数检测
        if(model_name.empty()) {
            ERR("ollama model name is required for chatsdk");
            return false;
        }
        if(!ollamaconfig || ollamaconfig->endpoint.empty()) {
            ERR("ollama endpoint is required for chatsdk");
            return false;
        }
        // 检测模型是否已经初始化
        if(_llm_manager.isModelAvailable(model_name)) {
            ERR("ollama model {} is already initialized", model_name.c_str());
            return false;
        }
        //初始化模型
        std::map<std::string , std::string> modelParams;
        modelParams["model_name"] = ollamaconfig->model_name;
        modelParams["endpoint"] = ollamaconfig->endpoint;
        modelParams["model_desc"] = ollamaconfig->model_desc;
        bool initSuccess = _llm_manager.initModel(model_name, modelParams);
        if(!initSuccess) {
            ERR("init ollama model {} failed", model_name.c_str());
            return false;
        }
        //管理模型配置
        _configs[model_name] = ollamaconfig;
        INFO("chat sdk init {} model provider success with endpoint", model_name);
        return true;
    }



















































}
