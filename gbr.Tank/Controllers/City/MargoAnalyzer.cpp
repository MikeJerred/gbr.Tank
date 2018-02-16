#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>

#include "../../Awaitable.h"
#include "../../Exceptions.h"
#include "../../Utilities/AgentUtility.h"
#include "MargoAnalyzer.h"

namespace gbr::Tank::Controllers::City {
	using AgentUtility = Utilities::AgentUtility;
	using namespace GW::Constants::ModelID::DoA;

	const std::vector<std::vector<int>> MargoAnalyzer::MargoGroup::PossibleGroups{
		{ MargoniteAnurDabi, MargoniteAnurDabi, MargoniteAnurKi, MargoniteAnurRund, MargoniteAnurVu },
		{ MargoniteAnurDabi, MargoniteAnurDabi, MargoniteAnurKi, MargoniteAnurRund, MargoniteAnurTuk },
		{ MargoniteAnurDabi, MargoniteAnurDabi, MargoniteAnurKaya, MargoniteAnurSu, MargoniteAnurSu, MargoniteAnurTuk },
		{ MargoniteAnurDabi, MargoniteAnurRuk, MargoniteAnurRuk, MargoniteAnurRund, MargoniteAnurVu },
		{ MargoniteAnurDabi, MargoniteAnurRuk, MargoniteAnurRuk, MargoniteAnurRund, MargoniteAnurVu, MargoniteAnurVu },
		{ MargoniteAnurDabi, MargoniteAnurRund, MargoniteAnurSu, MargoniteAnurVu, MargoniteAnurVu },
		{ MargoniteAnurDabi, MargoniteAnurRuk, MargoniteAnurRund, MargoniteAnurSu, MargoniteAnurVu, MargoniteAnurVu },
		{ MargoniteAnurKi, MargoniteAnurSu, MargoniteAnurSu, MargoniteAnurVu, MargoniteAnurVu },
		{ MargoniteAnurKaya, MargoniteAnurKaya, MargoniteAnurKi, MargoniteAnurTuk, MargoniteAnurTuk },
		{ MargoniteAnurRuk, MargoniteAnurRuk, MargoniteAnurRund, MargoniteAnurVu, MargoniteAnurVu }
	};

	MargoAnalyzer::MargoAnalyzer() : _margoGroups() {
		_hookId = GW::GameThread::AddPermanentCall([=]() { Tick(); });
	}

	MargoAnalyzer::~MargoAnalyzer() {
		GW::GameThread::RemovePermanentCall(_hookId);

		for (auto margoGroup : _margoGroups) {
			delete margoGroup;
		}
	}

	void MargoAnalyzer::Tick() {
		auto agentArray = GW::Agents::GetAgentArray();

		if (agentArray.valid()) {
			auto newPotentialBalls = std::vector<std::vector<GW::Agent*>>();

			// update groups and remove stale ones
			for (auto it = _margoGroups.begin(); it != _margoGroups.end();) {
				if (!(*it)->StillExists()) {
					delete *it;
					it = _margoGroups.erase(it);
				}
				else {
					(*it)->Update();
					it++;
				}
			}

			// try to find any groups we are not already tracking
			for (auto agent : agentArray) {
				if (!AgentIsMargonite(agent))
					continue;

				auto alreadyInGroup = false;
				for (auto group : _margoGroups) {
					if (group->ContainsAgent(agent->Id)) {
						alreadyInGroup = true;
						break;
					}
				}

				if (alreadyInGroup)
					continue;

				auto wasAddedToBall = false;
				for (auto potentialBall : newPotentialBalls) {
					for (auto ballAgent : potentialBall) {
						if (agent->pos.SquaredDistanceTo(ballAgent->pos) < GW::Constants::SqrRange::Area) {
							wasAddedToBall = true;
							break;
						}
					}

					if (wasAddedToBall) {
						potentialBall.push_back(agent);
						break;
					}
				}

				if (!wasAddedToBall) {
					newPotentialBalls.push_back(std::vector<GW::Agent*> { agent });
				}
			}

			for (auto potentialBall : newPotentialBalls) {
				if (potentialBall.size() == 5 || potentialBall.size() == 6) {
					for (auto validModelIds : MargoGroup::PossibleGroups) {
						if (MatchesGroup(potentialBall, validModelIds)) {
							_margoGroups.push_back(new MargoGroup(potentialBall));
							break;
						}
					}
				}
			}
		}
	}

	bool MargoAnalyzer::MatchesGroup(std::vector<GW::Agent*> potentialBall, std::vector<int> validModelIds) {
		auto matched = std::vector<GW::Agent*>();

		for (auto modelId : validModelIds) {
			auto anyMatched = false;
			for (auto agent : potentialBall) {
				if (agent->PlayerNumber == modelId && std::find(matched.cbegin(), matched.cend(), agent) == matched.cend()) {
					matched.push_back(agent);
					anyMatched = true;
					break;
				}
			}

			if (!anyMatched)
				return false;
		}

		return potentialBall.size() == matched.size();
	}

