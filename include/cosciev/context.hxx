/*
 *  cosciev/context.hxx
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

#include <chrono>
#include <utility>

#include <nvscievent.h>

#include "cosciev/local_event.hxx"

namespace cosciev {

struct Context {
    ::NvSciEventLoopService* m_loop_service = nullptr;

    Context() {
        const auto error = ::NvSciEventLoopServiceCreateSafe(1uz, nullptr, &m_loop_service);
        assert(error == ::NvSciError_Success);
    }

    ~Context() noexcept {
        if (m_loop_service) {
            m_loop_service->EventService.Delete(&m_loop_service->EventService);
            m_loop_service = nullptr;
        }
    }

    [[nodiscard]] constexpr operator NvSciEventService*() const& noexcept { return &m_loop_service->EventService; }

    [[nodiscard]] LocalEvent make_event() const {
        LocalEvent local_event;
        m_loop_service->EventService.CreateLocalEvent(&m_loop_service->EventService, &local_event.m_local_event);
        return local_event;
    }

    [[noreturn]] void run() const {
        m_loop_service->WaitForMultipleEventsExt(&m_loop_service->EventService, nullptr, 0uz, -1, nullptr);
        std::unreachable();
    }
};

} // namespace cosciev
