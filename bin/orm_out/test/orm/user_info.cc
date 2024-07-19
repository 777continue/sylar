#include "user_info.h"

namespace test {
namespace orm {

UserInfo::UserInfo()
    :m_status()
    ,m_id()
    ,m_name()
    ,m_email()
    ,m_phone() {
}

std::string UserInfo::toJsonString() const {
    Json::Value jvalue;
    jvalue["id"] = std::to_string(m_id);
    jvalue["name"] = m_name;
    jvalue["email"] = m_email;
    jvalue["phone"] = m_phone;
    jvalue["status"] = m_status;
    return sylar::JsonUtil::ToString(jvalue);
}

void UserInfo::setId(const int64_t& v) {
    m_id = v;
}

void UserInfo::setName(const std::string& v) {
    m_name = v;
}

void UserInfo::setEmail(const std::string& v) {
    m_email = v;
}

void UserInfo::setPhone(const std::string& v) {
    m_phone = v;
}

void UserInfo::setStatus(const int32_t& v) {
    m_status = v;
}


int UserInfoDao::Update(UserInfo::ptr info, sylar::SQLite3::ptr conn) {
    return conn->execStmt("update user set name = ?, email = ?, phone = ?, status = ? where id = ?", info->m_name, info->m_email, info->m_phone, info->m_status, info->m_id);
}

int UserInfoDao::Insert(UserInfo::ptr info, sylar::SQLite3::ptr conn) {
    int rt = conn->execStmt("insert into user (name, email, phone, status) values (?, ?, ?, ?)", info->m_name, info->m_email, info->m_phone, info->m_status);
    if(rt == SQLITE_OK) {
        info->m_id = conn->getLastInsertId();
    }
    return rt;
}

int UserInfoDao::Delete(UserInfo::ptr info, sylar::SQLite3::ptr conn) {
    return conn->execStmt("delete from user where id = ?", info->m_id);
}

int UserInfoDao::DeleteById(const int64_t& id, sylar::SQLite3::ptr conn) {
    return conn->execStmt("delete from user where id = ?", id);
}

int UserInfoDao::DeleteByName(const std::string& name, sylar::SQLite3::ptr conn) {
    return conn->execStmt("delete from user where name = ?", name);
}

int UserInfoDao::DeleteByEmail(const std::string& email, sylar::SQLite3::ptr conn) {
    return conn->execStmt("delete from user where email = ?", email);
}

int UserInfoDao::DeleteByStatus(const int32_t& status, sylar::SQLite3::ptr conn) {
    return conn->execStmt("delete from user where status = ?", status);
}

int UserInfoDao::QueryAll(std::vector<UserInfo::ptr>& results, sylar::SQLite3::ptr conn) {
    auto rt = conn->queryStmt("select id, name, email, phone, status from user");
    if(!rt) {
        return 0;
    }
    do {
        UserInfo::ptr v(new UserInfo);
        v->m_id = rt->getInt64(0);
        v->m_name = rt->getTextString(1);
        v->m_email = rt->getTextString(2);
        v->m_phone = rt->getTextString(3);
        v->m_status = rt->getInt(4);
        results.push_back(v);
    } while (rt->next());
    return 0;
}

UserInfo::ptr UserInfoDao::Query(const int64_t& id, sylar::SQLite3::ptr conn) {
    auto rt = conn->queryStmt("select id, name, email, phone, status from user where id = ?", id);
    if(!rt) {
        return nullptr;
    }
    UserInfo::ptr v(new UserInfo);
    v->m_id = rt->getInt64(0);
    v->m_name = rt->getTextString(1);
    v->m_email = rt->getTextString(2);
    v->m_phone = rt->getTextString(3);
    v->m_status = rt->getInt(4);
    return v;
}

UserInfo::ptr UserInfoDao::QueryByName(const std::string& name, sylar::SQLite3::ptr conn) {
    auto rt = conn->queryStmt("select id, name, email, phone, status from user where name = ?", name);
    if(!rt) {
        return nullptr;
    }
    UserInfo::ptr v(new UserInfo);
    v->m_id = rt->getInt64(0);
    v->m_name = rt->getTextString(1);
    v->m_email = rt->getTextString(2);
    v->m_phone = rt->getTextString(3);
    v->m_status = rt->getInt(4);
    return v;
}

UserInfo::ptr UserInfoDao::QueryByEmail(const std::string& email, sylar::SQLite3::ptr conn) {
    auto rt = conn->queryStmt("select id, name, email, phone, status from user where email = ?", email);
    if(!rt) {
        return nullptr;
    }
    UserInfo::ptr v(new UserInfo);
    v->m_id = rt->getInt64(0);
    v->m_name = rt->getTextString(1);
    v->m_email = rt->getTextString(2);
    v->m_phone = rt->getTextString(3);
    v->m_status = rt->getInt(4);
    return v;
}

int UserInfoDao::QueryByStatus(std::vector<UserInfo::ptr>& results, const int32_t& status, sylar::SQLite3::ptr conn) {
    auto rt = conn->queryStmt("select id, name, email, phone, status from user where status = ?", status);
    if(!rt) {
        return 0;
    }
    do {
        UserInfo::ptr v(new UserInfo);
        v->m_id = rt->getInt64(0);
        v->m_name = rt->getTextString(1);
        v->m_email = rt->getTextString(2);
        v->m_phone = rt->getTextString(3);
        v->m_status = rt->getInt(4);
        results.push_back(v);
    } while (rt->next());
    return 0;
}

int UserInfoDao::CreateTable(sylar::SQLite3::ptr conn) {
    return conn->execute("CREATE TABLE user(id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT NOT NULL, email TEXT NOT NULL, phone TEXT NOT NULL, status INTEGER NOT NULL);CREATE UNIQUE INDEX user_name ON user(name);CREATE UNIQUE INDEX user_email ON user(email);CREATE INDEX user_status ON user(status);");
}
} //namespace orm
} //namespace test
