#pragma once

#include <Windows.h>

namespace gbr::Tank::Controllers {
	class ConsController {
	private:
		DWORD hookId;
		DWORD sleepUntilTick;

		void Tick();
		bool IsPartyAlive();
		bool ShouldUseConsumable(DWORD skillId);
		bool UseConsumable(DWORD itemId);
	public:
		ConsController();
		~ConsController();
	};
}