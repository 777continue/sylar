#ifndef __TEST_ORMUSER_INFO_H__
#define __TEST_ORMUSER_INFO_H__

#include <json/json.h>
#include <vector>
#include "sylar/db/sqlite3.h"
#include "sylar/util.h"


namespace test {
namespace orm {

class UserInfoDao;
class UserInfo {
friend class UserInfoDao;
public:
    typedef std::shared_ptr<UserInfo> ptr;

    UserInfo();

    const int64_t& getId() { return m_id; }
    void setId(const int64_t& v);

    const std::string& getName() { return m_name; }
    void setName(const std::string& v);

    const std::string& getEmail() { return m_email; }
    void setEmail(const std::string& v);

    const std::string& getPhone() { return m_phone; }
    void setPhone(const std::string& v);

    const int32_t& getStatus() { return m_status; }
    void setStatus(const int32_t& v);

    std::string toJsonString() const;

private:
    int32_t m_status;
    int64_t m_id;
    std::string m_name;
    std::string m_email;
    std::string m_phone;
};


class UserInfoDao {
public:
    typedef std::shared_ptr<UserInfoDao> ptr;
    static int Update(UserInfo::ptr info, sylar::SQLite3::ptr conn);
    static int Insert(UserInfo::ptr info, sylar::SQLite3::ptr conn);
    static int Delete(UserInfo::ptr info, sylar::SQLite3::ptr conn);
    static int Delete(const int64_t& id, sylar::SQLite3::ptr conn);
    static int DeleteById(const int64_t& id, sylar::SQLite3::ptr conn);
    static int DeleteByName(const std::string& name, sylar::SQLite3::ptr conn);
    static int DeleteByEmail(const std::string& email, sylar::SQLite3::ptr conn);
    static int DeleteByStatus(const int32_t& status, sylar::SQLite3::ptr conn);
    static int QueryAll(std::vector<UserInfo::ptr>& results, sylar::SQLite3::ptr conn);
    static UserInfo::ptr Query(const int64_t& id, sylar::SQLite3::ptr conn);
    static UserInfo::ptr QueryByName(const std::string& name, sylar::SQLite3::ptr conn);
    static UserInfo::ptr QueryByEmail(const std::string& email, sylar::SQLite3::ptr conn);
    static int QueryByStatus(std::vector<UserInfo::ptr>& results, const int32_t& status, sylar::SQLite3::ptr conn);
    static int CreateTable(sylar::SQLite3::ptr info);
};

} //namespace orm
} //namespace test
#endif //__TEST_ORMUSER_INFO_H__
