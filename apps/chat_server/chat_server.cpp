#include "chat_server.h"
#include <ai_chat_sdk/util/my_logger.h>
#include <ai_chat_sdk/chat_sdk.h>
#include <cstdint>
#include <httplib.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/writer.h>
#include <jsoncpp/json/reader.h>
#include <thread>

namespace ai_chat_server
{
    ChatServer::ChatServer(const ServerConfig& config):
        _config(config),
        _running(false)
    {
        _chatSDK = std::make_shared<chatsdk::chat_sdk>();

        chatsdk::ApiConfig DeepseekApiConfig;
        DeepseekApiConfig.api_key = _config.deepseekApiKey;
        DeepseekApiConfig.model_name = "deepseek-v4-pro";
        DeepseekApiConfig.max_tokens = _config.maxTokens;
        DeepseekApiConfig.temperature = _config.temperature;

        chatsdk::ApiConfig ChatgptApiConfig;
        ChatgptApiConfig.api_key = _config.chatgptApiKey;
        ChatgptApiConfig.model_name = "gpt-5.4-mini";
        ChatgptApiConfig.max_tokens = _config.maxTokens;
        ChatgptApiConfig.temperature = _config.temperature;

        chatsdk::ApiConfig GeminiApiConfig;
        GeminiApiConfig.api_key = _config.geminiApiKey;
        GeminiApiConfig.model_name = "gemini-3.5-flash";
        GeminiApiConfig.max_tokens = _config.maxTokens;
        GeminiApiConfig.temperature = _config.temperature;

        chatsdk::OllamaConfig OllamaConfig;
        OllamaConfig.model_name = _config.ollamaModelName;
        OllamaConfig.max_tokens = _config.maxTokens;
        OllamaConfig.temperature = _config.temperature;
        OllamaConfig.endpoint = _config.ollamaEndpoint;
        OllamaConfig.model_desc = _config.ollamaDesc;

        std::vector<std::shared_ptr<chatsdk::Config>> configs;
        configs.push_back(std::make_shared<chatsdk::ApiConfig>(DeepseekApiConfig));
        configs.push_back(std::make_shared<chatsdk::ApiConfig>(ChatgptApiConfig));
        configs.push_back(std::make_shared<chatsdk::ApiConfig>(GeminiApiConfig));
        configs.push_back(std::make_shared<chatsdk::OllamaConfig>(OllamaConfig));

        DBG("初始化chatsdk");
        if(!_chatSDK->initModels(configs))
        {
            ERR("初始化chatsdk失败");
            return;
        }

        _httpServer = std::make_unique<httplib::Server>();
        INFO("http服务器初始化成功");
    }

    std::string ChatServer::buildErrorResponse(bool isSuccess,
        const std::string& message , const std::string& data)
    {
        Json::Value response;
        response["success"] = isSuccess;
        response["message"] = message;
        if(!data.empty())
        {
            Json::Value dataJson;
            Json::Reader reader;
            if(reader.parse(data, dataJson))
            {
                response["data"] = dataJson;
            }
            else
            {
                response["data"] = data;
            }
        }
        Json::StreamWriterBuilder builder;
        return Json::writeString(builder, response);
    }

    void ChatServer::handleCreateSession(const httplib::Request& req, httplib::Response& resp)
    {
        Json::Value requestJson;
        Json::Reader reader;
        if(!reader.parse(req.body, requestJson))
        {
            std::string errorMessage = buildErrorResponse(false, "请求体解析失败");
            resp.status = 400;
            resp.set_content(errorMessage, "application/json");
            return;
        }

        std::string modelName = requestJson.get("model" , "deepseek-v4-pro").asString();
        std::string sessionID = _chatSDK->createSession(modelName);
        if(sessionID.empty())
        {
            std::string errorMessage = buildErrorResponse(false, "创建会话失败");
            resp.status = 500;
            resp.set_content(errorMessage, "application/json");
            return;
        }

        Json::Value data;
        data["session_id"] = sessionID;
        data["model"] = modelName;

        Json::StreamWriterBuilder builder;
        std::string dataStr = Json::writeString(builder, data);

        std::string response = buildErrorResponse(true, "创建会话成功", dataStr);
        resp.set_content(response, "application/json");
    }

