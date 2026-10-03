#pragma once
#include <string>
#include <vector>
#include <memory>
#include <mutex>
#include "common.h"

struct sqlite3;

namespace chatsdk {

    class DataManager {
    public:
        DataManager(const std::string& db_name);
        ~DataManager();

        //session相关操作
        //插入新会话
        bool insertSession(const Session& session);
        //获取指定会话
        std::shared_ptr<Session> getSession(const std::string& session_id) const;
        //更新指定会话的时间戳
        void updateSessionTimestamp(const std::string& session_id, std::time_t timestamp);
        //删除指定会话 会话删除后该会话管理的历史消息也会被删除
        bool deleteSession(const std::string& session_id);
        //获取所有会话ID
        std::vector<std::string> getAllSessionIds() const;
        //获取所有会话的信息
        std::vector<std::shared_ptr<Session>> getAllSessions() const;
        //删除所有会话
        bool deleteAllSessions();
        //获取所有会话的个数
        size_t getSessionCount() const;

        //message相关操作
        //插入新消息
        bool insertMessage(const std::string& session_id, const Message& message);
        //获取指定会话的历史消息
        std::vector<Message> getMessagesbySessionId(const std::string& session_id) const;
        //删除指定会话的历史消息
        bool deleteMessagesbySessionId(const std::string& session_id);

    private:
        //初始化数据库 创建表
        bool initDatabase();

        //执行sql语句的工具函数
        bool executeSql(const std::string& sql);


    private:
        sqlite3* _db;
        std::string _db_name;
        mutable std::mutex _mutex; //mutable允许在const成员函数中修改这个互斥锁 用于保护线程安全
    };

} //end namespace chatsdk

