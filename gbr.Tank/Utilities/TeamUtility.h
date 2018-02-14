#pragma once

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class TeamUtility {
	private:
	public:
		static void Move(float x, float y);
		static awaitable<void> PlaceEoe(float x, float y, std::function<awaitable<void>()> afterSleepCheck);
		static awaitable<void> Spike(DWORD agentId, std::function<awaitable<void>()> afterSleepCheck);
		static awaitable<void> CleanupEnemies(float x, float y, std::function<awaitable<void>()> afterSleepCheck);

		static GW::Agent* GetBonder();
	};
}