    void ChatServer::handleGetSessions(const httplib::Request& req, httplib::Response& resp)
    {
        auto sessionIDs = _chatSDK->getSessionList();
        Json::Value sessionJsonArray(Json::arrayValue);
        for(const auto& id : sessionIDs)
        {
            auto session = _chatSDK->getSession(id);
            if(session)
            {
                Json::Value sessionJson;
                sessionJson["id"] = session->session_id;
                sessionJson["model"] = session->model_name;
                sessionJson["created_at"] = static_cast<int64_t>(session->create_time);
                sessionJson["updated_at"] = static_cast<int64_t>(session->update_time);
                sessionJson["message_count"] = static_cast<int>(session->messages.size());

                if(!session->messages.empty())
                {
                    sessionJson["first_user_message_content"] = session->messages.front().content;
                }

                sessionJsonArray.append(sessionJson);
            }
        }

        Json::Value response;
        response["success"] = true;
        response["message"] = "获取会话列表成功";
        response["data"] = sessionJsonArray;

        Json::StreamWriterBuilder builder;
        std::string responseStr = Json::writeString(builder, response);
        resp.set_content(responseStr, "application/json");
    }

    void ChatServer::handleGetModels(const httplib::Request& req, httplib::Response& resp)
    {
        auto models = _chatSDK->getAvailableModels();
        Json::Value modelJsonArray(Json::arrayValue);
        for(const auto& model : models)
        {
            Json::Value modelJson;
            modelJson["name"] = model;
            modelJsonArray.append(modelJson);
        }
        Json::Value response;
        response["success"] = true;
        response["message"] = "获取模型列表成功";
        response["data"] = modelJsonArray;
        Json::StreamWriterBuilder builder;
        std::string responseStr = Json::writeString(builder, response);
        resp.set_content(responseStr, "application/json");
    }

    void ChatServer::handleGetSessionHistory(const httplib::Request& req, httplib::Response& resp)
    {
        std::string sessionID = req.matches[1];
        auto session = _chatSDK->getSession(sessionID);
        if(!session)
        {
            std::string errorMessage = buildErrorResponse(false, "会话不存在");
            resp.status = 404;
            resp.set_content(errorMessage, "application/json");
            return;
        }
        Json::Value messageJsonArray(Json::arrayValue);
        for(const auto& msg : session->messages)
        {
            Json::Value messageJson;
            messageJson["id"] = msg.id;
            messageJson["role"] = msg.role;
            messageJson["content"] = msg.content;
            messageJson["timestamp"] = static_cast<int64_t>(msg.timestamp);
            messageJsonArray.append(messageJson);
        }
        Json::Value response;
        response["success"] = true;
        response["message"] = "获取会话历史成功";
        response["data"] = messageJsonArray;
        Json::StreamWriterBuilder builder;
        std::string responseStr = Json::writeString(builder, response);
        resp.set_content(responseStr, "application/json");
    }

    void ChatServer::handleDeleteSession(const httplib::Request& req, httplib::Response& resp)
    {
        std::string sessionID = req.matches[1];
        bool success = _chatSDK->deleteSession(sessionID);
        if(success)
        {
            std::string response = buildErrorResponse(true, "删除会话成功");
            resp.set_content(response, "application/json");
        }
        else
        {
            std::string errorMessage = buildErrorResponse(false, "删除会话失败");
            resp.status = 404;
            resp.set_content(errorMessage, "application/json");
        }
    }

    void ChatServer::handleSendMessage(const httplib::Request& req, httplib::Response& resp)
    {
        Json::Value requestJson;
        Json::Reader reader;
        if(!reader.parse(req.body, requestJson))
        {
            std::string errorMessage = buildErrorResponse(false, "请求体解析错误");
            resp.status = 400;
            resp.set_content(errorMessage, "application/json");
            return;
        }

        std::string sessionID = requestJson.get("session_id", "").asString();
        std::string message = requestJson.get("message", "").asString();

        if(sessionID.empty() || message.empty())
        {
            std::string errorMessage = buildErrorResponse(false, "会话ID或消息为空");
            resp.status = 400;
            resp.set_content(errorMessage, "application/json");
            return;
        }

        std::string assistantMessage = _chatSDK->sendMessage(sessionID, message);
        if(assistantMessage.empty())
        {
            std::string errorMessage = buildErrorResponse(false, "发送消息失败");
            resp.status = 500;
            resp.set_content(errorMessage, "application/json");
            return;
        }
        Json::Value data;
        data["response"] = assistantMessage;
        data["session_id"] = sessionID;

        Json::StreamWriterBuilder builder;
        std::string dataStr = Json::writeString(builder, data);
        std::string responseStr = buildErrorResponse(true, "发送消息成功", dataStr);
        resp.set_content(responseStr, "application/json");
    }

