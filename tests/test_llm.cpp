#include <gtest/gtest.h>
#include <ai_chat_sdk/DeepSeekProvider.h>
#include <ai_chat_sdk/ChatGPTProvider.h>
#include <ai_chat_sdk/GeminiProvider.h>
#include <ai_chat_sdk/OllamaDeepSeekProvider.h>
#include <ai_chat_sdk/util/my_logger.h>
#include <ai_chat_sdk/common.h>
#include <ai_chat_sdk/chat_sdk.h>
#include <ai_chat_sdk/session_manager.h>
#include <ai_chat_sdk/dataManager.h>


// TEST(DeepSeekProviderTEST   , sendMessageDeepseek) {
//     std::map<std::string , std::string> param_map;
//     param_map["api_key"] = std::getenv("deepseek_api_key");
//    param_map["temperature"] = "0.7";
//    auto deepseek_provider = std::make_shared<chatsdk::DeepSeekProvider>();
//   ASSERT_TRUE(deepseek_provider!=nullptr);

//   deepseek_provider->initModel(param_map);
//   ASSERT_TRUE(deepseek_provider->isAvailable());

//   std::vector<std::string> messages;
//   messages.push_back("user  : 你好");
 
 
//   std::string reply = deepseek_provider->sendMessage(messages ,param_map);
//   ASSERT_TRUE(!reply.empty());
//   INFO("DeepSeek API FULLDATA: {}", reply);

// }

// TEST(DeepSeekProviderTEST   , sendMessageStreamDeepseek) {
//     std::map<std::string , std::string> param_map;
//     param_map["api_key"] = std::getenv("deepseek_api_key");
//    param_map["temperature"] = "0.7";
//    auto deepseek_provider = std::make_shared<chatsdk::DeepSeekProvider>();
//   ASSERT_TRUE(deepseek_provider!=nullptr);

//   deepseek_provider->initModel(param_map);
//   ASSERT_TRUE(deepseek_provider->isAvailable());

//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你好"});
//   messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   messages.push_back({"user" , "请问什么是SSE"});
 
//     auto write_chunk = [&](const std::string& chunk  , bool last)
//     {
//         INFO("chunk: {}" ,chunk);
//         if(last)
//         {
//             INFO("[DONE]");
//         }
//     };

//   std::string reply = deepseek_provider->sendStreamMessage(messages ,param_map ,write_chunk);
//   ASSERT_FALSE(reply.empty());
//   INFO("DeepSeek API FULLDATA: {}", reply);

// }

// TEST(ChatGPTProviderTEST   , sendMessageChatGPT) {
//     std::map<std::string , std::string> param_map;
//     param_map["api_key"] = std::getenv("chatgpt_api_key");
//    //param_map["temperature"] = "0.7";
//    param_map["endpoint"] = "https://api.1475258.xyz";
//    auto chatgpt_provider = std::make_shared<chatsdk::ChatGPTProvider>();
//   ASSERT_TRUE(chatgpt_provider!=nullptr);

//   chatgpt_provider->initModel(param_map);
//   ASSERT_TRUE(chatgpt_provider->isAvailable());
  
//   std::map<std::string , std::string> request_param;
//   request_param["max_output_tokens"] = "2048";
//   request_param["temperature"] = "0.7";


//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你好"});
//   //messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   //messages.push_back({"user" , "请问什么是SSE"});
 
//     // auto write_chunk = [&](const std::string& chunk  , bool last)
//     // {
//     //     INFO("chunk: {}" ,chunk);
//     //     if(last)
//     //     {
//     //         INFO("[DONE]");
//     //     }
//     // };
  
//   std::string reply = chatgpt_provider->sendMessage(messages ,request_param );
//   ASSERT_FALSE(reply.empty());
//   INFO("ChatGPT API FULLDATA: {}", reply);

// }

//gpt流式输出   
// TEST(ChatGPTProviderTEST   , sendMessageStreamChatGPT) {
//     std::map<std::string , std::string> param_map;
//     param_map["api_key"] = std::getenv("chatgpt_api_key");
//    param_map["temperature"] = "0.7";
//   //param_map["endpoint"] = "https://api.1475258.xyz";
//    auto chatgpt_provider = std::make_shared<chatsdk::ChatGPTProvider>();
//   ASSERT_TRUE(chatgpt_provider!=nullptr);

//   chatgpt_provider->initModel(param_map);
//   ASSERT_TRUE(chatgpt_provider->isAvailable());
  
//   // std::map<std::string , std::string> request_param;
//   // request_param["max_output_tokens"] = "2048";
//   // request_param["temperature"] = "0.7";


//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你是谁？"});
//   //messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   //messages.push_back({"user" , "请问什么是SSE"});
 
