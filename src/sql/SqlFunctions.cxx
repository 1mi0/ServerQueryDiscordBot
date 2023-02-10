#include "../pch.hpp"
#include "SqlFunctions.hpp"

[[nodiscard]] auto to_string(std::string const& str) -> std::string const& { return str; }

template<typename T, typename... Args>
[[nodiscard]] constexpr auto string_unpacker(T arg1, Args const&... args) -> std::string
{
    std::string result = arg1;
    using ::to_string;
    using std::to_string;
    int unpack[]{0, (result += ", " + to_string(args), 0)...};
    static_cast<void>(unpack);
    return result;
}

[[nodiscard]] auto SQL_ExecFunc(pqxx::connection& con, const std::string& fname) -> pqxx::result {
    pqxx::work txn(con);
    pqxx::result res(txn.exec(fname));
    txn.commit();
    return res;
}

template<const char* funcname, CSQLFuncArg... TArgs>
[[nodiscard]] auto SQL_CallFunc(pqxx::connection& con, const TArgs... args) -> pqxx::result {
    std::string func_name = std::string("SELECT * FROM ") + funcname + "(" + string_unpacker(args...) + ");";
    return SQL_ExecFunc(con, func_name);
}

template<const char* funcname>
[[nodiscard]] auto SQL_CallFunc(pqxx::connection& con) -> pqxx::result {
    std::string func_name = std::string("SELECT * FROM ") + funcname + "();";
    return SQL_ExecFunc(con, func_name);
}

constexpr const char sql_func_name_insert[] = "server_insert";
const SQL_IPPort_Type SQL_CallFunc_Insert = &SQL_CallFunc<sql_func_name_insert, const std::string, const uint16_t>;

constexpr const char sql_func_name_delete[] = "server_delete";
const SQL_IPPort_Type SQL_CallFunc_Delete = &SQL_CallFunc<sql_func_name_delete, const std::string, const uint16_t>;

constexpr const char sql_func_name_deactivate[] = "server_deactivate";
const SQL_IPPort_Type SQL_CallFunc_Deactivate = &SQL_CallFunc<sql_func_name_deactivate, const std::string, const uint16_t>;

constexpr const char sql_func_name_is_active[] = "server_isactive";
const SQL_IPPort_Type SQL_CallFunc_IsActive = &SQL_CallFunc<sql_func_name_is_active, const std::string, const uint16_t>;

constexpr const char sql_func_name_select_active[] = "server_selectactive";
const SQL_ConOnly_Type SQL_CallFunc_SelectActive = &SQL_CallFunc<sql_func_name_select_active>;

constexpr const char sql_func_name_channel_insert[] = "channel_insert";
const SQL_IPPortCidGid_Type SQL_CallFunc_Channel_Insert = &SQL_CallFunc<sql_func_name_channel_insert, const std::string, const uint16_t, const uint64_t, const uint64_t>;