    void ChatServer::handleSendStreamMessage(const httplib::Request& req, httplib::Response& resp)
    {
        Json::Value requestJson;
        Json::Reader reader;
        if(!reader.parse(req.body, requestJson))
        {
            std::string errorMessage = buildErrorResponse(false, "请求体解析错误");
            resp.status = 400;
            resp.set_content(errorMessage, "application/json");
            return;
        }

        std::string sessionID = requestJson.get("session_id", "").asString();
        std::string message = requestJson.get("message", "").asString();
        if(sessionID.empty() || message.empty())
        {
            std::string errorMessage = buildErrorResponse(false, "会话ID或消息为空");
            resp.status = 400;
            resp.set_content(errorMessage, "application/json");
            return;
        }

        resp.status = 200;
        resp.set_header("Cache-Control", "no-cache");
        resp.set_header("Connection", "keep-alive");
        resp.set_header("Content-Type", "text/event-stream");
        resp.set_chunked_content_provider("text/event-stream", [this, sessionID, message](size_t offset, httplib::DataSink& sink) -> bool
        {
            try {
                auto writeChunk = [&](const std::string& chunk, bool last)
                {
                    std::string event = "data: " + chunk + "\n\n";
                    sink.write(event.data(), event.size());
                    if(last)
                    {
                        std::string doneStream = "data: [DONE]\n\n";
                        sink.write(doneStream.data(), doneStream.size());
                        sink.done();
                    }
                };
                writeChunk("", false);
                _chatSDK->sendStreamMessage(sessionID, message, [&](const std::string& chunk, bool last) {
                    writeChunk(chunk, last);
                });
            } catch (const std::exception& e) {
                ERR("Stream exception: {}", e.what());
                sink.done();
            } catch (...) {
                ERR("Stream unknown exception");
                sink.done();
            }
            return true;
        });
    }

    void ChatServer::setHTTPRoutes()
    {
        _httpServer->Get("/api/models", [this](const httplib::Request& req, httplib::Response& resp) {
            handleGetModels(req, resp);
        });
        _httpServer->Post("/api/sessions", [this](const httplib::Request& req, httplib::Response& resp) {
            handleCreateSession(req, resp);
        });
        _httpServer->Post("/api/message", [this](const httplib::Request& req, httplib::Response& resp) {
            handleSendMessage(req, resp);
        });
        _httpServer->Post("/api/message/async", [this](const httplib::Request& req, httplib::Response& resp) {
            handleSendStreamMessage(req, resp);
        });
        _httpServer->Get("/api/sessions", [this](const httplib::Request& req, httplib::Response& resp) {
            handleGetSessions(req, resp);
        });
        _httpServer->Get("/api/sessions/(.*)/history", [this](const httplib::Request& req, httplib::Response& resp) {
            handleGetSessionHistory(req, resp);
        });
        _httpServer->Delete("/api/sessions/(.*)", [this](const httplib::Request& req, httplib::Response& resp) {
            handleDeleteSession(req, resp);
        });
    }

    bool ChatServer::start()
    {
        if(_running.load())
        {
            WARN("服务器已运行中");
            return false;
        }
        setHTTPRoutes();
        _httpServer->set_mount_point("/", "./www");
        std::thread httpServerThread([this]() {
            INFO("启动HTTP服务器");
            _httpServer->listen(_config.host, _config.port);
        });
        httpServerThread.detach();
        _running.store(true);
        INFO("HTTP服务器启动成功");
        return true;
    }

    void ChatServer::stop()
    {
        if(!_running.load())
        {
            WARN("服务器未运行中");
            return;
        }
        if(_httpServer)
        {
            _httpServer->stop();
        }
        _running.store(false);
        INFO("HTTP服务器停止成功");
    }

    bool ChatServer::isRunning() const { return _running.load(); }

}
