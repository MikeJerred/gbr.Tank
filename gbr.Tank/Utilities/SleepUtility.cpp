#include "LogUtility.h"
#include "SleepUtility.h"

namespace gbr::Tank::Utilities {
	/*template<typename T>
	Promise<T>::Promise<T>() {
		todo = nullptr;
		children = std::vector<Promise<T>>();
	}

	template<typename T>
	Promise<T> Promise<T>::Then(std::function<void(T)> success) {
		todo = success;

		auto promise = Promise<T>();
		children.push_back(promise);
		return promise;
	}

	template<typename T>
	void Promise<T>::Resolve() {
		if (todo)
			todo();

		for (auto const& promise : children) {
			promise.Resolve();
		}

		children.clear();
	}*/


	/*Promise<T>* SleepUtility::Sleep(int ticks) {
		if (promise) {
			LogUtility::Log(L"Tried to sleep whilst already sleeping");
			promise->Resolve();
		}

		sleepUntilTick = GetTickCount() + ticks;

		promise = new Promise<void>();
		return promise;
	}

	bool SleepUtility::IsSleeping() {
		auto isSleeping = sleepUntilTick - GetTickCount() > 0;

		if (!isSleeping && promise) {
			promise->Resolve();
			delete promise;
		}

		return isSleeping;
	}*/



	std::vector<SleepAwaitable&> SleepUtility::SleepAwaitables = std::vector<SleepAwaitable&>();

	bool SleepAwaitable::await_suspend(std::experimental::coroutine_handle<void> awaitingCoroutine) noexcept {
		_awaitingCoroutine = awaitingCoroutine;
		SleepUtility::SleepAwaitables.push_back(*this);

		return !hasFinished();
	}

	SleepAwaitable& SleepUtility::Sleep(int millisecondsToSleep) {
		return SleepAwaitable(millisecondsToSleep);
	}
}