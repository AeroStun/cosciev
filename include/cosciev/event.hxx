/*
 *  cosciev/event.hxx
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

#include <coroutine>
#include <utility>

#include <nvscievent.h>

namespace cosciev {

struct EventAwaiter {
    ::NvSciEventNotifier* m_notifier{};
    std::coroutine_handle<void> m_handle{};

    ~EventAwaiter() noexcept {
        if (m_notifier) {
            m_notifier->SetHandler(m_notifier, nullptr, nullptr, 0u);
            m_handle = {};
        }
    }

    [[nodiscard]] constexpr bool await_ready() const noexcept { return false; }
    void await_suspend(std::coroutine_handle<void> handle) {
        m_handle = handle;
        m_notifier->SetHandler(
            m_notifier,
            +[](void* cookie) {
                auto& self = *static_cast<EventAwaiter*>(cookie);
                self.m_notifier->SetHandler(self.m_notifier, nullptr, nullptr, 0u);
                auto handle = std::exchange(self.m_handle, {});
                handle.resume();
            },
            this, 0u);
    }
    constexpr void await_resume() const noexcept {}
};

struct Event {
    ::NvSciEventNotifier* m_notifier{};

    EventAwaiter operator co_await() const { return {m_notifier}; }
};

} // namespace cosciev
