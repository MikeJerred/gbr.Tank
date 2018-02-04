#pragma once

#include <vector>

namespace gbr::Tank::Controllers::City {
	class CityController {
	private:
		DWORD _maintainEnchantsHookId;
	public:
		CityController();
		~CityController();

		concurrency::task<void> DoRun();
		void MaintainEnchants();
	};
}