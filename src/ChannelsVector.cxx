#include "pch.hpp"
#include "ChannelsVector.hpp"

using namespace mi0::sync;

auto ServerManagerTimer::Tick() -> bool {
    using namespace std::chrono_literals;

    std::unique_lock lock(_mtx);
    bool woken = !_cv.wait_for(lock, 1min, [&] { return _woken; });
    _woken = false;
    return woken;
}

void ServerManagerTimer::EarlyWake() {
    std::unique_lock lock(_mtx);
    _woken = true;
    _cv.notify_all();
}

void ServerManagerTimer::Kill() {
    std::unique_lock lock(_mtx);
    _woken = true;
    _should_die = true;
    _cv.notify_all();
}

auto ServerManagerTimer::IsDead() const noexcept -> bool { return _should_die; }

ChannelsVector::channel_details::channel_details(std::string ip, uint16_t port,
                                                 dpp::snowflake channel)
    : ip(std::move(ip)), port(port), channel(channel) {}

ChannelsVector::channel_details::channel_details(const channel_details &obj)
    : ip(obj.ip), port(obj.port), channel(obj.channel) {}

ChannelsVector::channel_details::channel_details(channel_details &&obj) noexcept
    : ip(obj.ip), port(obj.port),
      channel(obj.channel) {}

void ChannelsVector::push_back(channel_details&& chan) {
    std::unique_lock lock(_mtx);
    _vec.push_back(chan);
}

auto ChannelsVector::remove(dpp::snowflake channel_id) -> bool {
    std::unique_lock lock(_mtx);
    auto found = std::find_if(_vec.begin(), _vec.end(), [channel_id](auto &elem) {
        return elem.channel == channel_id;
    });

    if (found == _vec.end()) {
        return false;
    }
    lock.unlock();

    found->in_use = false;
    return true;
}

auto ChannelsVector::for_each_channel(const std::function<void(const channel_details &)>& func) -> bool {
    std::unique_lock lock(_mtx);
    if (_vec.empty()) {
        return false;
    }

    bool changed = false;
    for (const auto &chan : _vec) {
        if (chan.in_use) {
            changed = true;
            func(chan);
        }
    }
    return changed;
}
