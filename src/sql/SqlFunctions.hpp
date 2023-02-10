#pragma once

#include "../pch.hpp"

template<typename T>
concept CSQLFuncArg = std::is_integral_v<T> || std::same_as<T, const std::string>;

[[nodiscard]] static inline auto to_string(std::string const& str) -> std::string const& { return str; }

template<typename T, typename... Args>
[[nodiscard]] static inline constexpr auto string_unpacker(T arg1, Args const&... args) -> std::string {
    std::string result = arg1;
    using ::to_string;
    using std::to_string;
    int unpack[]{0, (result += ", " + to_string(args), 0)...};
    static_cast<void>(unpack);
    return result;
}

[[nodiscard]] static inline auto SQL_ExecFunc(pqxx::connection& con, const std::string& fname) -> pqxx::result {
    pqxx::work txn(con);
    pqxx::result res(txn.exec(fname));
    txn.commit();
    return res;
}

template<const char* funcname, CSQLFuncArg... TArgs>
[[nodiscard]] static inline auto SQL_CallFunc(pqxx::connection& con, const TArgs... args) -> pqxx::result {
    std::string func_name = std::string("SELECT * FROM ") + funcname + "(" + string_unpacker(args...) + ");";
    return SQL_ExecFunc(con, func_name);
}

template<const char* funcname>
[[nodiscard]] static inline auto SQL_CallFunc(pqxx::connection& con) -> pqxx::result {
    std::string func_name = std::string("SELECT * FROM ") + funcname + "();";
    return SQL_ExecFunc(con, func_name);
}

class RequestExecutor {
public:
    virtual ~RequestExecutor() = default;
    virtual auto Execute(pqxx::connection &con) -> void = 0;
};

template <const char* funcname, CSQLFuncArg... TArgs>
class SQLRequest : public RequestExecutor {
public:
    [[nodiscard]] SQLRequest(const TArgs... args) noexcept : _args(std::forward<TArgs>(args)...) {}
    auto Execute(pqxx::connection &con) -> void override {
        auto res = std::apply([&con](TArgs... args) -> pqxx::result {
            return SQL_CallFunc<funcname, TArgs...>(con, args...);
        }, _args);

        _res_data.set_value(res);
    }

    [[nodiscard]] auto GetFuture() noexcept -> std::future<pqxx::result> {
        return _res_data.get_future();
    }

private:
    std::promise<pqxx::result> _res_data;
    std::tuple<TArgs...>       _args;
};

constexpr const char sql_func_name_insert[] = "server_insert";
using SQL_Request_Insert = SQLRequest<sql_func_name_insert, const std::string, const uint16_t>;

constexpr const char sql_func_name_delete[] = "server_delete";
using SQL_Request_Delete = SQLRequest<sql_func_name_delete, const std::string, const uint16_t>;

constexpr const char sql_func_name_deactivate[] = "server_deactivate";
using SQL_Request_Deactivate = SQLRequest<sql_func_name_deactivate, const std::string, const uint16_t>;

constexpr const char sql_func_name_is_active[] = "server_isactive";
using SQL_Request_IsActive = SQLRequest<sql_func_name_is_active, const std::string, const uint16_t>;

constexpr const char sql_func_name_select_active[] = "server_selectactive";
using SQL_Request_SelectActive = SQLRequest<sql_func_name_select_active>;

constexpr const char sql_func_name_channel_insert[] = "channel_insert";
using SQL_Request_Channel_Insert = SQLRequest<sql_func_name_channel_insert, const std::string, const uint16_t, const uint64_t, const uint64_t>;
