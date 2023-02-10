#include "pch.hpp"
#include "SyncQueue.hpp"
#include "sql/SqlFunctions.hpp"

using namespace mi0::sync;
 
template <typename T>
void SyncQueue<T>::Push(std::unique_ptr<T> element) noexcept {
    std::unique_lock lck(_mtx);
    push_no_sync(std::forward<decltype(element)>(element));
}

template <typename T>
[[nodiscard]] auto SyncQueue<T>::TryPush(std::unique_ptr<T> element) noexcept -> bool {
    if (_mtx.try_lock()) {
        push_no_sync(std::forward<decltype(element)>(element));
        _mtx.unlock();
        return true;
    }

    return false;
}

template <typename T>
[[nodiscard]] auto SyncQueue<T>::Pop() noexcept -> std::unique_ptr<T> {
    std::unique_lock lck(_mtx);

    _cv.wait(lck, [&] { return Size() > 0; });

    return pop_no_sync();
}

template <typename T>
[[nodiscard]] auto SyncQueue<T>::TryPop() noexcept -> std::optional<std::unique_ptr<T>> {
    if (_mtx.try_lock()) {
        auto res = pop_no_sync();
        _mtx.unlock();
        return res;
    }

    return {};
}

template <typename T>
[[nodiscard]] auto SyncQueue<T>::pop_no_sync() noexcept -> std::unique_ptr<T> {
    auto res = std::move(_queue.front());
    _queue.pop();
    return res;
}

template <typename T>
void SyncQueue<T>::push_no_sync(std::unique_ptr<T> obj) noexcept {
    _queue.push(std::move(obj));
    _cv.notify_one();
}

void do_nothing() {
    SyncQueue<RequestExecutor> asd;
}
