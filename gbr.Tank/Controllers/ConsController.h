#pragma once

#include <Windows.h>

namespace gbr::Tank::Controllers {
	class ConsController {
	private:
		DWORD hookId;
		DWORD sleepUntilTick;

		void Tick();
	public:
		ConsController();
		~ConsController();
	};
}