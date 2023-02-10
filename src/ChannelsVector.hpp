#pragma once

#include "pch.hpp"
#include <mutex>

namespace mi0::sync {

class ServerManagerTimer {
public:
    auto Tick() -> bool;

    void EarlyWake();

    void Kill();

    [[nodiscard]] auto IsDead() const noexcept -> bool;

private:
    std::mutex              _mtx;
    std::condition_variable _cv;
    bool                    _should_die = false;
    bool                    _woken = false;
};

class ChannelsVector {
public:
    struct channel_details {
        const std::string       ip;
        const uint16_t          port;
        const dpp::snowflake    channel;
        std::atomic<bool>       in_use = true;

        channel_details(std::string ip, uint16_t port, dpp::snowflake channel);

        channel_details(const channel_details &obj);

        channel_details(channel_details &&obj) noexcept ;
    };

    void push_back(channel_details&& chan);

    auto remove(dpp::snowflake channel_id) -> bool;

    auto for_each_channel(const std::function<void(const channel_details &)>& func) -> bool;

private:
    std::mutex                   _mtx;
    std::vector<channel_details> _vec;
};

}
