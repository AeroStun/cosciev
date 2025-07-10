
#include <cassert>
#include <cstdlib>

#include <chrono>
#include <coroutine>
#include <iostream>
#include <optional>
#include <utility>

#include <cosciev.hxx>

int main() {

    cosciev::Context ctx;

    const auto co_main = [](auto& ctx) -> cosciev::Async<int> {
        std::cout << "In co_main\n";

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
        std::cout << "co_main: " << res << '\n';

        ctx.shutdown();
        std::cout << "Exiting co_main\n";
        co_return res;
    }(ctx);

    std::cout << "Yes\n";
    ctx.run();
}
