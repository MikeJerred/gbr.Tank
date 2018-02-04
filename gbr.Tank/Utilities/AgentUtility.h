#pragma once

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class AgentUtility {
	private:
	public:
		static GW::Agent* FindAgent(float x, float y, float range, int modelId = 0);
	};


	class MargoniteGroup {
	private:
		std::vector<DWORD> _agentIds;
	public:
		MargoniteGroup(GW::Agent* agent);
	};
}