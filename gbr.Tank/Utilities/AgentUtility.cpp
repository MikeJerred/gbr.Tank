#include <algorithm>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>

#include "AgentUtility.h"

namespace gbr::Tank::Utilities {
	GW::Agent* AgentUtility::FindAgent(float x, float y, float range, int modelId) {
		auto agentArray = GW::Agents::GetAgentArray();
		const auto sqRange = range * range;

		if (agentArray.valid()) {
			for (auto agent : agentArray) {
				if (agent && agent->pos.SquaredDistanceTo(GW::Vector2f(x, y)) < sqRange && (modelId == 0 || agent->PlayerNumber == modelId)) {
					return agent;
				}
			}
		}

		return nullptr;
	}

	Ball::Ball(float x, float y) : _agentIds() {
		auto agentArray = GW::Agents::GetAgentArray();

		if (agentArray.valid()) {
			for (auto agent : agentArray) {
				if (agent
					&& agent->GetIsLivingType()
					&& !agent->GetIsDead()
					&& agent->PlayerNumber != GW::Constants::ModelID::EoE
					&& agent->PlayerNumber != GW::Constants::ModelID::QZ
					&& agent->PlayerNumber != GW::Constants::ModelID::Winnowing
					&& agent->Allegiance == 3
					&& agent->pos.SquaredDistanceTo(GW::Vector2f(x, y)) < GW::Constants::SqrRange::Nearby) {
					
					_agentIds.push_back(agent->Id);
				}
			}
		}
	}

	DWORD Ball::GetCentralTarget() {
		auto agentArray = GW::Agents::GetAgentArray();
		auto playerPos = GW::Agents::GetPlayer()->pos;

		std::

		if (agentArray.valid()) {
			for (auto id : _agentIds) {
				auto agent = agentArray[id];

				if (agent
					&& !agent->GetIsDead()
					&& ) {

				}
			}
		}

		return 0;
	}


	MargoniteGroup::MargoniteGroup(GW::Agent* agent) : _agentIds() {
		_agentIds.push_back(agent->Id);


	}
}