#include <algorithm>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>

#include "AgentUtility.h"

namespace gbr::Tank::Utilities {
	GW::Agent* AgentUtility::FindAgent(float x, float y, float range, int modelId) {
		auto agentArray = GW::Agents::GetAgentArray();
		auto player = GW::Agents::GetPlayer();
		const auto sqRange = range * range;

		if (agentArray.valid() && player) {
			for (auto agent : agentArray) {
				if (agent && agent->pos.SquaredDistanceTo(player->pos) < sqRange && (modelId == 0 || agent->PlayerNumber == modelId)) {
					return agent;
				}
			}
		}

		return nullptr;
	}


	MargoniteGroup::MargoniteGroup(GW::Agent* agent) : _agentIds() {
		_agentIds.push_back(agent->Id);


	}
}