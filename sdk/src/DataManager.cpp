#include <ai_chat_sdk/dataManager.h>
#include <ai_chat_sdk/util/my_logger.h>
#include <sqlite3.h>

namespace chatsdk {

    DataManager::DataManager(const std::string& db_name) :_db(nullptr), _db_name(db_name) {
        //创建或打开数据库连接
        if(sqlite3_open(_db_name.c_str(), &_db) != SQLITE_OK) {
            ERR("Failed to open database: %s", sqlite3_errmsg(_db));
        }
        INFO("Database %s opened successfully", _db_name.c_str());

        //初始化数据库 创建表
        if(!initDatabase()) {
            sqlite3_close(_db);
            _db = nullptr;
            ERR("Failed to initialize database");
        }
    }

    DataManager::~DataManager() {
        if(_db) {
            sqlite3_close(_db);
            _db = nullptr;
            INFO("Database %s closed successfully", _db_name.c_str());
        }
    }

    bool DataManager::initDatabase() {
        std::lock_guard<std::mutex> lock(_mutex);

        //创建会话表
        const std::string create_sessions_table = "CREATE TABLE IF NOT EXISTS sessions ("
        " session_id TEXT PRIMARY KEY, "
        " model_name TEXT NOT NULL, "
        " create_time INTEGER NOT NULL ,"
        " update_time INTEGER NOT NULL "
        ")";
        if(!executeSql(create_sessions_table)) {
            ERR("Failed to create sessions table");
            return false;
        }
        //创建消息表
        const std::string create_messages_table = "CREATE TABLE IF NOT EXISTS messages ("
        " message_id TEXT PRIMARY KEY, "
         " session_id TEXT, "
         " role TEXT NOT NULL, "
        " content TEXT, "
        " timestamp INTEGER NOT NULL,"
        " FOREIGN KEY (session_id) REFERENCES sessions(session_id) ON DELETE CASCADE "
        ")";
        if(!executeSql(create_messages_table)) {
            return false;
        }
        //创建索引加速查询
        const std::string create_messages_index = "CREATE INDEX IF NOT EXISTS idx_messages_session_id ON messages (session_id)";
        if(!executeSql(create_messages_index)) {
            return false;
        }

        INFO("Database %s initialized successfully", _db_name.c_str());
        return true;
    }