	bool MargoAnalyzer::AgentIsMargonite(GW::Agent* agent) {
		return agent
			&& agent->GetIsLivingType()
			&& agent->Allegiance == 3
			&& !agent->GetIsDead()
			&& (agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurDabi
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurKaya
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurKi
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurRuk
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurRund
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurSu
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurTuk
				|| agent->PlayerNumber == GW::Constants::ModelID::DoA::MargoniteAnurVu
				);
		// todo: exclude the enemies on the wall
	}

	bool MargoAnalyzer::PlayerShouldWait() {
		for (auto group : _margoGroups) {
			if (group->GetDistanceFromPlayer() < 1400 && group->GetTimeBeforeBall() < 2000) {
				return true;
			}
		}

		return false;
	}



	template<>
	struct enable_bitmask_operators<MargoAnalyzer::MargoGroup::StateType> { static constexpr bool enable = true; };

	MargoAnalyzer::MargoGroup::MargoGroup(const std::vector<GW::Agent*>& agents) : _states(), _agentIds() {
		for (auto agent : agents)
			_agentIds.push_back(agent->Id);
	}

	bool MargoAnalyzer::MargoGroup::StillExists() {
		return std::all_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) {
			auto agent = GW::Agents::GetAgentByID(id);

			return agent && !agent->GetIsDead() && agent->pos.SquaredDistanceTo(GW::Agents::GetPlayer()->pos) > GW::Constants::SqrRange::Earshot;
		});
	}

	void MargoAnalyzer::MargoGroup::Update() {

		if (std::any_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) {
			auto speed = GetSpeed(id);
			return speed < 1.3f && speed > 0.1f;
		})) {

			_states.push_back(State(StateType::Wandering, GetTickCount()));
			return;
		}

		if (std::any_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) { return GetSpeed(id) > 1.3f; })) {

			if (_states.size() == 0) {
				_states.push_back(State(StateType::Collapsing | StateType::Running, GetTickCount()));
				return;
			}

			auto lastState = _states.back();
			if (!(lastState.state && StateType::Wandering)) {
				_states.push_back(State(StateType::Running, GetTickCount()));
				return;
			}
			if (!(lastState.state && StateType::Collapsed)) {
				_states.push_back(State(StateType::Collapsing, GetTickCount()));
				return;
			}

			_states.push_back(State(StateType::Running | StateType::Collapsing, GetTickCount()));
			return;
		}

		if (std::all_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) { return GetSpeed(id) < 0.1f; })) {

			if (_states.size() == 0) {
				_states.push_back(State(StateType::Collapsed | StateType::WaitingToWander, GetTickCount()));
				return;
			}

			auto lastState = _states.back();
			if (!(lastState.state && StateType::Running)) {
				_states.push_back(State(StateType::Collapsed, GetTickCount()));
				return;
			}
			if (!(lastState.state && StateType::Collapsing)) {
				_states.push_back(State(StateType::WaitingToWander, GetTickCount()));
				return;
			}

			_states.push_back(State(StateType::Collapsed | StateType::WaitingToWander, GetTickCount()));
			return;
		}


	}

	bool MargoAnalyzer::MargoGroup::ContainsAgent(DWORD agentId) {
		return std::any_of(_agentIds.begin(), _agentIds.end(), [=](DWORD id) { return id == agentId; });
	}

	float MargoAnalyzer::MargoGroup::GetSpeed(DWORD agentId) {
		auto agent = GW::Agents::GetAgentByID(agentId);
		return sqrtf(agent->MoveX * agent->MoveX + agent->MoveY * agent->MoveY) / 288.0f;
	}

	DWORD MargoAnalyzer::MargoGroup::GetTimeBeforeBall() {
		if (_states.size() == 0)
			return 0;

		auto lastState = _states.back();
		if ((lastState.state && StateType::Collapsed) || (lastState.state && StateType::Collapsing)) {
			return 0;
		}

		if (lastState.state && StateType::Wandering) {
			if (_states.size() == 1) {
				return 0;
			}
			else {
				auto timeSpentWandering = GetTickCount() - lastState.tick;

				return WanderingTime - timeSpentWandering;
			}
		}

		if (lastState.state && StateType::WaitingToWander) {
			if (_states.size() == 1) {
				return WanderingTime;
			}
			else {
				auto timeSpentWaiting = GetTickCount() - lastState.tick;

				return WanderingTime + WaitingTime - timeSpentWaiting;
			}
		}
		if (lastState.state && StateType::Running) {
			if (_states.size() == 1) {
				return WanderingTime + WaitingTime;
			}
			else {
				auto timeSpentRunning = GetTickCount() - lastState.tick;

				return WanderingTime + WaitingTime + RunningTime - timeSpentRunning;
			}
		}
	}

	float MargoAnalyzer::MargoGroup::GetDistanceFromPlayer() {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto closestAgentId = *std::min_element(_agentIds.begin(), _agentIds.end(), [=](DWORD id1, DWORD id2) {
			auto agent1 = GW::Agents::GetAgentByID(id1);
			auto agent2 = GW::Agents::GetAgentByID(id2);

			return agent1->pos.SquaredDistanceTo(playerPos) < agent2->pos.SquaredDistanceTo(playerPos);
		});

		return GW::Agents::GetAgentByID(closestAgentId)->pos.DistanceTo(playerPos);
	}
}