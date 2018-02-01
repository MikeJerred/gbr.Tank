#pragma once

#include <Windows.h>
#include <chrono>
#include <experimental/resumable>
#include <functional>
#include <vector>

namespace gbr::Tank::Utilities {

	/*template<typename T> class Promise {
	private:
		std::function<void(T)> todo;
		std::vector<Promise<T>> children;
	public:
		Promise<T>();

		Promise<T> Then(std::function<void(T)> success);
		void Resolve();
	};*/

	struct SleepAwaitable {
	private:
		int _tickToResumeAt;
		std::experimental::coroutine_handle<void> _awaitingCoroutine;
	public:
		SleepAwaitable(int millisecondsToSleep) noexcept {
			_tickToResumeAt = GetTickCount() + millisecondsToSleep;
		}

		bool hasFinished() const noexcept {
			return GetTickCount() > _tickToResumeAt;
		}

		void complete() noexcept {
			_awaitingCoroutine.resume();
		}

		bool await_ready() const noexcept {
			return hasFinished();
		}

		bool await_suspend(std::experimental::coroutine_handle<void> awaitingCoroutine) noexcept;

		void await_resume() noexcept {}

		SleepAwaitable operator co_await() const noexcept {
			return *this;
		}
	};

	struct Task {
		struct promise_type {
			Task get_return_object() { return {}; }
			std::experimental::suspend_never initial_suspend() { return {}; }
			std::experimental::suspend_never final_suspend() { return {}; }
			void return_void() {}
			void unhandled_exception() {}
		};
	};

	class SleepUtility {
	private:
		//static DWORD sleepUntilTick;
		//static Promise<void>* promise;
	public:
		static SleepAwaitable& Sleep(int millisecondsToSleep);
		//static bool IsSleeping();

		static std::vector<SleepAwaitable&> SleepAwaitables;
	};
}