#pragma once

#include "pch.hpp"

namespace mi0::sync {

template <typename T>
class QueueReader {
public:
    virtual ~QueueReader() = default;

    virtual auto               Pop()    noexcept -> std::unique_ptr<T> = 0;
    [[nodiscard]] virtual auto TryPop() noexcept -> std::optional<std::unique_ptr<T>> = 0;

    [[nodiscard]] virtual auto Size()  const noexcept -> size_t = 0; // discourage usage of unsigned
};

template <typename T>
class QueueWriter {
public:
    virtual ~QueueWriter() = default;

    virtual void               Push(std::unique_ptr<T> element) noexcept = 0;
    [[nodiscard]] virtual auto TryPush(std::unique_ptr<T> element) noexcept -> bool = 0;
};

template <typename T>
class QueueReaderWriter : public QueueWriter<T>, QueueReader<T> {};

template <typename TData>
class SyncQueue : QueueReaderWriter<TData> {
public:
    void Push(std::unique_ptr<TData> element) noexcept override;

    [[nodiscard]] auto TryPush(std::unique_ptr<TData> element) noexcept -> bool override;

    [[nodiscard]] auto Pop() noexcept -> std::unique_ptr<TData> override;

    [[nodiscard]] auto TryPop() noexcept -> std::optional<std::unique_ptr<TData>> override;

    [[nodiscard]] auto Size() const noexcept -> size_t override {
        return _queue.size();
    }

private:
    [[nodiscard]] auto pop_no_sync() noexcept -> std::unique_ptr<TData>;

    void push_no_sync(std::unique_ptr<TData> obj) noexcept;

    std::condition_variable _cv;
    std::mutex              _mtx;
    std::queue<std::unique_ptr<TData>>       _queue;
};

}
