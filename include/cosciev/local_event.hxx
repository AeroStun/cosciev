/*
 *  cosciev/local_event.hxx
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

#include <utility>

#include <nvscievent.h>

#include "cosciev/event.hxx"

namespace cosciev {

struct LocalEvent {
    ::NvSciLocalEvent* m_local_event{};

    constexpr LocalEvent() = default;
    LocalEvent(const LocalEvent&) = delete;
    constexpr LocalEvent(LocalEvent&& from) noexcept : m_local_event{std::exchange(from.m_local_event, nullptr)} {}
    LocalEvent& operator=(const LocalEvent&) = delete;
    constexpr LocalEvent& operator=(LocalEvent&& from) noexcept {
        auto other = std::move(from);
        std::swap(m_local_event, other.m_local_event);
        return *this;
    };

    ~LocalEvent() noexcept {
        if (m_local_event) {
            m_local_event->eventNotifier->Delete(m_local_event->eventNotifier);
            m_local_event->Delete(m_local_event);
            m_local_event = nullptr;
        }
    }

    void signal() const { m_local_event->Signal(m_local_event); }

    EventAwaiter operator co_await() const { return {m_local_event->eventNotifier}; }
};

} // namespace cosciev
