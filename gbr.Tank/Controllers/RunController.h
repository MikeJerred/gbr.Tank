#pragma once

#include "../Awaitable.h"
#include "../Exceptions.h"
#include "City/CityController.h"

namespace gbr::Tank::Controllers {
	class RunController {
	private:
		DWORD _hookId;

		static awaitable<void> Start();
		static awaitable<void> DoRun();
		static void ZonedCheck();
	public:
		RunController();
		~RunController();
	};
}