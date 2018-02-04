#pragma once

#include "../Awaitable.h"
#include "City/CityController.h"

namespace gbr::Tank::Controllers {
	class RunController {
	private:
		DWORD _hookId;

		static concurrency::task<void> DoRun();
	public:
		RunController();
		~RunController();
	};
}