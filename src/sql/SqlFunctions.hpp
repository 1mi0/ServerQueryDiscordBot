#pragma once

#include "../pch.hpp"

template<typename T>
concept CSQLFuncArg = std::is_integral_v<T> || std::same_as<T, const std::string>;

template<const char* funcname, CSQLFuncArg... TArgs>
[[nodiscard]] auto SQL_CallFunc(pqxx::connection& con, const TArgs... args) -> pqxx::result;

template<const char* funcname>
[[nodiscard]] auto SQL_CallFunc(pqxx::connection& con) -> pqxx::result;

using SQL_ConOnly_Type      = pqxx::result (*)(pqxx::connection &);
using SQL_IPPort_Type       = pqxx::result (*)(pqxx::connection &, const std::string, const uint16_t);
using SQL_IPPortCidGid_Type = pqxx::result (*)(pqxx::connection &, const std::string, const uint16_t, const uint64_t, const uint64_t);

extern const SQL_IPPort_Type SQL_CallFunc_Insert;
extern const SQL_IPPort_Type SQL_CallFunc_Delete;
extern const SQL_IPPort_Type SQL_CallFunc_Deactivate;
extern const SQL_IPPort_Type SQL_CallFunc_IsActive;
extern const SQL_ConOnly_Type SQL_CallFunc_SelectActive;
extern const SQL_IPPortCidGid_Type SQL_CallFunc_Channel_Insert;