//     auto write_chunk = [&](const std::string& chunk  , bool last)
//     {
//         INFO("chunk: {}" ,chunk);
//         if(last)
//         {
//             INFO("[DONE]");
//         }
//     };
  
//   std::string reply = chatgpt_provider->sendStreamMessage(messages ,param_map ,write_chunk);
//   ASSERT_FALSE(reply.empty());
//   INFO("ChatGPT API FULLDATA: {}", reply);

// }

// //gemini 全量输出
// TEST(GeminiProviderTEST   , sendMessageGemini) {
//     std::map<std::string , std::string> param_map;
//     param_map["api_key"] = std::getenv("gemini_api_key");
//    //param_map["temperature"] = "0.7";
//   param_map["endpoint"] = "https://generativelanguage.googleapis.com";
//    auto gemini_provider = std::make_shared<chatsdk::GeminiProvider>();
//   ASSERT_TRUE(gemini_provider!=nullptr);

//   gemini_provider->initModel(param_map);
//   ASSERT_TRUE(gemini_provider->isAvailable());
  
//   std::map<std::string , std::string> request_param;
//   request_param["max_output_tokens"] = "2048";
//   request_param["temperature"] = "0.7";


//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你是谁？"});
//   //messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   //messages.push_back({"user" , "请问什么是SSE"});
 
//     // auto write_chunk = [&](const std::string& chunk  , bool last)
//     // {
//     //     INFO("chunk: {}" ,chunk);
//     //     if(last)
//     //     {
//     //         INFO("[DONE]");
//     //     }
//     // };
  
//   std::string reply = gemini_provider->sendMessage(messages ,request_param );
//   ASSERT_FALSE(reply.empty());
//   INFO("Gemini API FULLDATA: {}", reply);

// }


//gemini 流式输出
// TEST(GeminiProviderTEST   , sendMessageStreamGemini) {
//     std::map<std::string , std::string> param_map;
//     param_map["api_key"] = std::getenv("gemini_api_key");
//    //param_map["temperature"] = "0.7";
//   param_map["endpoint"] = "https://generativelanguage.googleapis.com";
//    auto gemini_provider = std::make_shared<chatsdk::GeminiProvider>();
//   ASSERT_TRUE(gemini_provider!=nullptr);

//   gemini_provider->initModel(param_map);
//   ASSERT_TRUE(gemini_provider->isAvailable());
  
//   std::map<std::string , std::string> request_param;
//   request_param["max_output_tokens"] = "2048";
//   request_param["temperature"] = "0.7";


//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你是谁？"});
//   //messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   //messages.push_back({"user" , "请问什么是SSE"});
 
//     auto write_chunk = [&](const std::string& chunk  , bool last)
//     {
//         INFO("chunk: {}" ,chunk);
//         if(last)
//         {
//             INFO("[DONE]");
//         }
//     };
  
//   std::string reply = gemini_provider->sendStreamMessage(messages ,request_param ,write_chunk);
//   ASSERT_FALSE(reply.empty());
//   INFO("Gemini API FULLDATA: {}", reply);
// }


//ollama deepseek 全量输出
// TEST(OllamaDeepSeekProviderTEST   , sendMessageOllamaDeepSeek) {
//     std::map<std::string , std::string> param_map;
//     param_map["model_name"] = "deepseek-r1:1.5b";
//    //param_map["temperature"] = "0.7";
//   param_map["endpoint"] = "http://127.0.0.1:11434";
//   param_map["model_desc"] = "本地部署deepseek-r1:1.5b模型, 采用专家混合架构，专注于深度理解与推理"; 

//   auto ollama_deepseek_provider = std::make_shared<chatsdk::OllamaDeepSeekProvider>();
//   ASSERT_TRUE(ollama_deepseek_provider!=nullptr);

//   ollama_deepseek_provider->initModel(param_map);
//   ASSERT_TRUE(ollama_deepseek_provider->isAvailable()); 
  
// //   std::map<std::string , std::string> request_param;
// //   request_param["max_output_tokens"] = "2048";
// //   request_param["temperature"] = "0.7";


//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你是谁？"});
//   //messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   //messages.push_back({"user" , "请问什么是SSE"});
 
//     // auto write_chunk = [&](const std::string& chunk  , bool last)
//     // {
//     //     INFO("chunk: {}" ,chunk);
//     //     if(last)
//     //     {
//     //         INFO("[DONE]");
//     //     }
//     // };
  
//   std::string reply = ollama_deepseek_provider->sendMessage(messages ,param_map );
//   ASSERT_FALSE(reply.empty());
//   INFO("OllamaDeepSeek API FULLDATA: {}", reply);

// }


