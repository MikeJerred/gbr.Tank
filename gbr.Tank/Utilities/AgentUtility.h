#pragma once

#include <GWCA/GWCA.h>
#include <GWCA/GWStructures.h>

#include "../Awaitable.h"

namespace gbr::Tank::Utilities {

	class AgentUtility {
	private:
	public:
		static GW::Agent* FindAgent(float x, float y, float range, int modelId = 0);
		static Ball GetBall(float x, float y) {
			return Ball(x, y);
		}
	};




	class Ball {
	private:
		std::vector<DWORD> _agentIds;
	public:
		Ball(float x, float y);

		DWORD GetCentralTarget();
	};


	class MargoniteGroup {
	private:
		std::vector<DWORD> _agentIds;
	public:
		MargoniteGroup(GW::Agent* agent);
	};
}