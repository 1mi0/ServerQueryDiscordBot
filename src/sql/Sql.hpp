#pragma once

#include "../pch.hpp"
#include "SqlFunctions.hpp"

struct server_active {
    bool active;
};

struct channel_server {
    uint64_t channel_id;
    uint64_t message_id;
};

struct discord_channel {
    uint64_t channel_id;
    uint64_t guild_id;
};

struct server_details {
    std::string                    address;
    uint16_t                       port;
    
    server_details(std::string addr, uint16_t port) noexcept
        : address(std::move(addr)), port(port) {}

    std::optional<server_active>   active;
    std::optional<channel_server>  channel;
    std::optional<discord_channel> discord_channel;
};

class SQL {
public:
    [[nodiscard]] SQL();

    std::function<pqxx::result(std::string, uint16_t)> Insert = [this](const std::string P1, const uint16_t P2) -> pqxx::result {
        return SQL_CallFunc_Insert(this->_con, std::forward<decltype(P1)>(P1), std::forward<decltype(P2)>(P2));
    };

    std::function<pqxx::result(std::string, uint16_t)> Delete = [this](const std::string P1, const uint16_t P2) -> pqxx::result {
        return SQL_CallFunc_Delete(this->_con, std::forward<decltype(P1)>(P1), std::forward<decltype(P2)>(P2));
    };

    std::function<pqxx::result(std::string, uint16_t)> DeactivateServer = [this](const std::string P1, const uint16_t P2) -> pqxx::result {
        return SQL_CallFunc_Deactivate(this->_con, std::forward<decltype(P1)>(P1), std::forward<decltype(P2)>(P2));
    };

    std::function<pqxx::result(std::string, uint16_t)> IsActive = [this](const std::string P1, const uint16_t P2) -> pqxx::result {
        return SQL_CallFunc_IsActive(this->_con, std::forward<decltype(P1)>(P1), std::forward<decltype(P2)>(P2));
    };

    std::function<pqxx::result()> GetActiveServers = [this]() -> pqxx::result {
        return SQL_CallFunc_SelectActive(this->_con);
    };

    std::function<pqxx::result(const std::string, const uint16_t, const uint64_t, const uint64_t)> ChannelInsert = [this](const std::string P1, const uint16_t P2, const uint64_t P3, const uint64_t P4) -> pqxx::result {
        return SQL_CallFunc_Channel_Insert(this->_con, std::forward<decltype(P1)>(P1), std::forward<decltype(P2)>(P2), std::forward<decltype(P3)>(P3), std::forward<decltype(P4)>(P4));
    };

  private:
    pqxx::connection _con;

};
