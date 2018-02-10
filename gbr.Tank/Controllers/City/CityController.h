#pragma once

#include <vector>

namespace gbr::Tank::Controllers::City {
	class CityController {
	private:
	public:
		CityController();
		~CityController();

		awaitable<void> DoRun();
		static awaitable<void> MaintainEnchants();
		awaitable<void> WaitForBonds();

		static void CheckForFail();
	};
}