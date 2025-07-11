/*
 *  demo.cxx
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

#include <cassert>
#include <cstdlib>

#include <iostream>

#include <cosciev.hxx>

cosciev::Main co_main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    const auto& ctx = co_await cosciev::getContext();

    const auto event_a = ctx.make_event();
    const auto event_b = ctx.make_event();

    const auto a = [](const auto& event_a, const auto& event_b) -> cosciev::Async<int> {
        for (auto i = 0uz; i < 16uz; ++i) {
            event_a.signal();
            co_await event_b;
            std::cout << "A: " << i << '\n';
        }

        co_return 70;
    }(event_a, event_b);

    const auto b = [](const auto& event_a, const auto& event_b) -> cosciev::Async<int> {
        for (auto i = 0uz; i < 16uz; ++i) {
            co_await event_a;
            event_b.signal();
            std::cout << "B: " << i << '\n';
        }

        co_return 7;
    }(event_a, event_b);

    const auto res = co_await a + co_await b;
    std::cout << "Result: " << res << '\n';

    co_return EXIT_SUCCESS;
}
