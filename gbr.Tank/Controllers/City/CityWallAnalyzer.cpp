#include <algorithm>

#include <GWCA/GWCA.h>
#include <GWCA/Managers/AgentMgr.h>
#include <GWCA/Managers/GameThreadMgr.h>

#include "CityWallAnalyzer.h"

namespace gbr::Tank::Controllers::City {
	const std::vector<std::tuple<GW::GamePos, GW::GamePos>> CityWallAnalyzer::Positions {
		{ GW::GamePos(-9826, -12092), GW::GamePos(-9528, -11596) },
		{ GW::GamePos(-8884, -11889), GW::GamePos(-9063, -11527) },
		{ GW::GamePos(-8173, -11730), GW::GamePos(-8225, -11433) },
		{ GW::GamePos(-7449, -11720), GW::GamePos(-7441, -11428) },
		{ GW::GamePos(-7131, -11688), GW::GamePos(-7441, -11428) },
		{ GW::GamePos(-5733, -11719), GW::GamePos(-5672, -11481) },
		{ GW::GamePos(-5402, -11672), GW::GamePos(-5672, -11481) },
		{ GW::GamePos(-4721, -11643), GW::GamePos(-4820, -11393) },
		{ GW::GamePos(-3957, -11686), GW::GamePos(-3989, -11402) },
		{ GW::GamePos(-3034, -11695), GW::GamePos(-3481, -11389) }
	};

	CityWallAnalyzer::CityWallAnalyzer() : _enemies() {
		_hookId = GW::GameThread::AddPermanentCall([=]() { Tick(); });

		for (auto pos : Positions)
			_enemies.push_back(CityWallEnemy(std::get<0>(pos), std::get<1>(pos), true));
	}
	
	CityWallAnalyzer::~CityWallAnalyzer() {
		GW::GameThread::RemovePermanentCall(_hookId);
	}

	void CityWallAnalyzer::Tick() {
		auto agentArray = GW::Agents::GetAgentArray();
		if (agentArray.valid()) {
			for (auto agent : agentArray) {
				if (!agent
					|| !agent->GetIsCharacterType()
					|| agent->Allegiance != 3
					|| (agent->PlayerNumber != GW::Constants::ModelID::DoA::MargoniteAnurSu && agent->PlayerNumber != GW::Constants::ModelID::DoA::MargoniteAnurMank))
					continue;

				for (auto enemy : _enemies) {
					if (agent->pos.SquaredDistanceTo(enemy.position) < GW::Constants::SqrRange::Adjacent) {
						enemy.isAlive = !agent->GetIsDead();
						break;
					}
				}
			}
		}
	}

	bool CityWallAnalyzer::IsWallDead() {
		for (auto enemy : _enemies) {
			if (enemy.isAlive)
				return false;
		}

		return true;
	}

	CityWallAnalyzer::CityWallEnemy* CityWallAnalyzer::GetNextWallEnemy() {
		for (auto& enemy : _enemies) {
			if (enemy.isAlive)
				return &enemy;
		}

		return nullptr;
	}
}