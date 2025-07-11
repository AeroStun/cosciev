/*
 *  cosciev/main.hxx
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

#include <coroutine>
#include <optional>
#include <utility>

#include "cosciev/context.hxx"

namespace cosciev {

class Main final {
    struct Promise;
    friend Promise;
    using handle_type = std::coroutine_handle<Promise>;

    handle_type m_handle;

    [[nodiscard]] constexpr Main(const handle_type handle) noexcept : m_handle{handle} {}

  public:
    using promise_type = Promise;

    ~Main() noexcept;

    [[noreturn]] void run() const;
};

class Main::Promise final {
    Context m_ctx;
    std::optional<int> m_exit_code;

  public:
    [[nodiscard]] constexpr std::suspend_never initial_suspend() noexcept { return {}; }
    [[nodiscard]] auto final_suspend() noexcept {
        struct FinalAwaiter final {
            [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
            [[noreturn]] void await_suspend(std::coroutine_handle<Promise> self) const noexcept {
                std::exit(self.promise().m_exit_code.value());
            }
            void await_resume() const noexcept { std::unreachable(); }
        };
        return FinalAwaiter{};
    }
    [[nodiscard]] Main get_return_object() { return {std::coroutine_handle<Promise>::from_promise(*this)}; }

    [[noreturn]] void unhandled_exception() const noexcept { std::abort(); }

    void return_value(int exit_code) noexcept {
        assert(!m_exit_code);
        m_exit_code.emplace(exit_code);
    }

    [[nodiscard]] constexpr const Context& context() const noexcept { return m_ctx; }
};

class ContextAwaiter final {
  public:
    [[nodiscard]] constexpr bool await_ready() noexcept { return false; }

    [[nodiscard]] auto await_suspend(std::coroutine_handle<Main::promise_type> cont) noexcept {
        m_ctx_ptr = &cont.promise().context();
        return cont;
    }

    [[nodiscard]] const Context& await_resume() const noexcept { return *m_ctx_ptr; }

  private:
    const Context* m_ctx_ptr = nullptr;
};

constexpr ContextAwaiter getContext() noexcept { return {}; }

} // namespace cosciev

cosciev::Main co_main(int argc, char** argv);
