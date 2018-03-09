#include <algorithm>
#include <numeric>
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

	std::vector<GW::Agent*> AgentUtility::GetEnemiesInRange(float x, float y, float range) {
		auto agentArray = GW::Agents::GetAgentArray();
		const auto sqRange = range * range;

		std::vector<GW::Agent*> results;

		if (agentArray.valid()) {
			for (auto agent : agentArray) {
				if (agent
					&& agent->GetIsCharacterType()
					&& !agent->GetIsDead()
					&& agent->PlayerNumber != GW::Constants::ModelID::EoE
					&& agent->PlayerNumber != GW::Constants::ModelID::QZ
					&& agent->PlayerNumber != GW::Constants::ModelID::Winnowing
					&& agent->PlayerNumber != GW::Constants::ModelID::Famine
					&& agent->PlayerNumber != GW::Constants::ModelID::Quicksand
					&& agent->Allegiance == 3
					&& agent->pos.SquaredDistanceTo(GW::Vector2f(x, y)) < sqRange) {

					results.push_back(agent);
				}
			}
		}

		return results;
	}

	awaitable<void> AgentUtility::WaitForSettle(std::function<awaitable<void>()> afterSleepCheck) {
		bool settled = false;

		while (!settled) {
			co_await Sleep(50);
			if (afterSleepCheck)
				co_await afterSleepCheck();

			settled = true;
			auto playerPos = GW::Agents::GetPlayer()->pos;

			for (auto agent : GW::Agents::GetAgentArray()) {
				if (agent
					&& agent->GetIsCharacterType()
					&& !agent->GetIsDead()
					&& agent->Allegiance == 3
					&& agent->pos.SquaredDistanceTo(playerPos) < 1500 * 1500
					&& (agent->MoveX > 1 || agent->MoveY > 1)) {

					settled = false;
					break;
				}
			}
		}
	}

	void AgentUtility::StopMoving() {
		INPUT ip;
		ip.type = INPUT_KEYBOARD;
		ip.ki.wScan = 0;
		ip.ki.time = 0;
		ip.ki.dwExtraInfo = 0;
		ip.ki.wVk = 0x53; // S

		// keydown
		ip.ki.dwFlags = 0;
		SendInput(1, &ip, sizeof(INPUT));

		// keyup
		ip.ki.dwFlags = KEYEVENTF_KEYUP;
		SendInput(1, &ip, sizeof(INPUT));
	}

	Ball::Ball(float x, float y) : _agentIds() {
		for (auto agent : AgentUtility::GetEnemiesInRange(x, y, GW::Constants::Range::Nearby)) {
			_agentIds.push_back(agent->Id);
		}
	}

	GW::Agent* Ball::GetCentralTarget() {
		auto agentArray = GW::Agents::GetAgentArray();
		auto playerPos = GW::Agents::GetPlayer()->pos;

		std::vector<GW::Agent*> validAgents;

		for (auto id : _agentIds) {
			auto agent = agentArray[id];

			if (agent && !agent->GetIsDead())
				validAgents.push_back(agent);
		}

		auto avgX = std::accumulate(validAgents.cbegin(), validAgents.cend(), 0.0f, [](float acc, GW::Agent* agent) {
			return acc + agent->X;
		}) / validAgents.size();

		auto avgY = std::accumulate(validAgents.cbegin(), validAgents.cend(), 0.0f, [](float acc, GW::Agent* agent) {
			return acc + agent->Y;
		}) / validAgents.size();

		auto avgPos = GW::Vector2f(avgX, avgY);

		auto result = std::min_element(validAgents.begin(), validAgents.end(), [=](GW::Agent* a, GW::Agent* b) {
			return a->pos.SquaredDistanceTo(avgPos) < b->pos.SquaredDistanceTo(avgPos);
		});

		return result != validAgents.end() ? *result : nullptr;
	}


	MargoniteGroup::MargoniteGroup(GW::Agent* agent) : _agentIds() {
		_agentIds.push_back(agent->Id);


	}
}