/*
 *  cosciev/async.hxx
 *  Copyright 2025 AeroStun
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 */

#pragma once

#include <cassert>
#include <cstdlib>

#include <concepts>
#include <coroutine>
#include <optional>
#include <utility>

namespace cosciev {

template <class T>
struct Async {
  private:
    struct PromiseStorage;
    struct Promise;
    friend Promise;
    using handle_type = std::coroutine_handle<Promise>;

    struct Awaiter;
    friend Awaiter;
    using awaiter_type = Awaiter;

    handle_type m_handle;

    [[nodiscard]] Async(const handle_type handle) noexcept : m_handle{handle} {}

  public:
    using promise_type = Promise;

    ~Async() noexcept { m_handle.destroy(); }

    Awaiter operator co_await() const { return {m_handle}; }
};

template <class T>
struct Async<T>::PromiseStorage {
    std::optional<T> m_value;

    void return_value(T value) {
        assert(!m_value);
        m_value.emplace(std::move(value));
    }
};

template <>
struct Async<void>::PromiseStorage {
    struct Unit final {};
    std::optional<Unit> m_value;

    void return_void() {
        assert(!m_value);
        m_value.emplace();
    }
};

template <class T>
struct Async<T>::Promise : Async<T>::PromiseStorage {
    std::coroutine_handle<> m_cont;

    [[nodiscard]] constexpr std::suspend_never initial_suspend() noexcept { return {}; }
    [[nodiscard]] auto final_suspend() noexcept {
        struct FinalAwaiter {
            [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
            [[nodiscard]] std::coroutine_handle<> await_suspend(std::coroutine_handle<Promise> self) {
                const auto cont = std::exchange(self.promise().m_cont, {});
                return cont ? cont : std::noop_coroutine();
            }
            void await_resume() const noexcept { std::unreachable(); }
        };
        return FinalAwaiter{};
    }
    [[nodiscard]] Async<T> get_return_object() { return {std::coroutine_handle<Promise>::from_promise(*this)}; }

    void unhandled_exception() { std::abort(); }
};

template <class T>
struct Async<T>::Awaiter {
    Async<T>::handle_type m_handle;

    [[nodiscard]] bool await_ready() const noexcept { return m_handle.promise().m_value.has_value(); }
    void await_suspend(auto cont) const noexcept { m_handle.promise().m_cont = cont; }
    constexpr void await_resume() const noexcept
        requires std::same_as<T, void>
    {}
    [[nodiscard]] T await_resume()
        requires(not std::same_as<T, void>)
    {
        auto& opt = m_handle.promise().m_value;
        assert(opt);
        return std::move(*opt);
    }
};

} // namespace cosciev
