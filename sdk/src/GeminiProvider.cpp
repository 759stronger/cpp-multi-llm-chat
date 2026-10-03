#include <ai_chat_sdk/GeminiProvider.h>
#include <ai_chat_sdk/util/my_logger.h>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <httplib.h>
#include <sstream>

namespace chatsdk {
    // 初始化模型
    bool GeminiProvider::initModel(const std::map<std::string , std::string>& model_config) {
       auto it = model_config.find("api_key");
       if (it == model_config.end()) {
           ERR("api_key is not found in model_config");
           return false;
       }
       else {
           _api_key = it->second;
       }

       it = model_config.find("base_url");
       if (it != model_config.end()) {
           _endpoint = it->second;
       }
       else {
           _endpoint = "https://generativelanguage.googleapis.com";
       }

        _isAvailable = true;
        INFO("GeminiProvider initModel success");
        INFO("OPENAI_API_ENDPOINT: {}", _endpoint);
        return true;
    }

    // 检测模型是否有用
    bool GeminiProvider::isAvailable() {
        return _isAvailable;
    }

    // 获取模型名称
    std::string GeminiProvider::getModelName() const {
        return "gemini-3.5-flash";
    }

    // 获取模型描述信息
    std::string GeminiProvider::getModelDesc() const {
        return "一款基于Google Gemini的聊天模型";
    }

    //全量发送消息
     std::string GeminiProvider::sendMessage(const std::vector<Message>& messages, const std::map<std::string, std::string>& request_param) {
        if(!_isAvailable) {
            ERR("GeminiProvider is not available");
            return "";
        }
        double temperature = 0.7;
        int max_tokens = 2048;

        if(request_param.find("temperature") != request_param.end()) {
            temperature = std::stof(request_param.find("temperature")->second);
        }
        if(request_param.find("max_tokens") != request_param.end()) {
            max_tokens = std::stoi(request_param.find("max_tokens")->second);
        }

        //构建历史信息
        Json::Value message_array(Json::arrayValue);
        for(const auto& message : messages) {
           Json::Value msg;  
           msg["role"] = message.role;
            msg["content"] = message.content;
            message_array.append(msg);
        }

        //构建请求体
        Json::Value request_body;
        request_body["model"] = getModelName();
        request_body["messages"] = message_array;
        request_body["temperature"] = temperature;
        request_body["max_tokens"] = max_tokens;        

        //序列化
        Json::StreamWriterBuilder writer;
        std::string json_str = Json::writeString(writer, request_body);
        DBG("GeminiProvider: request_body: {}", json_str);

        //创建HTTP client
        httplib::Client client(_endpoint);
        client.set_connection_timeout(30,0);   // 30秒超时
        client.set_read_timeout(60,0);              // 60秒读取超时
        client.set_proxy("127.0.0.1", 7890);

        //设置请求头
        httplib::Headers headers = {{"Authorization", "Bearer " + _api_key}};
        
        //发送POST请求
        auto response = client.Post("/v1beta/openai/chat/completions", headers, json_str, "application/json");
        if(!response)
        {
            ERR("Failed to connect to Gemini , check your netword and SSL");
            return "";
        }

        //解析响应体
        // Json::Reader reader;
        // Json::Value root;
        // if(!reader.parse(response->body, root))
        // {
        //     ERR("Failed to parse response body");
        //     return "";
        // }
        DBG("Gemini API response  status: {}", response->status);
        DBG("Gemini API response body: {}", response->body);
        
        //检测响应是否成功
        if(response->status != 200)
        {
            ERR("Gemini API returned non 200 status {}-{}", response->status,response->body);
            return ""; 
        }

        //解析响应体
        Json::Value response_json;  // 解析后的JSON对象
        Json::CharReaderBuilder reader_builder; // JSON读取器构建器
        std::string parse_errors;  // 解析错误信息
        std::istringstream response_stream(response->body); // 响应体流

        if(!Json::parseFromStream( reader_builder, response_stream, &response_json, &parse_errors)) // 解析响应体   
        {
            ERR("Failed to parse Gemini response body: {}", parse_errors);
            return "";
        }

        //解析大模型回复
        //大模型回复包含在choices数组的第一个元素的message.content中
      if(response_json.isMember("choices") && response_json["choices"].isArray() && !response_json["choices"].empty())
      {
          auto& choice  = response_json["choices"][0];
         if(choice.isMember("message")  && choice["message"].isArray())
         {      
            std::string reply_content = choice["content"]["content"].asString();
            INFO("Gemini API reply: {}", reply_content);
            return reply_content;
         }
         
      }
      ERR("Invalid response format from Gemini");
      return "Invalid response format from Gemini";

    }

