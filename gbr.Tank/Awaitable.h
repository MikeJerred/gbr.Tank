#pragma once

#include <Windows.h>
#include <experimental/resumable>
#include <functional>
#include <pplawait.h>
#include <ppltasks.h>
#include <vector>

namespace gbr::Tank {
	struct Awaitable {
	private:
		std::function<bool(void)> _hasFinished;
		std::experimental::coroutine_handle<void> _awaitingCoroutine;
	public:
		static std::vector<Awaitable*> Awaitables;

		Awaitable(std::function<bool(void)> hasFinished) noexcept : _hasFinished(hasFinished) {
		}

		bool hasFinished() const {
			return _hasFinished();
		}

		void complete() noexcept {
			_awaitingCoroutine.resume();
		}

		bool await_ready() const noexcept {
			return _hasFinished();
		}

		bool await_suspend(std::experimental::coroutine_handle<void> awaitingCoroutine) noexcept {
			_awaitingCoroutine = awaitingCoroutine;
			Awaitables.push_back(this);

			return !_hasFinished();
		}

		void await_resume() noexcept {}

		Awaitable operator co_await() const noexcept {
			return *this;
		}
	};

	static Awaitable Sleep(int millisecondsToSleep) {
		auto tickToResumeAt = GetTickCount() + millisecondsToSleep;

		return Awaitable([=]() {
			return GetTickCount() > tickToResumeAt;
		});
	}
}