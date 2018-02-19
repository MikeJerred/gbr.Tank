#include <algorithm>
#include <numeric>
#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/PartyMgr.h>

#include <gbr.Shared/Clients/NamedPipeClient.h>
#include <gbr.Shared/Commands/InteractAgent.h>
#include <gbr.Shared/Commands/MoveTo.h>
#include <gbr.Shared/Commands/PlaceSpirit.h>
#include <gbr.Shared/Commands/Seed.h>

#include "TeamUtility.h"
#include "AgentUtility.h"

namespace gbr::Tank::Utilities {

	void TeamUtility::Move(float x, float y) {
		gbr::Shared::Commands::MoveTo::Request request;
		request.x = x;
		request.y = y;
		request.zPlane = 0;

		gbr::Shared::Clients::SingletonNamedPipeClient::Send(request);
	}

	void TeamUtility::Seed() {
		gbr::Shared::Commands::Seed::Request request;
		gbr::Shared::Clients::SingletonNamedPipeClient::Send(request);
	}

	awaitable<void> TeamUtility::PlaceEoe(float x, float y, std::function<awaitable<void>()> afterSleepCheck) {

		gbr::Shared::Commands::PlaceSpirit::Request request;
		request.x = x;
		request.y = y;
		request.zPlane = 0;

		gbr::Shared::Clients::SingletonNamedPipeClient::Send(request);

		while (!AgentUtility::FindAgent(x, y, GW::Constants::Range::Nearby, GW::Constants::ModelID::EoE)) {
			co_await Sleep(100);
			if (afterSleepCheck)
				co_await afterSleepCheck();
		}
	}

	awaitable<void> TeamUtility::Spike(DWORD agentId, std::function<awaitable<void>()> afterSleepCheck) {
		while (GW::Agents::GetAgentByID(agentId) && !GW::Agents::GetAgentByID(agentId)->GetIsDead()) {
			gbr::Shared::Commands::InteractAgent::Request request;
			request.agentId = agentId;

			gbr::Shared::Clients::SingletonNamedPipeClient::Send(request);

			co_await Sleep(50);
			if (afterSleepCheck)
				co_await afterSleepCheck();
		}
	}

	awaitable<void> TeamUtility::CleanupEnemies(float x, float y, std::function<awaitable<void>()> afterSleepCheck) {
		while (true) {
			co_await Sleep(100);
			if (afterSleepCheck)
				co_await afterSleepCheck();

			auto enemies = AgentUtility::GetEnemiesInRange(x, y, 2000);

			if (enemies.size() > 0) {
				co_await Spike(enemies[0]->Id, afterSleepCheck);
			}
			else {
				break;
			}
		}
	}


	GW::Agent* TeamUtility::GetBonder() {
		for (auto player : GW::PartyMgr::GetPartyInfo()->players) {
			auto agentId = GW::Agents::GetAgentIdByLoginNumber(player.loginnumber);
			auto agent = GW::Agents::GetAgentByID(agentId);

			if (agent && agent->Primary == (BYTE)GW::Constants::Profession::Elementalist)
				return agent;
		}

		return nullptr;
	}

	GW::Agent* TeamUtility::GetMesmer() {
		for (auto player : GW::PartyMgr::GetPartyInfo()->players) {
			auto agentId = GW::Agents::GetAgentIdByLoginNumber(player.loginnumber);
			auto agent = GW::Agents::GetAgentByID(agentId);

			if (agent && agent->Primary == (BYTE)GW::Constants::Profession::Mesmer)
				return agent;
		}

		return nullptr;
	}
}