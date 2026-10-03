#include <ai_chat_sdk/session_manager.h>
#include <sstream>
#include <ai_chat_sdk/util/my_logger.h>
#include <iomanip>

namespace chatsdk {

    std::atomic<int64_t> SessionManager::_message_count = 0;
    std::atomic<int64_t> SessionManager::_session_count = 0;
    
    SessionManager::SessionManager() : _data_manager("chatDB.db") {
       //获取所有会话
       auto sessions = _data_manager.getAllSessions();
       for(auto& session : sessions) {
           _sessions[session->session_id] = session;
       }
       INFO("Loaded {} sessions from database", sessions.size());
    }
    SessionManager::~SessionManager() {
    }
    //生成消息id
    std::string SessionManager::generate_message_id() {
        //消息计数自增
        _message_count.fetch_add(1);
        std::time_t time = std::time(nullptr);
       
        //消息id格式为 消息_时间戳_消息计数
        std::ostringstream ss;
        ss << "msg_" << time <<  "_" <<std::setfill('0') << std::setw(8) << _message_count;
        return ss.str();
    }

    //生成会话id
    std::string SessionManager::generate_session_id() {
        //会话计数自增
        _session_count.fetch_add(1);
        std::time_t time = std::time(nullptr);
       
        //会话id格式为 会话_时间戳_会话计数
        std::ostringstream ss;
        ss << "session_" << time <<  "_" <<std::setfill('0') << std::setw(8) << _session_count;
        return ss.str();
    }

    //创建会话
    std::string SessionManager::create_session(const std::string& model_name) {
        std::lock_guard<std::mutex> lock(_mutex);
        
        std::string session_id = generate_session_id();
        auto session = std::make_shared<Session>(model_name);
        session->session_id = session_id;
        _sessions[session_id] = session;
        INFO("create session: %s, model: %s", session_id.c_str(), model_name.c_str());
        _data_manager.insertSession(*session);
        return session_id;
    }

    //获取会话
   std::shared_ptr<Session> SessionManager::get_session(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(session_id);
        if(it != _sessions.end()) {
            it->second->messages = _data_manager.getMessagesbySessionId(session_id);
            return it->second;
        }
       auto sessionPtr = _data_manager.getSession(session_id);
       if(sessionPtr) {
           auto it2 = _sessions.find(session_id);
           if(it2 == _sessions.end()) {
              _sessions[session_id] = sessionPtr;
           }
           sessionPtr->messages = _data_manager.getMessagesbySessionId(session_id);
           return sessionPtr;
       }
       WARN("session not found in database: %s", session_id.c_str());
       return nullptr;
    }

    //添加消息到会话
    bool SessionManager::add_message(const std::string& session_id, const Message& message) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(session_id);
        if(it == _sessions.end()) {
            return false;
        }
        Message msg(message.role ,message.content);
        msg.id = generate_message_id();
       
        it->second->messages.push_back(msg);
        it->second->update_time = std::time(nullptr);
        _data_manager.insertMessage(session_id, msg);
        INFO("add message to session: %s, msg: %s", session_id.c_str(), msg.id.c_str());
        return true;
    }

    //获取会话历史消息
    std::vector<Message> SessionManager::get_session_history(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(session_id);
        if(it == _sessions.end()) {
            return {};
        }
        return _data_manager.getMessagesbySessionId(session_id);
    }

    //获取会话列表, 包含sessionId和modelName
    std::vector<std::string> SessionManager::get_session_list() const {
        auto sessions = _data_manager.getAllSessions();
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::pair<std::time_t,std::shared_ptr<Session>>> temp;
        temp.reserve(sessions.size() + _sessions.size());
        for(const auto& it : _sessions) {
            temp.emplace_back(it.second->update_time, it.second);
        }
        for(const auto& it : sessions) {
            if(_sessions.find(it->session_id) == _sessions.end()) {
                temp.emplace_back(it->update_time, it);
            }
        }
        std::sort(temp.begin(), temp.end(), [](const auto& a, const auto& b) {
            return a.first > b.first;
        });

        std::vector<std::string> session_ids;
        session_ids.reserve(temp.size());
        for(const auto& it : temp) {
            session_ids.push_back(it.second->session_id);
        }
        return session_ids;
    }

    //删除会话
    bool SessionManager::delete_session(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(session_id);
        if(it == _sessions.end()) {
            INFO("session: %s not found", session_id.c_str());
            return false;
        }
        _sessions.erase(it);
        INFO("delete session: %s", session_id.c_str());
        _data_manager.deleteSession(session_id);
        return true;
    }

    //更新会话时间戳
    void SessionManager::update_session_timestamp(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        auto it = _sessions.find(session_id);
        if(it != _sessions.end()) {
            it->second->update_time = std::time(nullptr);
            _data_manager.updateSessionTimestamp(session_id, it->second->update_time);
        }
    }

    //清空所有会话
    void SessionManager::clear_all_sessions() {
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.clear();
        INFO("clear all sessions");
    }

    //获取会话总数
    size_t SessionManager::get_session_count() const {
        std::lock_guard<std::mutex> lock(_mutex);
        return _data_manager.getSessionCount();
    }


} //end namespace chatsdk