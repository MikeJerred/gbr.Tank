#pragma once

#include <vector>

namespace gbr::Tank::Controllers::City {
	class CityController {
	private:
	public:
		CityController();
		~CityController();

		concurrency::task<bool> DoRun();
		concurrency::task<bool> MaintainEnchants();
		concurrency::task<bool> WaitForBonds();
	};
}