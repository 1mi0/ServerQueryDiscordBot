#include "pch.hpp"

class ServerManagerTimer {
public:
    bool Tick() {
        using namespace std::chrono_literals;

        std::unique_lock lock(_mtx);
        bool woken = !_cv.wait_for(lock, 1min, [&] { return _woken; });
        _woken = false;
        return woken;
    }

    void EarlyWake() {
        std::unique_lock lock(_mtx);
        _woken = true;
        _cv.notify_all();
    }

    void Kill() {
        std::unique_lock lock(_mtx);
        _woken = true;
        _should_die = true;
        _cv.notify_all();
    }

    [[nodiscard]] bool IsDead() noexcept {
        return _should_die;
    }

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

        channel_details(std::string ip, uint16_t port, dpp::snowflake channel)
            : ip(ip), port(port), channel(channel) { }

        channel_details(const channel_details& obj)
            : ip(obj.ip), port(obj.port), channel(obj.channel) { }

        channel_details(channel_details&& obj)
            : ip(std::move(obj.ip)), port(std::move(obj.port)), channel(std::move(obj.channel)) { }
    };

public:
    void push_back(const channel_details chan) {
        std::unique_lock lock(_mtx);
        _vec.push_back(std::move(chan));
    }

    bool remove(dpp::snowflake channel_id) {
        std::unique_lock lock(_mtx);
        auto found = std::find_if(_vec.begin(), _vec.end(), [channel_id](auto& el) {
            return el.channel == channel_id;
        });

        if (found == _vec.end()) {
            return false;
        }
        lock.unlock();

        found->in_use = false;
        return true;
    }

    bool for_each_channel(std::function<void(const channel_details&)> f) {
        std::unique_lock lock(_mtx);
        if (_vec.empty()) {
            return false;
        }

        bool changed = false;
        for (const auto& ch : _vec) {
            if (ch.in_use) {
                changed = true;
                f(ch);
            }
        }
        return changed;
    }

private:
    std::mutex                   _mtx;
    std::vector<channel_details> _vec;
};