    std::string GeminiProvider::sendStreamMessage(const std::vector<Message>& messages,const std::map<std::string, std::string>& request_param,std::function<void(const std::string&, bool)> callback) 
{
    
        INFO("GeminiProvider sendStreamMessage");
        if (!_isAvailable) {
            ERR("Error: GeminiProvider is not available");
            return "";
        } 
        
        //获取采样温度 和 max_tokens
        double temperature = 0.7;
        int max_tokens = 2048;

        if(request_param.find("temperature") != request_param.end())
        {
            temperature = std::stof(request_param.find("temperature")->second);
        }
        if(request_param.find("max_tokens") != request_param.end())
        {
            max_tokens = std::stoi(request_param.find("max_tokens")->second);
        }



        //构建历史信息
        Json::Value messages_array(Json::arrayValue);
        for(const auto & message : messages)
        {
            Json::Value msg;
            msg["role"] = message.role;
            msg["content"] = message.content;
            messages_array.append(msg);
        }

        //构建请求体
        Json::Value request_body;
        //request_body["model"] = "deepseek-chat";
        request_body["model"] = getModelName();
        request_body["messages"] = messages_array;
        request_body["temperature"] = temperature;
        request_body["max_tokens"] = max_tokens; 
        request_body["stream"] = true; //开启流式

        //序列化
        Json::StreamWriterBuilder writer;
        std::string     json_string = Json::writeString(writer , request_body);
        DBG("GeminiProvider: Send stream request to Gemini Server,request_body: {}", json_string);

        //创建HTTP client
        httplib::Client client (_endpoint);
        client.set_connection_timeout(30,0); //30秒超时
        client.set_read_timeout(300,0);    //流式要更长时间
        client.set_proxy("127.0.0.1", 7890);

        //设置请求头
        httplib::Headers headers = {
            {"Authorization" , "Bearer " + _api_key},
            {"Content-Type" , "application/json"},
            {"Accept" , "text/event-stream"}
        };
        
        //流式处理变量
        std::string buffer;//接受流式响应的数据块
        bool getERRor  = false; //响应是否成功
        std::string errorMsg ;//错误描述符
        int statusCode = 0 ;//状态码
        bool streamFinish =false ; //标记流式返回数据是否结束

        std::string fullResponse; //累计完整响应

        //创建请求对象
        httplib::Request req;
        req.method = "POST" ;
        req.path = "/v1beta/openai/chat/completions";
        req.headers = headers;
        req.body = json_string;

        // HTTP协议响应的格式：
// 一个状态行：HTTP/1.1 200 OK
// 一组响应头：Content-Type，Content-Length
// 空行：\r\n\r\n
// 响应体
// 普通的HTTP响应是 一头一体
// 流式响应：只有一个响应头，但响应体被拆分成多个块(chunks)陆续发送，即一头多块

        // 响应头处理
        req.response_handler = [&](const httplib::Response& response){
            statusCode = response.status;
            if(200 != statusCode)
            {
                getERRor = true;
                errorMsg = "HTTP Error" +std::to_string(statusCode);
                return false;
            }
            return true;
        };

        // 响应体处理(流式回调)
        req.content_receiver=[&](const char * data , size_t len ,uint64_t offset , uint64_t totalLength)
        {
            //请求头错误 禁止接收
            if(getERRor == true)
            {
                return false ;
            }
            //追加新数据到缓冲区
            buffer.append(data , len);
            std::cout<<"buffer: "<<buffer<<std::endl;

            //处理完所有事件  事件和事件之间以\n\n分隔
            size_t pos = 0;
            while( (pos = buffer.find("\n\n")) != std::string::npos  )
            {
                std::string event = buffer.substr(0 , pos);
                buffer.erase(0 , pos+2); //移除已经处理的事件
                //处理空行和注释 以：开头是注释行
                if(event.empty() || event[0] ==':')
                {
                    continue;
                }
                std::string jsonStr;

                if(0 == event.compare(0 , 6 ,"data: "))
                {
                    jsonStr = event.substr(6);
                    if(jsonStr == "[DONE]")
                    {
                        callback("" ,true);
                        streamFinish = true;
                        return true;
                    }
                }
               
                    //解析json数据
                    Json::Value chunk;
                    Json::CharReaderBuilder reader_builder;
                    std::string errs;
                    std::istringstream jsonStream(jsonStr);

                    if(Json::parseFromStream(reader_builder , jsonStream ,&chunk ,&errs))
                    {
                       std::string text;
                       if(chunk.isMember("choices") && chunk["choices"].isArray()&& !chunk["choices"].empty() && chunk["choices"][0].isMember("delta"))
                       {
                           auto& delta = chunk["choices"][0]["delta"];
                           if(delta.isMember("content"))
                               text = delta["content"].asString();
                           if(delta.isMember("reasoning_content"))
                               text += delta["reasoning_content"].asString();
                       }
                       if(!text.empty())
                       {
                           fullResponse += text;
                           callback(text, false);
                       }
                    }
            };
        
            return true;//继续接受数据
        };

        //发送请求并处理结果
        auto res = client.send(req);
        // send的返回值类型是Result类型，Result类型内部实现了operator bool(), 允许将Result类型的实例隐式转换为bool类型
        if(!res)
        {
            //请求连接失败，网问题，DNS解析失败等
            auto err = res.error();
            ERR("Netword error : {}" ,std::to_string(static_cast<int>(err)));
            return "";
        }

        //确保流正常
        if(!streamFinish)
        {
            WARN("Stream ended without [DONE] marker");
            callback("" , true);
        }
        return fullResponse;
    }

}