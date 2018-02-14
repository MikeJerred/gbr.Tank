#pragma once

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class AgentUtility {
	private:
	public:
		static GW::Agent* FindAgent(float x, float y, float range, int modelId = 0);
		static std::vector<GW::Agent*> GetEnemiesInRange(float x, float y, float range);

		static awaitable<void> WaitForSettle(std::function<awaitable<void>()> afterSleepCheck);
	};




	class Ball {
	private:
		std::vector<DWORD> _agentIds;
	public:
		Ball(float x, float y);

		GW::Agent* GetCentralTarget();
	};


	class MargoniteGroup {
	private:
		std::vector<DWORD> _agentIds;
	public:
		MargoniteGroup(GW::Agent* agent);
	};
}