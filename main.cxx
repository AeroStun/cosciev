
#include <cassert>
#include <cstdlib>

#include <chrono>
#include <coroutine>
#include <iostream>
#include <optional>
#include <utility>

#include <nvscievent.h>

namespace cosciev {

template <class T>
struct Async {
  private:
    struct Promise;
    friend Promise;
    using handle_type = std::coroutine_handle<Promise>;

    struct Awaiter;
    friend Awaiter;
    using awaiter_type = Awaiter;

    handle_type m_handle;

    Async(const handle_type handle) noexcept : m_handle{handle} {}

  public:
    using promise_type = Promise;

    ~Async() noexcept { m_handle.destroy(); }

    Awaiter operator co_await() const { return {m_handle}; }
};

template <class T>
struct Async<T>::Promise {
    std::optional<T> m_value;
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

    void return_value(T value) {
        assert(!m_value);
        m_value.emplace(std::move(value));
    }
};

template <class T>
struct Async<T>::Awaiter {
    Async<T>::handle_type m_handle;

    [[nodiscard]] bool await_ready() const noexcept { return m_handle.promise().m_value.has_value(); }
    void await_suspend(auto cont) const noexcept { m_handle.promise().m_cont = cont; }
    [[nodiscard]] T await_resume() {
        auto& opt = m_handle.promise().m_value;
        assert(opt);
        return std::move(*opt);
    }
};

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

struct Context {
    ::NvSciEventLoopService* m_loop_service = nullptr;
    bool m_done = false;

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

    [[nodiscard]] LocalEvent make_event() {
        LocalEvent local_event;
        m_loop_service->EventService.CreateLocalEvent(&m_loop_service->EventService, &local_event.m_local_event);
        return local_event;
    }

    void run(std::chrono::microseconds timeout = std::chrono::microseconds{100'000}) {
        while (!m_done)
            m_loop_service->WaitForMultipleEventsExt(&m_loop_service->EventService, nullptr, 0uz, timeout.count(),
                                                     nullptr);
    }

    void shutdown() { m_done = true; }
};

} // namespace cosciev

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