//ollama deepseek 流式输出
// TEST(OllamaDeepSeekProviderTEST   , sendMessageStreamOllamaDeepSeek) {
//     std::map<std::string , std::string> param_map;
//     param_map["model_name"] = "deepseek-r1:1.5b";
//    //param_map["temperature"] = "0.7";
//   param_map["endpoint"] = "http://127.0.0.1:11434";
//   param_map["model_desc"] = "本地部署deepseek-r1:1.5b模型, 采用专家混合架构，专注于深度理解与推理"; 

//   auto ollama_deepseek_provider = std::make_shared<chatsdk::OllamaDeepSeekProvider>();
//   ASSERT_TRUE(ollama_deepseek_provider!=nullptr);

//   ollama_deepseek_provider->initModel(param_map);
//   ASSERT_TRUE(ollama_deepseek_provider->isAvailable()); 
  
// //   std::map<std::string , std::string> request_param;
// //   request_param["max_output_tokens"] = "2048";
// //   request_param["temperature"] = "0.7";


//   std::vector<chatsdk::Message> messages;
//   messages.push_back({"user" , "你是谁？"});
//   //messages.push_back({"assistant" , "你好! ,很高兴见到你，有什么可以帮你的吗？"});
//   //messages.push_back({"user" , "请问什么是SSE"});
 
//     auto write_chunk = [&](const std::string& chunk  , bool last)
//     {
//         INFO("chunk: {}" ,chunk);
//         if(last)
//         {
//             INFO("[DONE]");
//         }
//     };
  
//   std::string reply = ollama_deepseek_provider->sendStreamMessage(messages ,param_map ,write_chunk);
//   ASSERT_FALSE(reply.empty());
//   INFO("OllamaDeepSeek API FULLDATA: {}", reply);

// }

// 测试ChatSDK
TEST(ChatSDKTEST , sendMessage) {
  auto sdk = std::make_shared<chatsdk::chat_sdk>();
  ASSERT_TRUE(sdk!=nullptr);

  auto deepseekConfig = std::make_shared<chatsdk::ApiConfig>();
  ASSERT_TRUE(deepseekConfig!=nullptr);
  deepseekConfig->model_name = "deepseek-v4-pro";
  deepseekConfig->api_key = std::getenv("deepseek_api_key") ? std::getenv("deepseek_api_key") : "";
  deepseekConfig->temperature = 0.7;
  deepseekConfig->max_tokens = 2048;
  
  auto gptConfig = std::make_shared<chatsdk::ApiConfig>();
  ASSERT_TRUE(gptConfig!=nullptr);
  gptConfig->model_name = "gpt-5.4-mini";
  gptConfig->api_key = std::getenv("chatgpt_api_key") ? std::getenv("chatgpt_api_key") : "";
  gptConfig->temperature = 0.7;
  gptConfig->max_tokens = 2048;

  auto geminiConfig = std::make_shared<chatsdk::ApiConfig>();
  ASSERT_TRUE(geminiConfig!=nullptr);
  geminiConfig->model_name = "gemini-3.5-flash";
  geminiConfig->api_key = std::getenv("gemini_api_key") ? std::getenv("gemini_api_key") : "";
  geminiConfig->temperature = 0.7;
  geminiConfig->max_tokens = 2048;

  auto ollamaConfig = std::make_shared<chatsdk::OllamaConfig>();
  ASSERT_TRUE(ollamaConfig!=nullptr);
  ollamaConfig->model_name = "deepseek-r1:1.5b";
  ollamaConfig->endpoint = "http://127.0.0.1:11434";
  ollamaConfig->temperature = 0.7;
  ollamaConfig->max_tokens = 2048;
  ollamaConfig->model_desc = "本地部署deepseek-r1:1.5b模型, 采用专家混合架构，专注于深度理解与推理";


  std::vector<std::shared_ptr<chatsdk::Config>> modelConfigs = {deepseekConfig , gptConfig , geminiConfig , ollamaConfig};
  
  sdk->initModels(modelConfigs);

  std::string sessionID = sdk->createSession(geminiConfig->model_name);
  ASSERT_FALSE(sessionID.empty());

  std::string message;
  std::cout<<">>>";
  std::cin>>message;
  std::string response = sdk->sendMessage(sessionID , message);
  ASSERT_FALSE(response.empty());
  INFO("response: {}", response);

  std::cout<<">>>";
  std::cin>>message;
  response = sdk->sendMessage(sessionID , message);
  ASSERT_FALSE(response.empty());
  INFO("response: {}", response);

  auto session = sdk->getSession(sessionID);
  ASSERT_TRUE(session != nullptr);
  for(auto &msg : session->messages)
  {
    INFO("{}: {} ", msg.role , msg.content);
  }
}



int main(int argc, char **argv) {
    // 初始化日志
    my_logger::Logger::init_logger("aiChatServer" , "stdout" ,spdlog::level::debug);
    // 初始化测试
    testing::InitGoogleTest(&argc, argv);
    // 运行测试
    return RUN_ALL_TESTS();
}