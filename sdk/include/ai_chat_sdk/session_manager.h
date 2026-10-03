#pragma once
#include "common.h"
#include <unordered_map>
#include <vector>
#include <mutex>
#include <memory>
#include <atomic>
#include "dataManager.h"

namespace chatsdk {

    class SessionManager {
    public:
        SessionManager();
        ~SessionManager();

        // 创建新会话. model_name : 模型名称 返回会话id
        std::string create_session(const std::string& model_name);

        // 获取具体会话，通过会话id获取. session_id : 会话id 返回会话指针
        std::shared_ptr<Session> get_session(const std::string& session_id);

        // 添加消息到会话. session_id : 会话id, message : 消息 返回是否添加成功
        bool add_message(const std::string& session_id, const Message& message);

        // 获取会话历史，即获取具体某次会话的所有消息. session_id : 会话id 返回消息列表
        std::vector<Message> get_session_history(const std::string& session_id);

        // 获取会话列表 返回会话列表
        std::vector<std::string> get_session_list() const;

        // 删除会话. session_id : 会话id 返回是否删除成功
        bool delete_session(const std::string& session_id);

        // 更新会话时间戳. session_id : 会话id
        void update_session_timestamp(const std::string& session_id);

        // 清空所有会话
        void clear_all_sessions();

        // 获取会话总数 返回会话总数
        size_t get_session_count() const;

    private:
        // 生成唯一会话id 返回会话id
        std::string generate_session_id();

        // 生成唯一消息id 返回消息id
        std::string generate_message_id();

    private:
        // 管理所有会话，key为session_id, value为会话指针
        std::unordered_map<std::string, std::shared_ptr<Session>> _sessions;

        // 在const成员函数中，可能会修改所的状态，因此需要mutable修饰
        mutable std::mutex _mutex;

        // 记录所有会话中消息总数
        static std::atomic<int64_t> _message_count ;

        // 记录会话总数
        static std::atomic<int64_t> _session_count ;

        DataManager _data_manager;
    };
} //end namespace chatsdk