    //辅助方法
    bool DataManager::executeSql(const std::string& sql) {
        char* errMsg = nullptr;
        if(sqlite3_exec(_db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
            ERR("Failed to execute SQL: %s", errMsg);
            sqlite3_free(errMsg);
            return false;
        }
        return true;
    }

    //插入会话
    bool DataManager::insertSession(const Session& session) {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string insert_sql = "INSERT INTO sessions (session_id, model_name, create_time, update_time) VALUES (?, ?, ?, ?)";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, insert_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt, 1, session.session_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, session.model_name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, static_cast<int64_t>(session.create_time));
        sqlite3_bind_int64(stmt, 4, static_cast<int64_t>(session.update_time));
        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            ERR("Failed to execute SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //释放语句
        sqlite3_finalize(stmt);
        INFO("Session %s inserted successfully", session.session_id.c_str());
        return true;
    }

    //获取指定会话
    std::shared_ptr<Session> DataManager::getSession(const std::string& session_id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string select_sql = "SELECT model_name, create_time, update_time FROM sessions WHERE session_id = ?;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return nullptr;
        }
        //绑定参数
        sqlite3_bind_text(stmt, 1, session_id.c_str(), -1, SQLITE_TRANSIENT);
        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_ROW) {
            sqlite3_reset(stmt);
            return nullptr;
        }
        //获取结果
        std::string model_name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        auto session = std::make_shared<Session>(model_name);
        session->session_id = session_id;
        session->create_time = static_cast<std::time_t>(sqlite3_column_int64(stmt, 1));
        session->update_time = static_cast<std::time_t>(sqlite3_column_int64(stmt, 2));
        //释放语句
        sqlite3_finalize(stmt);
        //获取该会话所有消息
        session->messages = getMessagesbySessionId(session_id);
        return session;
    }

    //更新指定会话的时间戳
    void DataManager::updateSessionTimestamp(const std::string& session_id, std::time_t timestamp) {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string update_sql = "UPDATE sessions SET update_time = ? WHERE session_id = ?;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, update_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return;
        }
        //绑定参数
        sqlite3_bind_int64(stmt, 1, static_cast<int64_t>(timestamp));
        sqlite3_bind_text(stmt, 2, session_id.c_str(), -1, SQLITE_TRANSIENT);
        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            ERR("Failed to execute SQL: %s", sqlite3_errmsg(_db));
            return;
        }
        //释放语句
        sqlite3_finalize(stmt);
        INFO("Session %s timestamp updated successfully", session_id.c_str());
    }

    //删除指定会话
    bool DataManager::deleteSession(const std::string& session_id) {
        // 从数据库中删除会话的所有消息，在删除会话列表
        // 否则，该函数中已经加锁了，在未退出前调用deleteMessagesBySessionId，会重新加锁
        // 就导致死锁
        deleteMessagesbySessionId(session_id);

        std::lock_guard<std::mutex> lock(_mutex);
        const std::string delete_sql = "DELETE FROM sessions WHERE session_id = ?;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, delete_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt, 1, session_id.c_str(), -1, SQLITE_TRANSIENT);
        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            ERR("Failed to execute SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //释放语句
        sqlite3_finalize(stmt);
        INFO("Session %s deleted successfully", session_id.c_str());
        return true;
    }

    //获取所有会话id
    std::vector<std::string> DataManager::getAllSessionIds() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::string> session_ids;
        const std::string select_sql = "SELECT session_id FROM sessions ORDER BY update_time DESC;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return session_ids;
        }

        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            session_ids.push_back(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
        }
        //释放语句
        sqlite3_finalize(stmt);
        return session_ids;
    }

    //获取所有session信息 并按时间降序排序
    std::vector<std::shared_ptr<Session>> DataManager::getAllSessions() const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<std::shared_ptr<Session>> sessions;
        const std::string select_sql = "SELECT session_id, model_name, create_time, update_time FROM sessions ORDER BY update_time DESC;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return sessions;
        }

        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::string session_id(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            std::string model_name(reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            auto session = std::make_shared<Session>(model_name);
            session->session_id = session_id;
            session->create_time = static_cast<std::time_t>(sqlite3_column_int64(stmt, 2));
            session->update_time = static_cast<std::time_t>(sqlite3_column_int64(stmt, 3));
            sessions.push_back(session);
        }
        //释放语句
        sqlite3_finalize(stmt);
        return sessions;
    }

    //获取会话总个数
    size_t DataManager::getSessionCount() const {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string select_sql = "SELECT COUNT(*) FROM sessions;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return 0;
        }
        //执行语句
        rc = sqlite3_step(stmt);
        size_t count = 0;
        if(rc == SQLITE_ROW) {
            count = static_cast<size_t>(sqlite3_column_int64(stmt, 0));
        }
         //释放语句
        sqlite3_finalize(stmt);
        INFO("Session count: %zu", count);
        //获取结果
        return count;
    }

    //清空所有会话
    bool DataManager::deleteAllSessions() {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string delete_sql = "DELETE FROM sessions;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, delete_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            ERR("Failed to execute SQL: %s", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //释放语句
        sqlite3_finalize(stmt);
        INFO("All sessions deleted successfully");
        return true;
    }

    //在指定会话中插入新消息
    bool DataManager::insertMessage(const std::string& session_id, const Message& message) {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string insert_sql = "INSERT INTO messages (message_id, session_id, role, content, timestamp) VALUES (?, ?, ?, ?, ?);";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, insert_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt, 1, message.id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, session_id.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, message.role.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 4, message.content.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 5, static_cast<int64_t>(message.timestamp));   

        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            ERR("Failed to execute SQL: %s", sqlite3_errmsg(_db));
            sqlite3_finalize(stmt);
            return false;
        }
        //释放语句
        sqlite3_finalize(stmt);
        // 同时更新session的update_time
       const std::string update_session_sql = "UPDATE sessions SET update_time = ? WHERE session_id = ?;";
       //准备sql语句
       sqlite3_stmt* update_stmt = nullptr;
       rc = sqlite3_prepare_v2(_db, update_session_sql.c_str(), -1, &update_stmt, nullptr);
       if(rc == SQLITE_OK) {
            //绑定参数
            sqlite3_bind_int64(update_stmt, 1, static_cast<int64_t>(message.timestamp));
            sqlite3_bind_text(update_stmt, 2, session_id.c_str(), -1, SQLITE_TRANSIENT);    
            //执行语句
            rc = sqlite3_step(update_stmt);
            //释放语句
            sqlite3_finalize(update_stmt);
       }
       else
       {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
       }
       DBG("Inserted message: {} into session: {}", message.id, session_id.c_str());
       return true;
    }

    //获取指定会话的历史消息
    std::vector<Message> DataManager::getMessagesbySessionId(const std::string& session_id) const {
        std::lock_guard<std::mutex> lock(_mutex);
        std::vector<Message> messages;
        const std::string select_sql = "SELECT message_id,  role, content, timestamp FROM messages WHERE session_id = ? ORDER BY timestamp ASC;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, select_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return messages;
        }
        //绑定参数
        sqlite3_bind_text(stmt, 1, session_id.c_str(), -1, SQLITE_TRANSIENT);
        //执行语句
        while((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
            std::string message_id ( reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0)));
            std::string role ( reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1)));
            std::string content ( reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2)));
            std::time_t timestamp ( static_cast<std::time_t>(sqlite3_column_int64(stmt, 3)));
            Message message (role, content);
            message.id = message_id;
            message.timestamp = timestamp;
            messages.push_back(message);
        }
        //释放语句
        sqlite3_finalize(stmt);
        return messages;
    }

    //删除指定会话历史信息
    bool DataManager::deleteMessagesbySessionId(const std::string& session_id) {
        std::lock_guard<std::mutex> lock(_mutex);
        const std::string delete_sql = "DELETE FROM messages WHERE session_id = ?;";
        //准备sql语句
        sqlite3_stmt* stmt = nullptr;
        int rc = sqlite3_prepare_v2(_db, delete_sql.c_str(), -1, &stmt, nullptr);
        if(rc != SQLITE_OK) {
            ERR("Failed to prepare SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //绑定参数
        sqlite3_bind_text(stmt, 1, session_id.c_str(), -1, SQLITE_TRANSIENT);
        //执行语句
        rc = sqlite3_step(stmt);
        if(rc != SQLITE_DONE) {
            ERR("Failed to execute SQL: %s", sqlite3_errmsg(_db));
            return false;
        }
        //释放语句
        sqlite3_finalize(stmt);
        INFO("Deleted messages for session: {}", session_id.c_str());
        return true;
    }
    


}
        


