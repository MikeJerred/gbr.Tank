#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>

#include "../../Awaitable.h"
#include "../../Exceptions.h"
#include "../../Utilities/AgentUtility.h"
#include "../../Utilities/LogUtility.h"
#include "MargoAnalyzer.h"

namespace gbr::Tank::Controllers::City {
	using AgentUtility = Utilities::AgentUtility;
	using LogUtility = Utilities::LogUtility;
	using namespace GW::Constants::ModelID::DoA;

	const std::vector<std::vector<int>> MargoAnalyzer::MargoGroup::PossibleGroups {
		{ MargoniteAnurDabi, MargoniteAnurDabi, MargoniteAnurKaya, MargoniteAnurSu, MargoniteAnurSu, MargoniteAnurTuk },
		{ MargoniteAnurDabi, MargoniteAnurDabi, MargoniteAnurKi, MargoniteAnurRund, MargoniteAnurVu },
		{ MargoniteAnurDabi, MargoniteAnurDabi, MargoniteAnurKi, MargoniteAnurRund, MargoniteAnurTuk },
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

		for (auto margoGroup : _margoGroups)
			delete margoGroup;
	}

	void MargoAnalyzer::Tick() {
		auto agentArray = GW::Agents::GetAgentArray();

		if (agentArray.valid()) {
			auto newPotentialBalls = std::vector<std::vector<GW::Agent*>>();

			// update groups and remove stale ones
			for (auto it = _margoGroups.begin(); it != _margoGroups.end();) {
				if ((*it)->StillExists()) {
					(*it)->Update();
					++it;
				}
				else {
					delete *it;
					it = _margoGroups.erase(it);
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
				for (auto& potentialBall : newPotentialBalls) {
					for (auto ballAgent : potentialBall) {
						if (agent->pos.SquaredDistanceTo(ballAgent->pos) < 700*700) {
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

			for (auto& potentialBall : newPotentialBalls) {
				if (potentialBall.size() == 5 || potentialBall.size() == 6) {
					for (auto& validModelIds : MargoGroup::PossibleGroups) {
						if (MatchesGroup(potentialBall, validModelIds)) {
							_margoGroups.push_back(new MargoGroup(potentialBall));
							break;
						}
					}
				}
			}
		}

		// log info
		static int tickCount = 0;
		++tickCount;
		if (tickCount > 500) {
			tickCount = 0;

			int i = 0;
			for (auto grp : _margoGroups) {
				++i;
				LogUtility::Log(std::to_wstring(i) + L": " + grp->GetLastStateName());
			}

		}
	}

	bool MargoAnalyzer::MatchesGroup(const std::vector<GW::Agent*>& potentialBall, const std::vector<int>& validModelIds) {
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
			&& agent->GetIsCharacterType()
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
	}

	bool MargoAnalyzer::PlayerShouldWait() {
		for (auto group : _margoGroups) {
			auto distanceFromPlayer = group->GetDistanceFromPlayer();
			if (!group->IsAggroed() && distanceFromPlayer < 1800) {
				if (group->GetTimeBeforeBall() * 0.288f < (300 + distanceFromPlayer - GW::Constants::Range::Earshot))
					return true;
			}
		}

		return false;
	}

	bool MargoAnalyzer::AllGroupsInRangeAggroed(float x, float y, float sqrRange) {
		for (auto group : _margoGroups) {
			if (!group->IsAggroed() && group->GetSqrDistanceFrom(x, y) < sqrRange) {
				return false;
			}
		}

		return true;
	}



	template<>
	struct enable_bitmask_operators<MargoAnalyzer::MargoGroup::StateType> { static constexpr bool enable = true; };

	MargoAnalyzer::MargoGroup::MargoGroup(const std::vector<GW::Agent*>& agents) : _states(), _agentIds() {
		_isAggroed = false;

		for (auto agent : agents)
			_agentIds.push_back(agent->Id);
	}

	bool MargoAnalyzer::MargoGroup::StillExists() {
		return std::any_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) {
			auto agent = GW::Agents::GetAgentByID(id);
			return agent && !agent->GetIsDead();
		});
	}

	bool MargoAnalyzer::MargoGroup::IsAggroed() {
		if (_isAggroed) {
			_isAggroed = std::any_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) {
				auto agent = GW::Agents::GetAgentByID(id);
				return agent && !agent->GetIsDead() && agent->pos.SquaredDistanceTo(GW::Agents::GetPlayer()->pos) < 1800 * 1800;
			});

			if (!_isAggroed) {
				LogUtility::Log(L"Aggroed -> Not Aggroed");
			}
		}
		else {
			_isAggroed = std::any_of(_agentIds.begin(), _agentIds.end(), [](DWORD id) {
				auto agent = GW::Agents::GetAgentByID(id);
				return agent && !agent->GetIsDead() && agent->pos.SquaredDistanceTo(GW::Agents::GetPlayer()->pos) < GW::Constants::SqrRange::Earshot;
			});

			if (_isAggroed) {
				LogUtility::Log(L"Not Aggroed -> Aggroed");
			}
		}

		return _isAggroed;
	}

	bool MargoAnalyzer::MargoGroup::AreMostlyBalledTogether() {
		auto agent0 = GW::Agents::GetAgentByID(_agentIds[0]);

		if (std::count_if(_agentIds.begin(), _agentIds.end(), [=](DWORD id) {
			if (id == agent0->Id)
				return false;

			auto agent = GW::Agents::GetAgentByID(id);
			return agent->pos.SquaredDistanceTo(agent0->pos) < GW::Constants::SqrRange::Adjacent;
		}) >= 4) {
			return true;
		}

		auto agent1 = GW::Agents::GetAgentByID(_agentIds[1]);

		if (std::count_if(_agentIds.begin(), _agentIds.end(), [=](DWORD id) {
			if (id == agent1->Id)
				return false;

			auto agent = GW::Agents::GetAgentByID(id);
			return agent->pos.SquaredDistanceTo(agent1->pos) < GW::Constants::SqrRange::Adjacent;
		}) >= 4) {
			return true;
		}

		return false;
	}

	void MargoAnalyzer::MargoGroup::Update() {

		// remove any dead agents
		for (auto it = _agentIds.begin(); it != _agentIds.end();) {
			auto agent = GW::Agents::GetAgentByID(*it);
			if (!agent || agent->GetIsDead()) {
				it = _agentIds.erase(it);
			}
			else {
				++it;
			}
		}

		// if at least 3 of them are running
		if (std::count_if(_agentIds.begin(), _agentIds.end(), [](DWORD id) { return GetSpeed(id) > 1.0f; }) >= 3) {

			if (_states.size() == 0) {
				PushBackState(StateType::Collapsing | StateType::Running);
				return;
			}

			auto lastState = _states.back();
			if (lastState.state == (StateType::Running | StateType::Collapsing)) {
				if (GetTickCount() > lastState.tick + 3000)
					lastState.state = StateType::Running;

				return;
			}

			if (lastState.state == (StateType::Running | StateType::Wandering)) {
				lastState.state = StateType::Running;
				return;
			}
			if (lastState.state == (StateType::Collapsing | StateType::Wandering)) {
				lastState.state = StateType::Collapsing;
				return;
			}

			if ((lastState.state & StateType::Running) || (lastState.state & StateType::Collapsing))
				return;

			if (!(lastState.state & StateType::Wandering) && (lastState.state & StateType::Collapsed)) {
				PushBackState(StateType::Running);
				return;
			}
			if (!(lastState.state & StateType::Collapsed) && (lastState.state & StateType::Wandering)) {
				PushBackState(StateType::Collapsing);
				return;
			}

			PushBackState(StateType::Running | StateType::Collapsing);
			return;
		}

		// if at least 3 of them are wandering
		if (std::count_if(_agentIds.begin(), _agentIds.end(), [](DWORD id) {
			auto speed = GetSpeed(id);
			return speed <= 1.0f && speed > 0.3f;
		}) >= 3) {

			if (_states.size() == 0) {
				PushBackState(StateType::Wandering | StateType::Collapsing | StateType::Running);
				return;
			}

			auto lastState = _states.back();
			if (lastState.state == StateType::Wandering) {
				return;
			}

			
			// they might have just started running and haven't reached full speed yet
			if (lastState.state == StateType::Collapsed) {
				PushBackState(StateType::Wandering | StateType::Running);
				return;
			}

			if (lastState.state == StateType::WaitingToWander) {
				PushBackState(StateType::Wandering);
				return;
			}

			// they might be collapsing and slowed down
			if ((lastState.state == StateType::Collapsing) && AreMostlyBalledTogether()) {
				if (GetTickCount() < lastState.tick + 3000)
					return;
				else {
					PushBackState(StateType::Wandering | StateType::Collapsed);
					return;
				}
			}

			// they might be running and slowed down
			if ((lastState.state == StateType::Running) && AreMostlyBalledTogether()) {
				if (GetTickCount() < lastState.tick + 3000)
					return;
				else {
					PushBackState(StateType::Wandering | StateType::WaitingToWander);
					return;
				}
			}

			if (lastState.state == (StateType::Running | StateType::Collapsing) && AreMostlyBalledTogether()) {
				return;
			}

			// if they have been moving at this speed for more than 3 seconds, then they are definitely wandering
			if ((lastState.state & StateType::Wandering) && (GetTickCount() > lastState.tick + 3000)) {
				lastState.state = StateType::Wandering;
				return;
			}

			PushBackState(StateType::Wandering | StateType::Collapsing | StateType::Running);
			return;
		}

		// if at least 3 of them are still
		if (std::count_if(_agentIds.begin(), _agentIds.end(), [](DWORD id) { return GetSpeed(id) <= 0.1f; }) >= 3) {
			if (_states.size() == 0) {
				if (AreMostlyBalledTogether())
					PushBackState(StateType::Collapsed | StateType::WaitingToWander);
				else
					PushBackState(StateType::WaitingToWander | StateType::Wandering);

				return;
			}

			auto lastState = _states.back();

			if (lastState.state == (StateType::WaitingToWander | StateType::Wandering)) {
				if (GetTickCount() > lastState.tick + 1000)
					lastState.state = StateType::WaitingToWander;

				return;
			}

			if (lastState.state == (StateType::Collapsed | StateType::Wandering)) {
				if (GetTickCount() > lastState.tick + 1000)
					lastState.state = StateType::Collapsed;

				return;
			}

			if ((lastState.state & StateType::Collapsed) || (lastState.state & StateType::WaitingToWander) || (lastState.state & StateType::Wandering))
				return;

			if (!(lastState.state & StateType::Running) && (lastState.state & StateType::Collapsing)) {
				if (AreMostlyBalledTogether())
					PushBackState(StateType::Collapsed);

				return;
			}
			if (!(lastState.state & StateType::Collapsing) && (lastState.state & StateType::Running)) {
				PushBackState(StateType::WaitingToWander);
				return;
			}

			PushBackState(StateType::Collapsed | StateType::WaitingToWander);
			return;
		}
	}

	void MargoAnalyzer::MargoGroup::PushBackState(StateType state) {
		if (_states.size() > 0) {
			LogUtility::Log(L"State from [" + GetLastStateName() + L"] to [" + GetStateName(state) + L"]");
		}
		else {
			LogUtility::Log(L"Initial state [" + GetStateName(state) + L"]");
		}

		_states.push_back(State(state, GetTickCount()));
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
		if ((lastState.state & StateType::Collapsed) || (lastState.state & StateType::Collapsing)) {
			return 0;
		}

		if (lastState.state & StateType::Wandering) {
			if (_states.size() == 1) {
				return 0;
			}
			else {
				auto timeSpentWandering = GetTickCount() - lastState.tick;
				return WanderingTime - timeSpentWandering;
			}
		}

		if (lastState.state & StateType::WaitingToWander) {
			if (_states.size() == 1) {
				return WanderingTime;
			}
			else {
				auto timeSpentWaiting = GetTickCount() - lastState.tick;
				return WanderingTime + WaitingTime - timeSpentWaiting;
			}
		}

		if (lastState.state & StateType::Running) {
			if (_states.size() == 1) {
				return WanderingTime + WaitingTime;
			}
			else {
				auto timeSpentRunning = GetTickCount() - lastState.tick;
				return WanderingTime + WaitingTime + RunningTime - timeSpentRunning;
			}
		}

		return 0;
	}

	float MargoAnalyzer::MargoGroup::GetDistanceFromPlayer() {
		auto playerPos = GW::Agents::GetPlayer()->pos;

		auto closestAgentId = *std::min_element(_agentIds.begin(), _agentIds.end(), [=](DWORD id1, DWORD id2) {
			auto agent1 = GW::Agents::GetAgentByID(id1);
			auto agent2 = GW::Agents::GetAgentByID(id2);

			if (!agent1)
				return false;

			if (!agent2)
				return true;

			return agent1->pos.SquaredDistanceTo(playerPos) < agent2->pos.SquaredDistanceTo(playerPos);
		});

		auto agent = GW::Agents::GetAgentByID(closestAgentId);
		if (!agent)
			return GW::Constants::Range::Compass;

		return agent->pos.DistanceTo(playerPos);
	}

	float MargoAnalyzer::MargoGroup::GetSqrDistanceFrom(float x, float y) {
		auto pos = GW::GamePos(x, y);

		auto closestAgentId = *std::min_element(_agentIds.begin(), _agentIds.end(), [=](DWORD id1, DWORD id2) {
			auto agent1 = GW::Agents::GetAgentByID(id1);
			auto agent2 = GW::Agents::GetAgentByID(id2);

			if (!agent1)
				return false;

			if (!agent2)
				return true;

			return agent1->pos.SquaredDistanceTo(pos) < agent2->pos.SquaredDistanceTo(pos);
		});

		auto agent = GW::Agents::GetAgentByID(closestAgentId);
		if (!agent)
			return GW::Constants::Range::Compass;

		return agent->pos.SquaredDistanceTo(pos);
	}
}